/** \file   settings_rrnetmk3.c
 * \brief   Settings widget to control RRNet MK3 resourcs
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 *
 * \todo    Reimplement using cartimagehelper.c
 */

/*
 * $VICERES RRNETMK3_flashjumper    x64 x64sc xscpu64 x128
 * $VICERES RRNETMK3_bios_write     x64 x64sc xscpu64 x128
 */


/*
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
 */

#include "vice.h"
#include <gtk/gtk.h>

#include "cartridge.h"
#include "vice_gtk3.h"

#include "settings_rrnetmk3.h"


/** \brief  Callback for the save-dialog response handler
 *
 * \param[in,out]   dialog      save-file dialog
 * \param[in,out]   filename    filename
 * \param[in]       data        extra data (unused)
 */
static void save_filename_callback(GtkDialog *dialog,
                                  gchar      *filename,
                                  gpointer    data)
{
    if (filename != NULL) {
        if (cartridge_save_image(CARTRIDGE_RRNETMK3, filename) < 0) {
            vice_gtk3_message_error("RR-Net Mk3 Error",
                                    "Failed to save cartridge image '%s'",
                                    filename);
        }
        g_free(filename);
    }
    gtk_widget_destroy(GTK_WIDGET(dialog));
}

/** \brief  Handler for the "clicked" event of the "Save As" button
 *
 * \param[in]   widget      button
 * \param[in]   user_data   extra event data (unused)
 */
static void on_save_clicked(GtkWidget *widget, gpointer user_data)
{
    GtkWidget *dialog;

    dialog = vice_gtk3_save_file_dialog("Save image as",
                                        NULL,
                                        TRUE,
                                        NULL,
                                        save_filename_callback,
                                        NULL);
    gtk_widget_show(dialog);
}

/** \brief  Handler for the "clicked" event of the "Flush now" button
 *
 * \param[in]   widget      button
 * \param[in]   user_data   extra event data (unused)
 */
static void on_flush_clicked(GtkWidget *widget, gpointer user_data)
{
    if (cartridge_flush_image(CARTRIDGE_RRNETMK3) < 0) {
        vice_gtk3_message_error("RR-Net Mk3 Error",
                                "Failed to flush RR-Net Mk3 image.");
    }
}


/** \brief  Create widget to control RRNet Mk3 resources
 *
 * \param[in]   parent  parent widget (unused)
 *
 * \return  GtkGrid
 */
GtkWidget *settings_rrnetmk3_widget_create(GtkWidget *parent)
{
    GtkWidget *grid;
    GtkWidget *flash_label;
    GtkWidget *flash_jumper;
    GtkWidget *bios_write;
    GtkWidget *save_button;
    GtkWidget *flush_button;

    grid = vice_gtk3_grid_new_spaced(8, 8);

    flash_label  = gtk_label_new("RR-Net Mk3 flash jumper");
    flash_jumper = vice_gtk3_resource_switch_new("RRNETMK3_flashjumper");
    gtk_widget_set_halign(flash_label, GTK_ALIGN_START);
    gtk_widget_set_hexpand(flash_label, FALSE);
    gtk_widget_set_halign(flash_jumper, GTK_ALIGN_START);
    gtk_widget_set_valign(flash_jumper, GTK_ALIGN_CENTER);

    /* RRBiosWrite */
    bios_write = vice_gtk3_resource_check_button_new("RRNETMK3_bios_write",
            "Write back RR-Net Mk3 Flash ROM image automatically");
    gtk_widget_set_margin_top(bios_write, 8);

    /* Save image as... */
    save_button = gtk_button_new_with_label("Save image as ...");
    gtk_widget_set_halign(save_button, GTK_ALIGN_END);
    gtk_widget_set_hexpand(save_button, TRUE);
    gtk_widget_set_margin_top(save_button, 8);
    g_signal_connect(save_button,
                     "clicked",
                     G_CALLBACK(on_save_clicked),
                     NULL);

    /* Flush image now */
    flush_button = gtk_button_new_with_label("Save image now");
    gtk_widget_set_halign(flush_button, GTK_ALIGN_END);
    gtk_widget_set_hexpand(flush_button, TRUE);
    g_signal_connect(flush_button,
                     "clicked",
                     G_CALLBACK(on_flush_clicked),
                     NULL);

    gtk_grid_attach(GTK_GRID(grid), flash_label,  0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), flash_jumper, 1, 0, 3, 1);
    gtk_grid_attach(GTK_GRID(grid), bios_write,   0, 1, 3, 1);
    gtk_grid_attach(GTK_GRID(grid), save_button,  3, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), flush_button, 3, 2, 1, 1);

    gtk_widget_show_all(grid);
    return grid;
}
