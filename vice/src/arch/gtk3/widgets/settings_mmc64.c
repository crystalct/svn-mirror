/** \file   settings_mmc64.c
 * \brief   Settings widget to control MMC64 resources
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 */

/*
 * $VICERES MMC64               x64 x64sc xscpu64 x128
 * $VICERES MMC64BIOSfilename   x64 x64sc xscpu64 x128
 * $VICERES MMC64_bios_write    x64 x64sc xscpu64 x128
 * $VICERES MMC64_flashjumper   x64 x64sc xscpu64 x128
 * $VICERES MMC64_revision      x64 x64sc xscpu64 x128
 * $VICERES MMC64imagefilename  x64 x64sc xscpu64 x128
 * $VICERES MMC64_RO            x64 x64sc xscpu64 x128
 * $VICERES MMC64_sd_type       x64 x64sc xscpu64 x128
 * $VICERES MMC64ClockPort      x64 x64sc xscpu64 x128
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
#include "c64cart.h"
#include "cartridge.h"
#include "log.h"
#include "machine.h"
#include "resources.h"
#include "ui.h"

#include "settings_mmc64.h"


/** \brief  List of revisions
 */
static const vice_gtk3_radiogroup_entry_t revisions[] = {
    { "Rev. A", MMC64_REV_A },
    { "Rev. B", MMC64_REV_B },
    { NULL, -1 }
};


/** \brief  List of memory card types
 */
static const vice_gtk3_radiogroup_entry_t card_types[] = {
    { "Auto", MMC64_TYPE_AUTO },
    { "MMC", MMC64_TYPE_MMC },
    { "SD", MMC64_TYPE_SD },
    { "SDHC", MMC64_TYPE_SDHC },
    { NULL, -1 }
};


/** \brief  Enable checkbox */
static GtkWidget *enable_widget = NULL;

/** \brief  Jumper switch */
static GtkWidget *jumper_widget = NULL;

/** \brief  Revision radiogroup */
static GtkWidget *revision_widget = NULL;

/** \brief  Clockport device widget */
static GtkWidget *clockport_widget = NULL;

/** \brief  BIOS filename entry */
static GtkWidget *bios_filename_widget = NULL;

/** \brief  BIOS browse button */
static GtkWidget *bios_browse_widget = NULL;

/** \brief  BIOS write checkbox */
static GtkWidget *bios_write_widget = NULL;

/** \brief  SD card widget */
static GtkWidget *card_widget = NULL;

/** \brief  SD card widget filename entry */
static GtkWidget *card_filename_entry = NULL;

/** \brief  SD card widget type widget */
static GtkWidget *card_type_widget = NULL;

/** \brief  Save button */
static GtkWidget *save_button = NULL;

/** \brief  Flush button */
static GtkWidget *flush_button = NULL;


/** \brief  Callback for the BIOS file dilaog
 *
 * \param[in,out]   dialog      BIOS file dialog
 * \param[in]       filename    filename or NULL on cancel
 * \param[in]       data        extra data (unused)
 */
static void bios_filename_callback(GtkDialog *dialog,
                                   gchar *filename,
                                   gpointer data)
{
    if (filename != NULL) {
        vice_gtk3_resource_entry_set(bios_filename_widget, filename);
        g_free(filename);
    }
    gtk_widget_destroy(GTK_WIDGET(dialog));
}


/** \brief  Handler for the "clicked" event of the BIOS browse button
 *
 * \param[in]   button      browse button
 * \param[in]   user_data   BIOS text entry
 */
static void on_bios_browse_clicked(GtkWidget *button, gpointer user_data)
{
    vice_gtk3_open_file_dialog(
            "Open MMC64 BIOS image file",
            NULL, NULL, NULL,
            bios_filename_callback,
            NULL);
}


/** \brief  Callback for the card file dilaog
 *
 * \param[in,out]   dialog      card file dialog
 * \param[in]       filename    filename or NULL on cancel
 * \param[in]       data        extra data (unused)
 */
