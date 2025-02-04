/** \file   cartimagewidget.c
 * \brief   Widget to control load/save/flush for cart images
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
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

#include "basedialogs.h"
#include "basewidgets.h"
#include "cartridge.h"
#include "debug_gtk3.h"
#include "lastdir.h"
#include "machine.h"
#include "openfiledialog.h"
#include "resources.h"
#include "savefiledialog.h"
#include "ui.h"
#include "widgethelpers.h"

#include "cartimagewidget.h"


/** \brief  Cartridge name
 *
 * Used in messages.
 */
static const char *crt_name;

/** \brief  Cartridge ID
 *
 * Used for various cartridge functions
 */
static int crt_id;  /**< cartridge ID in cartridge_*() calls */

/** \brief  Name of resource containing the cartridge filename */
static const char *res_fname;

/** \brief  Name of resource containing the flush-on-write setting */
static const char *res_write;

/** \brief  Reference to the filename entry widget */
static GtkWidget *filename_entry;

static char *last_dir = NULL;
static char *last_file = NULL;


/** \brief  Callback for the open/create-file dialog
 *
 * \param[in,out]   dialog      open/create dialog
 * \param[in,out]   filename    filename
 * \param[in]       data        extra data (unused)
 */
static void browse_filename_callback(GtkDialog *dialog,
                                     gchar     *filename,
                                     gpointer   data)
{
    if (filename != NULL) {
        vice_gtk3_resource_entry_set(filename_entry, filename);
        g_free(filename);
        lastdir_update(GTK_WIDGET(dialog), &last_dir, &last_file);
    }
    gtk_widget_destroy(GTK_WIDGET(dialog));
}

/** \brief  Handler for the "clicked" event of the "browse" button
 *
 * Select an image file for the extension.
 *
 * \param[in]   button      browse button
 * \param[in]   user_data   unused
 */
static void on_browse_clicked(GtkWidget *button, gpointer user_data)
{
    GtkWidget *dialog;
    char       title[256];

    g_snprintf(title, sizeof title, "Open or create %s image file", crt_name);
    dialog = vice_gtk3_open_create_file_dialog(title,
                                               NULL,
                                               FALSE,
                                               NULL,
                                               browse_filename_callback,
                                               NULL);
    lastdir_set(dialog, &last_dir, &last_file);
    gtk_widget_show(dialog);
}

/** \brief  Callback for the save-dialog
 *
 * \param[in,out]   dialog      save-file dialog
 * \param[in,out]   filename    path to file to save
 * \param[in]       data        extra data (unused)
 */
static void save_filename_callback(GtkDialog *dialog,
                                   gchar     *filename,
                                   gpointer   data)
{
    debug_gtk3("Called with '%s'\n", filename);
    if (filename != NULL) {
        /* write file */
        if (cartridge_save_image(crt_id, filename) < 0) {
            /* oops */
            vice_gtk3_message_error("I/O error", "Failed to save '%s'", filename);
        }
        g_free(filename);
        lastdir_update(GTK_WIDGET(dialog), &last_dir, &last_file);
    }
    gtk_widget_destroy(GTK_WIDGET(dialog));
}

/** \brief  Handler for the 'clicked' event of the "save" button
 *
 * Opens a file chooser to save the cartridge.
 *
 * \param[in]   button      save button
 * \param[in]   user_data   extra event data (unused)
 */
static void on_save_clicked(GtkWidget *button, gpointer user_data)
{
    GtkWidget *dialog;
    char       buffer[256];

    g_snprintf(buffer, sizeof buffer, "Save %s image file", crt_name);
    dialog = vice_gtk3_save_file_dialog(buffer,
                                        NULL,
                                        TRUE,
                                        NULL,
                                        save_filename_callback,
                                        NULL);
    lastdir_set(dialog, &last_dir, &last_file);
    gtk_widget_show(dialog);
}

/** \brief  Handler for the "clicked" event of the "Flush image" button
 *
 * \param[in]   widget      button triggering the event
 * \param[in]   user_data   unused
 */
