/** \file   settings_mmcr.c
 * \brief   Settings widget to control MMC Replay resources
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 */

/*
 * $VICERES MMCRCardImage   x64 x64sc xscpu64 x128
 * $VICERES MMCREEPROMImage x64 x64sc xscpu64 x128
 * $VICERES MMCREEPROMRW    x64 x64sc xscpu64 x128
 * $VICERES MMCRRescueMode  x64 x64sc xscpu64 x128
 * $VICERES MMCRImageWrite  x64 x64sc xscpu64 x128
 * $VICERES MMCRCardRW      x64 x64sc xscpu64 x128
 * $VICERES MMCRSDType      x64 x64sc xscpu64 x128
 * $VICERES MMCRClockPort   x64 x64sc xscpu64 x128
 */

/*
 * This file is part of VICE, the Versatile Commodore Emulator.
 * See README for copyright notice.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 *  02111-1307  USA.
 *
 */

#include "vice.h"
#include <gtk/gtk.h>

#include "vice_gtk3.h"
#include "machine.h"
#include "resources.h"
#include "c64cart.h"
#include "cartridge.h"
#include "ui.h"

#include "settings_mmcr.h"


/** \brief  List of memory card types
 */
static const vice_gtk3_radiogroup_entry_t card_types[] = {
    { "Auto",    MMCR_TYPE_AUTO },
    { "MMC",     MMCR_TYPE_MMC },
    { "SD",      MMCR_TYPE_SD },
    { "SDHC",    MMCR_TYPE_SDHC },
    { NULL,     -1 }
};


/* FIXME:   The eeprom handling uses seperate widgets to handle the entry and
 *          button while the card widget uses a widget in widgets/base, why?
 */

/** \brief  EEPROM text entry widget */
static GtkWidget *eeprom_entry;

/** \brief  SD card widget */
static GtkWidget *card_widget;


/** \brief  Callback for the save-dialog response handler
 *
 * \param[in,out]   dialog      save-file dialog
 * \param[in,out]   filename    filename
 * \param[in]       data        extra data (unused)
 */
static void save_filename_callback(GtkDialog *dialog,
                                   gchar *filename,
                                   gpointer data)
{
    if (filename != NULL) {
        if (cartridge_save_image(CARTRIDGE_MMC_REPLAY, filename) < 0) {
            vice_gtk3_message_error("Saving failed",
                    "Failed to save cartridge image '%s'",
                    filename);
        }
        g_free(filename);
    }
    gtk_widget_destroy(GTK_WIDGET(dialog));
}


/** \brief  Handler for the "clicked" event of the Save Image button
 *
 * \param[in]   widget      button (unused)
 * \param[in]   user_data   extra event data (unused)
 */
static void on_save_clicked(GtkWidget *widget, gpointer user_data)
{
    vice_gtk3_save_file_dialog("Save cartridge image",
                               NULL, TRUE, NULL,
                               save_filename_callback,
                               NULL);
}


/** \brief  Handler for the "clicked" event of the Flush Image button
 *
 * \param[in]   widget      button
 * \param[in]   user_data   unused
 */
static void on_flush_clicked(GtkWidget *widget, gpointer user_data)
{
    if (cartridge_flush_image(CARTRIDGE_MMC_REPLAY) < 0) {
        vice_gtk3_message_error("Flushing failed",
                    "Failed to fush cartridge image");
    }
}


/** \brief  Callback for the EEPROM file selection dialog
 *
 * \param[in,out]   dialog      file chooser dialog
 * \param[in,out]   filename    filename (freed with g_free())
 * \param[in]       data        extra data (unused)
 */
static void eeprom_filename_callback(GtkDialog *dialog,
                                     gchar *filename,
                                     gpointer data)
{
    if (filename != NULL) {
        if (resources_set_string("MMCREEPROMImage", filename) < 0) {
            vice_gtk3_message_error("Failed to load EEPROM file",
                    "Failed to load EEPROM image file '%s'",
                    filename);
        } else {
            gtk_entry_set_text(GTK_ENTRY(eeprom_entry), filename);
        }
        g_free(filename);
    }
    gtk_widget_destroy(GTK_WIDGET(dialog));
}


