/** \file   settings_burstmode.c
 * \brief   Settings widget for Burst Mode Modification
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 */

/*
 * $VICERES BurstMod    x64 x64sc xscpu64
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

#include "c64fastiec.h"
#include "vice_gtk3.h"

#include "settings_burstmode.h"


/** \brief  List of burst modes
 */
static const vice_gtk3_radiogroup_entry_t burst_modes[] = {
    { "None",   BURST_MOD_NONE },
    { "CIA1",   BURST_MOD_CIA1 },
    { "CIA2",   BURST_MOD_CIA2 },
    { NULL,     -1 }
};


/** \brief  Create Burst Mode Modification widget
 *
 * \param[in]   parent  parent widget
 *
 * \return  GtkGrid
 */
GtkWidget *settings_burstmode_widget_create(GtkWidget *parent)
{
    GtkWidget *grid;
    GtkWidget *group;

    grid = vice_gtk3_grid_new_spaced_with_label(8, 0, "Burst Mode Modification", 1);
    vice_gtk3_grid_set_title_margin(grid, 8);
    group = vice_gtk3_resource_radiogroup_new("BurstMod",
                                              burst_modes,
                                              GTK_ORIENTATION_VERTICAL);
    gtk_widget_set_margin_start(group, 8);
    gtk_grid_attach(GTK_GRID(grid), group, 0, 1, 1, 1);

    gtk_widget_show_all(grid);
    return grid;
}