static void on_flush_clicked(GtkWidget *widget, gpointer user_data)
{
    if (cartridge_flush_image(crt_id) < 0) {
        vice_gtk3_message_error("I/O error", "Failed to flush image");
    }
}


/** \brief  Create widget to load/save/flush cart image file
 *
 * Create cartridge widget to do basic operations like saving and flushing.
 *
 * If \a title is `NULL` the title will be set to "\<\a cartname\> image".
 *
 * \param[in]   parent          parent widget (unused)
 * \param[in]   title           widget title (can be NULL)
 * \param[in]   resource_fname  resource for the image file name
 * \param[in]   resource_write  resource controlling flush-on-exit/detach
 * \param[in]   cart_name       cartridge name to use in dialogs
 * \param[in]   cart_id         cartridge ID to use in save/flush callbacks
 *
 * \note    \a cartname and \a cart_id should be taken from cartridge.h
 *
 *
 * \return  GtkGrid
 */
GtkWidget *cart_image_widget_create(GtkWidget  *parent,
                                    const char *title,
                                    const char *resource_fname,
                                    const char *resource_write,
                                    const char *cart_name,
                                    int         cart_id)
{
    GtkWidget *grid;
    GtkWidget *label;
    GtkWidget *browse;
    GtkWidget *auto_save;
    GtkWidget *save_button;
    GtkWidget *flush_button;
    char       buffer[256];

    res_fname = resource_fname;
    res_write = resource_write;
    crt_name  = cart_name;
    crt_id    = cart_id;

    if (title == NULL) {
        if (cart_name == NULL) {
            strcpy(buffer, "Cartridge Image");
        } else {
            g_snprintf(buffer, sizeof buffer, "%s Image", cart_name);
        }
    } else {
        strncpy(buffer, title, sizeof buffer);
        buffer[sizeof buffer - 1u] = '\0';
    }

    grid = vice_gtk3_grid_new_spaced_with_label(8, 8, buffer, 3);

    label = gtk_label_new("File name:");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    gtk_widget_set_margin_start(label, 8);
    filename_entry = vice_gtk3_resource_entry_new(resource_fname);
    gtk_widget_set_hexpand(filename_entry, TRUE);
    /* gtk_widget_set_sensitive(entry, FALSE); */
    browse = gtk_button_new_with_label("Browse ...");

    gtk_grid_attach(GTK_GRID(grid), label,          0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), filename_entry, 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), browse,         2, 1, 1, 1);

    auto_save = vice_gtk3_resource_check_button_new(resource_write,
            "Write image on image detach/emulator exit");
    gtk_widget_set_margin_start(auto_save, 8);
    gtk_grid_attach(GTK_GRID(grid), auto_save, 0, 2, 2, 1);

    save_button = gtk_button_new_with_label("Save as ...");
    gtk_grid_attach(GTK_GRID(grid), save_button, 2, 2, 1, 1);

    flush_button = gtk_button_new_with_label("Save image");
    gtk_grid_attach(GTK_GRID(grid), flush_button, 2, 3, 1, 1);
    gtk_widget_set_sensitive(flush_button,
                             (gboolean)(cartridge_can_flush_image(cart_id)));
    gtk_widget_set_sensitive(save_button,
                             (gboolean)(cartridge_can_save_image(cart_id)));

    g_signal_connect(browse,
                     "clicked",
                     G_CALLBACK(on_browse_clicked),
                     NULL);
    g_signal_connect(save_button,
                     "clicked",
                     G_CALLBACK(on_save_clicked),
                     NULL);
    g_signal_connect(flush_button,
                     "clicked",
                     G_CALLBACK(on_flush_clicked),
                     NULL);

    gtk_widget_show_all(grid);
    return grid;
}


/** \brief  Clean up resouces used by the cartridge image widget
 */
void cart_image_widget_shutdown(void)
{
    lastdir_shutdown(&last_dir, &last_file);
}