/** \brief  Handler for the 'clicked' event of the EEPROM "browse" button
 *
 * Opens a file chooser dialog to select an EEPROM image file.
 *
 * \param[in]   button  browse button (unused)
 * \param[in]   data    extra event data (unused)
 */
static void on_eeprom_browse_clicked(GtkWidget *button, gpointer data)
{
    GtkWidget *dialog;

    dialog = vice_gtk3_open_file_dialog(
            "Open EEMPROM image",
             NULL, NULL, NULL,
             eeprom_filename_callback,
             NULL);
    gtk_widget_show(dialog);
}


/** \brief  Callback for the SD card image file selection dialog
 *
 * \param[in,out]   dialog      file chooser dialog
 * \param[in,out]   filename    filename (freed with g_free())
 * \param[in]       data        extra data (unused)
 */
static void card_filename_callback(GtkDialog *dialog,
                                   gchar *filename,
                                   gpointer data)
{
    if (filename != NULL) {
        vice_gtk3_resource_entry_set(card_widget, filename);
        g_free(filename);
    }

}


/** \brief  Handler for the "clicked" event of the memory card browse button
 *
 * \param[in]   button      browse button
 * \param[in]   user_data   unused
 */
static void on_card_browse_clicked(GtkWidget *button, gpointer user_data)
{
    GtkWidget *dialog;

    dialog = vice_gtk3_open_file_dialog(
            "Open memory card file",
            NULL, NULL, NULL,
            card_filename_callback,
            NULL);
    gtk_widget_show(dialog);
}


/** \brief  Create check button for the MMCRRescueMode resource
 *
 * \return  GtkCheckButton
 */
static GtkWidget *create_rescue_mode_widget(void)
{
    return vice_gtk3_resource_check_button_new(
            "MMCRRescueMode", "Enable rescue mode");
}


/** \brief  Create widget for the MMCRClockPort resource
 *
 * \return  GtkComboBoxText
 */
static GtkWidget *create_clockport_widget(void)
{
    return clockport_device_widget_create("MMCRClockPort");
}


/** \brief  Create widget for the MMCREEPROMRW resource
 *
 * \return  GtkComboBoxText
 */
static GtkWidget *create_eeprom_rw_widget(void)
{
    return vice_gtk3_resource_check_button_new("MMCREEPROMRW",
            "Enable writes to EEPROM image");
}


/** \brief  Create widget to handle Cartridge image resources and save/flush
 *
 * \return  GtkGrid
 */
static GtkWidget *create_cart_image_widget(void)
{
    GtkWidget *grid;
    GtkWidget *write_back;
    GtkWidget *save_button;
    GtkWidget *flush_button;

    grid = vice_gtk3_grid_new_spaced_with_label(
            -1, -1, "MMC Replay Cartridge image", 3);

    write_back = vice_gtk3_resource_check_button_new("MMCRImageWrite",
                "Save image when changed");
    gtk_widget_set_margin_start(write_back, 16);
    gtk_grid_attach(GTK_GRID(grid), write_back, 0, 1, 1, 1);

    save_button = gtk_button_new_with_label("Save image as ...");
    g_signal_connect(save_button, "clicked", G_CALLBACK(on_save_clicked),
            NULL);
    gtk_grid_attach(GTK_GRID(grid), save_button, 1, 1, 1, 1);

    flush_button = gtk_button_new_with_label("Flush image now");
    g_signal_connect(flush_button, "clicked", G_CALLBACK(on_flush_clicked),
            NULL);
    gtk_grid_attach(GTK_GRID(grid), flush_button, 2, 1, 1, 1);

    gtk_widget_show_all(grid);
    return grid;
}



/** \brief  Create widget to control EEPROM resources
 *
 * \return  GtkGrid
 */
static GtkWidget *create_eeprom_image_widget(void)
{
    GtkWidget *grid;
    GtkWidget *readwrite;
    GtkWidget *label;
    GtkWidget *browse;

    grid = vice_gtk3_grid_new_spaced_with_label(
            -1, -1, "MMC Replay EEPROM image", 3);

    label = gtk_label_new("file name");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    gtk_widget_set_margin_start(label, 16);

    eeprom_entry = vice_gtk3_resource_entry_new("MMCREEPROMImage");
    gtk_widget_set_hexpand(eeprom_entry, TRUE);

    browse = gtk_button_new_with_label("Browse ...");
    g_signal_connect(browse, "clicked", G_CALLBACK(on_eeprom_browse_clicked),
            NULL);

    gtk_grid_attach(GTK_GRID(grid), label, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), eeprom_entry, 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), browse, 2, 1, 1, 1);


    /* add RW widget */
    readwrite = create_eeprom_rw_widget();
    gtk_widget_set_margin_start(readwrite, 16);
    gtk_grid_attach(GTK_GRID(grid), readwrite, 0, 3, 2, 1);

    gtk_widget_show_all(grid);
    return grid;
}