static void card_filename_callback(GtkDialog *dialog,
                                   gchar *filename,
                                   gpointer data)
{
    if (filename != NULL) {
        vice_gtk3_resource_entry_set(card_filename_entry, filename);
        g_free(filename);
    }

    gtk_widget_destroy(GTK_WIDGET(dialog));
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


/** \brief  Handler for the 'toggled' event of the "MMC64 Enabled" widget
 *
 * \param[in,out]   check       check button
 * \param[in]       user_data   extra event data (unused)
 */
static void on_enable_toggled(GtkWidget *check, gpointer user_data)
{
    int state = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check));
    const char *bios = gtk_entry_get_text(GTK_ENTRY(bios_filename_widget));

    if (state && (bios == NULL || *bios == '\0')) {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check), FALSE);
        vice_gtk3_message_error("VICE core error",
                "Cannot enable cartridge due to missing BIOS file");
        return;
    }

    /* TODO: this requires proper logging or error dialogs, not debug_gtk3() */
    if (state && bios != NULL && *bios != '\0') {
        if (cartridge_enable(CARTRIDGE_MMC64) < 0) {
            /* failed to set resource */
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check), FALSE);
            log_error(LOG_ERR, "failed to activate MMC64, please set BIOS file.");
        }
        /* doesn't work, attaching for example a KCS Power Cart will still
         * return 37 (MMC64) */
        if (!cartridge_type_enabled(CARTRIDGE_MMC64)) {
            debug_gtk3("failed to attach MMC64.");
        }
    } else if (!state) {
        if (cartridge_disable(CARTRIDGE_MMC64) < 0) {
            log_error(LOG_ERR, "failed to disable cartridge.");
        }
    }
}


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
        if (cartridge_save_image(CARTRIDGE_MMC64, filename) < 0) {
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
 * \param[in]   widget      button
 * \param[in]   user_data   unused
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
    if (cartridge_flush_image(CARTRIDGE_MMC64) < 0) {
        vice_gtk3_message_error("Flushing failed",
                    "Failed to fush cartridge image");
    }
}


/** \brief  Create widget to toggle the MMC64 on/off
 *
 * \return  GtkCheckButton
 */
static GtkWidget *create_mmc64_enable_widget(void)
{
    GtkWidget *check;

    check = gtk_check_button_new_with_label("Enable MMC64");

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
            cartridge_type_enabled(CARTRIDGE_MMC64));

    g_signal_connect(check, "toggled", G_CALLBACK(on_enable_toggled), NULL);
    return check;
}


/** \brief  Create widget to toggle the MMC64 flash jumper
 *
 * \return  GtkCheckButton
 */
static GtkWidget *create_mmc64_jumper_widget(void)
{
    GtkWidget *check;

    check = vice_gtk3_resource_check_button_new("MMC64_flashjumper",
            "Enable flash jumper");
    return check;
}


/** \brief  Create button to save the cartridge (BIOS) image
 *
 * \return  GtkButton
 */
static GtkWidget *create_save_button(void)
{
    GtkWidget *button = gtk_button_new_with_label("Save image as ...");
    g_signal_connect(button, "clicked", G_CALLBACK(on_save_clicked), NULL);
    return button;
}


/** \brief  Create button to flush the cartridge (BIOS) image
 *
 * \return  GtkButton
 */
static GtkWidget *create_flush_button(void)
{
    GtkWidget *button = gtk_button_new_with_label("Save image");
    g_signal_connect(button, "clicked", G_CALLBACK(on_flush_clicked), NULL);
    return button;
}



/** \brief  Create widget to set the MMC64 revision
 *
 * \return  GtkGrid
 */
static GtkWidget *create_mmc64_revision_widget(void)
{
    GtkWidget *grid;
    GtkWidget *label;
    GtkWidget *radio_group;

    grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid), 16);

    label = gtk_label_new("Revision");
    gtk_grid_attach(GTK_GRID(grid), label, 0, 0, 1, 1);

    radio_group = vice_gtk3_resource_radiogroup_new("MMC64_revision", revisions,
            GTK_ORIENTATION_HORIZONTAL);
    gtk_grid_attach(GTK_GRID(grid), radio_group, 1, 0, 1, 1);

    gtk_widget_show_all(grid);
    return grid;
}


/** \brief  Create widget to control BIOS resources
 *
 * \return  GtkGrid
 */
