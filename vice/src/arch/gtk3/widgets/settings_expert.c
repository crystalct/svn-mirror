/** \file   settings_expert.c
 * \brief   Settings widget to control Expert Cartridge resources
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 */

/*
 * Controls the following resource(s):
 *
 * $VICERES     ExpertCartridgeEnabled  x64 x64sc xscpu64 x128
 * $VICERES     ExpertCartridgeMode     x64 x64sc xscpu64 x128
 * $VICERES     Expertfilename          x64 x64sc xscpu64 x128
 * $VICERES     ExpertImageWrite        x64 x64sc xscpu64 x128
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

#include "c64cart.h"
#include "cartridge.h"
#include "vice_gtk3.h"

#include "settings_expert.h"


/** \brief  List of 'modes' for the Expert Cartridge
 */
static const vice_gtk3_radiogroup_entry_t mode_list[] = {
    { "Off",            EXPERT_MODE_OFF },
    { "Programmable",   EXPERT_MODE_PRG },
    { "On",             EXPERT_MODE_ON },
    { NULL,             -1 }
};


/** \brief  Create Expert Cartridge mode widget
 *
 * \return  GtkCheckButton
 */
static GtkWidget *create_expert_mode_widget(void)
{
    GtkWidget *grid;
    GtkWidget *group;

    grid = vice_gtk3_grid_new_spaced_with_label(8, 8, "Cartridge mode", 1);
    group = vice_gtk3_resource_radiogroup_new("ExpertCartridgeMode",
                                              mode_list,
                                              GTK_ORIENTATION_VERTICAL);
    gtk_widget_set_margin_start(group, 8);
    gtk_grid_attach(GTK_GRID(grid), group, 0, 1, 1, 1);
    gtk_widget_show_all(grid);
    return grid;
}

/** \brief  Create widget to load/save Expert Cartridge image file
 *
 * \return  GtkGrid
 */
static GtkWidget *create_expert_image_widget(void)
{
    return cart_image_widget_create(NULL,
                                    NULL,
                                    "Expertfilename",
                                    "ExpertImageWrite",
                                    CARTRIDGE_NAME_EXPERT,
                                    CARTRIDGE_EXPERT);
}


/** \brief  Create widget to control Expert Cartridge resources
 *
 * \param[in]   parent  parent widget (unused)
 *
 * \return  GtkGrid
 */
GtkWidget *settings_expert_widget_create(GtkWidget *parent)
{
    GtkWidget *grid;
    GtkWidget *enable;
    GtkWidget *image;
    GtkWidget *mode;

    grid = vice_gtk3_grid_new_spaced(8, 16);

    enable = carthelpers_create_enable_check_button(CARTRIDGE_NAME_EXPERT,
                                                    CARTRIDGE_EXPERT);
    mode   = create_expert_mode_widget();
    image  = create_expert_image_widget();

    gtk_grid_attach(GTK_GRID(grid), enable, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), mode,   0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), image,  0, 2, 1, 1);

    gtk_widget_show_all(grid);
    return grid;
}
