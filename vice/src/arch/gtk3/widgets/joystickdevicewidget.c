/** \file   joystickdevicewidget.c
 * \brief   Widget to select a joystick device
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 */

/*
 * $VICERES JoyDevice1      -xcbm2 -xpet -vsid
 * $VICERES JoyDevice2      -xcbm2 -xpet -vsid
 * $VICERES JoyDevice3      -xcbm5x0 -vsid
 * $VICERES JoyDevice4      -xcbm5x0 -xplus4 -vsid
 * $VICERES JoyDevice5      xplus4
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
#include <stdlib.h>

#include "vice_gtk3.h"
#include "joystick.h"
#include "lib.h"
#include "machine.h"
#include "resources.h"
#include "filechooserhelpers.h"

#include "joystickdevicewidget.h"


/** \brief  Struct containing device name and id
 */
typedef struct device_info_s {
    const char *name;   /**< device name */
    int         id;     /**< device ID (\see joy.h) */
} device_info_t;


/** \brief  List of available input devices on the host
 */
static const device_info_t predefined_device_list[] = {
    { "None",       JOYDEV_NONE },
    { "Numpad",     JOYDEV_NUMPAD },
    { "Keyset A",   JOYDEV_KEYSET1 },
    { "Keyset B",   JOYDEV_KEYSET2 },
    { NULL,         -1 }
};


/** \brief  Handler for the "changed" event of the combo box
 *
 * \param[in]   combo       combo box
 * \param[in]   user_data   device number (0-4) (int`)
 */
static void on_device_changed(GtkComboBoxText *combo, gpointer user_data)
{
    const char *id_str;
    int         id_val;
    char       *endptr;
    int         device;

    id_str = gtk_combo_box_get_active_id(GTK_COMBO_BOX(combo));
    id_val = (int)strtol(id_str, &endptr, 10);
    device = GPOINTER_TO_INT(user_data) + 1;

    if (*endptr == '\0') {
        resources_set_int_sprintf("JoyDevice%d", id_val, device);
    }
}


/** \brief  Create joystick device selection widget
 *
 * \param[in]   device  device number (0-4)
 * \param[in]   title   widget title
 *
 * \return  GtkGrid
 */
GtkWidget *joystick_device_widget_create(int device, const char *title)
{
    GtkWidget  *grid;
    GtkWidget  *combo;
    int         id;
    const char *name;
    char        idstr[32];
    int         dev;
    int         current = 0;

    resources_get_int_sprintf("JoyDevice%d", &current, device + 1);

    grid = vice_gtk3_grid_new_spaced_with_label(8, 0, title, 1);
    vice_gtk3_grid_set_title_margin(grid, 8);

    combo = gtk_combo_box_text_new();
    gtk_widget_set_margin_start(combo, 8);
    gtk_widget_set_hexpand(combo, TRUE);
    /* add predefined standard devices */
    for (dev = 0; (name = predefined_device_list[dev].name) != NULL; dev++) {
        id = predefined_device_list[dev].id;

        g_snprintf(idstr, sizeof idstr, "%d", id);
        gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo),
                                  idstr,
                                  name);
        if (id == current) {
            gtk_combo_box_set_active_id(GTK_COMBO_BOX(combo), idstr);
        }
    }
    /* add more devices (joysticks) */
    joystick_ui_reset_device_list();
    while ((name = joystick_ui_get_next_device_name(&id)) != NULL) {
        /* convert name from locale to UTF-8 to be used in the list */
        char *utf8 = file_chooser_convert_from_locale(name);

        g_snprintf(idstr, sizeof idstr, "%d", id);
        gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo),
                                  idstr,
                                  utf8);
        g_free(utf8);
        if (id == current) {
            gtk_combo_box_set_active_id(GTK_COMBO_BOX(combo), idstr);
        }
    }

    g_signal_connect(combo,
                     "changed",
                     G_CALLBACK(on_device_changed),
                     GINT_TO_POINTER(device));

    gtk_grid_attach(GTK_GRID(grid), combo, 0, 1, 1, 1);
    gtk_widget_show_all(grid);
    return grid;
}


/** \brief  Set joystick device \a widget to \a id
 *
 * \param[in,out]   widget  joystick device widget
 * \param[in]       id      new value for the \a widget
 */
void joystick_device_widget_update(GtkWidget *widget, int id)
{
    GtkWidget *combo;
    char       idstr[32];

    /* turn device_id into key for the combo box */
    g_snprintf(idstr, sizeof idstr, "%d", id);

    /* get combo box widget */
    combo = gtk_grid_get_child_at(GTK_GRID(widget), 0, 1);
    if (combo != NULL && GTK_IS_COMBO_BOX_TEXT(combo)) {
        gtk_combo_box_set_active_id(GTK_COMBO_BOX(combo), idstr);
    }
}