/** \brief  Create widget to control memory card image
 *
 * \return  GtkGrid
 */
static GtkWidget *create_card_image_widget(void)
{
    GtkWidget *grid;
    GtkWidget *label;
    GtkWidget *entry;
    GtkWidget *browse;
    GtkWidget *card_writes;

    grid = vice_gtk3_grid_new_spaced_with_label(
            -1, -1, "MMC Replay SD/MMC Card image", 3);

    label = gtk_label_new("file name");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    gtk_widget_set_margin_start(label, 16);
    gtk_grid_attach(GTK_GRID(grid), label, 0, 1, 1, 1);

    entry = vice_gtk3_resource_entry_new("MMCRCardImage");
    gtk_widget_set_hexpand(entry, TRUE);
    gtk_grid_attach(GTK_GRID(grid), entry, 1, 1, 1, 1);

    browse = gtk_button_new_with_label("Browse ...");
    gtk_grid_attach(GTK_GRID(grid), browse, 2, 1, 1, 1);

    card_writes = vice_gtk3_resource_check_button_new("MMCRCardRW",
            "Enable SD/MMC card writes");
    gtk_widget_set_margin_top(card_writes, 8);
    gtk_widget_set_margin_start(card_writes, 16);
    gtk_grid_attach(GTK_GRID(grid), card_writes, 0, 2, 3, 1);

    g_signal_connect(browse, "clicked", G_CALLBACK(on_card_browse_clicked),
            NULL);

    gtk_widget_show_all(grid);
    return grid;
}


/** \brief  Create widget to control memory card type
 *
 * \return  GtkGrid
 */
static GtkWidget *create_card_type_widget(void)
{
    GtkWidget *grid;
    GtkWidget *radio_group;
    GtkWidget *label;

    grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid), 8);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 8);
    label = gtk_label_new("Card type");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    gtk_widget_set_margin_start(label, 16);
    gtk_grid_attach(GTK_GRID(grid), label, 0, 0, 1, 1);

    radio_group = vice_gtk3_resource_radiogroup_new("MMCRSDType", card_types,
            GTK_ORIENTATION_HORIZONTAL);
    gtk_grid_set_column_spacing(GTK_GRID(radio_group), 16);
    gtk_grid_attach(GTK_GRID(grid), radio_group, 1, 0, 1, 1);

    gtk_widget_show_all(grid);
    return grid;
}



/** \brief  Create widget to control MMC Replay resources
 *
 * \param[in]   parent  parent widget (used for dialogs)
 *
 * \return  GtkGrid
 */
GtkWidget *settings_mmcr_widget_create(GtkWidget *parent)
{
    GtkWidget *grid;
    GtkWidget *label;

    grid = vice_gtk3_grid_new_spaced(8, 8);

    gtk_grid_attach(GTK_GRID(grid), create_rescue_mode_widget(), 0, 0, 1, 1);
    label = gtk_label_new("ClockPort device");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    gtk_widget_set_margin_start(label, 16);
    gtk_grid_attach(GTK_GRID(grid), label, 1, 0, 1, 1);

    gtk_grid_attach(GTK_GRID(grid), create_clockport_widget(), 2, 0, 1, 1);

    gtk_grid_attach(GTK_GRID(grid), create_cart_image_widget(), 0, 1, 3, 1);

    gtk_grid_attach(GTK_GRID(grid), create_eeprom_image_widget(), 0, 2, 3, 1);

    card_widget = create_card_image_widget();
    gtk_grid_attach(GTK_GRID(grid), card_widget, 0, 3, 3, 1);

    gtk_grid_attach(GTK_GRID(grid), create_card_type_widget(), 0, 4, 3, 1);

    gtk_widget_show_all(grid);
    return grid;
}
