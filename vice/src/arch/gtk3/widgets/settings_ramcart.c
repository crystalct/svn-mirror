/** \file   settings_ramcart.c
 * \brief   Settings widget to control RamCart resources
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 */

/*
 * $VICERES RAMCART             x64 x64sc xscpu64 x128
 * $VICERES RAMCARTsize         x64 x64sc xscpu64 x128
 * $VICERES RAMCARTfilename     x64 x64sc xscpu64 x128
 * $VICERES RAMCARTImageWrite   x64 x64sc xscpu64 x128
 * $VICERES RAMCART_RO          x64 x64sc xscpu64 x128
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

#include "cartridge.h"
#include "vice_gtk3.h"

#include "settings_ramcart.h"


/** \brief  List of supported RAM sizes in KiB */
static int ram_sizes[] = { 64, 128, -1 };


/** \brief  Create RAMCART enable check button
 *
 * \return  GtkCheckButton
 */
static GtkWidget *create_ramcart_enable_widget(void)
{
    return vice_gtk3_resource_check_button_new("RAMCART",
                                               "Enable RAMCART expansion");
}

/** \brief  Create check button to toggle read-only mode
 *
 * \return  GtkCheckButton
 */
static GtkWidget *create_ramcart_readonly_widget(void)
{
    return vice_gtk3_resource_check_button_new("RAMCART_RO",
                                               "RAMCART contents are read only");
}

/** \brief  Create radio button group to determine RAMCART size
 *
 * \return  GtkGrid
 */
static GtkWidget *create_ramcart_size_widget(void)
{
    return ram_size_radiogroup_new("RAMCARTSize",
                                   "RAM size",
                                   ram_sizes);
}

/** \brief  Create widget to load/save RAMCART image file
 *
 * \return  GtkGrid
 */
static GtkWidget *create_ramcart_image_widget(void)
{
    return cart_image_widget_create(NULL,
                                    NULL,
                                    "RAMCARTfilename",
                                    "RAMCARTImageWrite",
                                    CARTRIDGE_NAME_RAMCART,
                                    CARTRIDGE_RAMCART);
}


/** \brief  Create widget to control RAM Expansion Module resources
 *
 * \param[in]   parent  parent widget (unused)
 *
 * \return  GtkGrid
 */
GtkWidget *settings_ramcart_widget_create(GtkWidget *parent)
{
    GtkWidget *grid;
    GtkWidget *ramcart_enable;
    GtkWidget *ramcart_size;
    GtkWidget *ramcart_readonly;
    GtkWidget *ramcart_image;

    grid = vice_gtk3_grid_new_spaced(32, 8);

    ramcart_enable = create_ramcart_enable_widget();
    gtk_grid_attach(GTK_GRID(grid), ramcart_enable, 0, 0, 2, 1);

    ramcart_size = create_ramcart_size_widget();
    gtk_grid_attach(GTK_GRID(grid), ramcart_size, 0, 1, 1, 1);

    ramcart_image = create_ramcart_image_widget();
    gtk_grid_attach(GTK_GRID(grid), ramcart_image, 1, 1, 1, 1);

    ramcart_readonly = create_ramcart_readonly_widget();
    gtk_grid_attach(GTK_GRID(grid), ramcart_readonly, 0, 2, 2,1);

    gtk_widget_show_all(grid);
    return grid;
}