static GtkWidget *create_bios_image_widget(void)
{
    GtkWidget *grid;
    GtkWidget *label;

    grid = vice_gtk3_grid_new_spaced_with_label(-1, -1, "MMC64 BIOS image", 2);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 16);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 8);

    label = gtk_label_new("file name");
    bios_filename_widget = vice_gtk3_resource_entry_new("MMC64BIOSfilename");
    gtk_widget_set_hexpand(bios_filename_widget, TRUE);
    gtk_widget_set_margin_start(label, 16);
    bios_browse_widget = gtk_button_new_with_label("Browse ...");
    bios_write_widget = vice_gtk3_resource_check_button_new(
            "MMC64_bios_write", "Enable BIOS image writes");
    gtk_widget_set_margin_start(bios_write_widget, 16);

    gtk_grid_attach(GTK_GRID(grid), label, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), bios_filename_widget, 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), bios_browse_widget, 2, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), bios_write_widget, 0, 2, 3, 1);

    g_signal_connect(bios_browse_widget, "clicked",
            G_CALLBACK(on_bios_browse_clicked),
            (gpointer)bios_filename_widget);

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
    GtkWidget *browse;
    GtkWidget *card_writes;

    grid = vice_gtk3_grid_new_spaced_with_label(
            -1, -1, "MMC64 SD/MMC Card image", 3);

    label = gtk_label_new("file name");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    gtk_widget_set_margin_start(label, 16);
    gtk_grid_attach(GTK_GRID(grid), label, 0, 1, 1, 1);

    card_filename_entry = vice_gtk3_resource_entry_new("MMC64Imagefilename");
    gtk_widget_set_hexpand(card_filename_entry, TRUE);
    gtk_grid_attach(GTK_GRID(grid), card_filename_entry, 1, 1, 1, 1);

    browse = gtk_button_new_with_label("Browse ...");
    gtk_grid_attach(GTK_GRID(grid), browse, 2, 1, 1, 1);

    card_writes = vice_gtk3_resource_check_button_new("MMC64_RO",
            "Enable SD/MMC card read-only");
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

    grid = vice_gtk3_grid_new_spaced(VICE_GTK3_DEFAULT, VICE_GTK3_DEFAULT);
    label = gtk_label_new("Card type");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    gtk_widget_set_margin_start(label, 16);
    gtk_grid_attach(GTK_GRID(grid), label, 0, 0, 1, 1);

    radio_group = vice_gtk3_resource_radiogroup_new("MMC64_sd_type",
            card_types, GTK_ORIENTATION_HORIZONTAL);
    gtk_grid_set_column_spacing(GTK_GRID(radio_group), 16);
    gtk_grid_attach(GTK_GRID(grid), radio_group, 1, 0, 1, 1);

    gtk_widget_show_all(grid);
    return grid;
}


/** \brief  Create widget to select the clockport device
 *
 * \return  GtkGrid
 */
static GtkWidget *create_clockport_widget(void)
{
    GtkWidget *grid;
    GtkWidget *label;

    grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid), 8);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 8);

    label = gtk_label_new("ClockPort device");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), label, 0, 0, 1, 1);

    gtk_grid_attach(GTK_GRID(grid),
            clockport_device_widget_create("MMC64ClockPort"), 1, 0, 1, 1);

    gtk_widget_show_all(grid);
    return grid;
}



/** \brief  Create widget to control MMC64 resources
 *
 * \param[in]   parent  parent widget (unused)
 *
 * \return  GtkGrid
 */
GtkWidget *settings_mmc64_widget_create(GtkWidget *parent)
{
    GtkWidget *grid;

    grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid), 8);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 8);

    enable_widget = create_mmc64_enable_widget();
    gtk_grid_attach(GTK_GRID(grid), enable_widget, 0, 0, 2 ,1);

    gtk_grid_attach(GTK_GRID(grid), create_bios_image_widget(),
            0, 1, 2, 1);
    card_widget = create_card_image_widget();
    gtk_grid_attach(GTK_GRID(grid), card_widget, 0, 2, 2, 1);

    card_type_widget = create_card_type_widget();
    gtk_widget_set_margin_start(card_type_widget, 16);
    gtk_widget_set_margin_bottom(card_type_widget, 16);
    gtk_grid_attach(GTK_GRID(grid), card_type_widget, 0, 3, 2, 1);

    jumper_widget = create_mmc64_jumper_widget();
    gtk_grid_attach(GTK_GRID(grid), jumper_widget, 0, 4, 2, 1);

    revision_widget = create_mmc64_revision_widget();
    gtk_grid_attach(GTK_GRID(grid), revision_widget, 0, 5, 2, 1);

    clockport_widget = create_clockport_widget();
    gtk_grid_attach(GTK_GRID(grid), clockport_widget, 0, 6, 2, 1);

    save_button = create_save_button();
    flush_button = create_flush_button();
    gtk_grid_attach(GTK_GRID(grid), save_button, 0, 7, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), flush_button, 1, 7, 1, 1);

    gtk_widget_show_all(grid);
    return grid;
}
