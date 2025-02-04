/** \file   settings_tapeport.c
 * \brief   Tape port settings dialog widget
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 */

/*
 * $VICERES VirtualDevice1          -xscpu64 -vsid
 * $VICERES VirtualDevice2          -xscpu64 -vsid -x64sc -x64 -xvic -xplus4 -xcbm2 -xcbm5x0
 * $VICERES TapePort1Device         -xscpu64 -vsid
 * $VICERES TapePort2Device         -xscpu64 -vsid -x64sc -x64 -xvic -xplus4 -xcbm2 -xcbm5x0
 * $VICERES DatasetteResetWithCPU   -xscpu64 -vsid
 * $VICERES DatasetteZeroGapDelay   -xscpu64 -vsid
 * $VICERES DatasetteSpeedTuning    -xscpu64 -vsid
 * $VICERES DatasetteTapeWobble     -xscpu64 -vsid
 * $VICERES CPClockF83Save          -xscpu64 -vsid
 * $VICERES TapecartUpdateTCRT      x64 x64sc x128
 * $VICERES TapecartOptimizeTCRT    x64 x64sc x128
 * $VICERES TapecartLogLevel        x64 x64sc x128
 * $VICERES TapecartTCRTFilename    x64 x64sc x128
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

#include "basedialogs.h"
#include "basewidgets.h"
#include "debug_gtk3.h"
#include "lib.h"
#include "machine.h"
#include "resources.h"
#include "savefiledialog.h"
#include "tapecart.h"
#include "tapeport.h"
#include "ui.h"
#include "widgethelpers.h"

#include "settings_tapeport.h"

/** \brief  Column indexes in the tapeport devices model
 */
enum {
    COL_DEVICE_ID,          /**< device ID (int) */
    COL_DEVICE_NAME,        /**< device name (str) */
    COL_DEVICE_TYPE_ID,     /**< device type (int) */
    COL_DEVICE_TYPE_DESC    /**< device type description (str) */
};

/** \brief  List of log levels and their descriptions for the Tapecart
 */
static vice_gtk3_combo_entry_int_t tcrt_loglevels[] = {
    { "0 (errors only)",                            0 },
    { "1 (0 plus mode changes and command bytes)",  1 },
    { "2 (1 plus command parameter details)",       2 },
    { NULL, -1 }
};


/*
 * Reference to widgets to be able to enable/disabled them through event
 * handlers
 */

/** \brief  Tape port #1 device combo */
static GtkWidget *port1_type = NULL;

/** \brief  Tape port #2 device combo */
static GtkWidget *port2_type = NULL;

/** \brief  Datasette 1 device traps toggle button */
static GtkWidget *ds_traps1 = NULL;

/** \brief  Datasette 2 device traps toggle button */
static GtkWidget *ds_traps2 = NULL;

/** \brief  Datasette reset toggle button */
static GtkWidget *ds_reset = NULL;

/** \brief  Datasette zerogap delay spine button */
static GtkWidget *ds_zerogap = NULL;

/** \brief  Datasette speed tuning spin button */
static GtkWidget *ds_speed = NULL;

/** \brief  Datasette wobble frequency spin button */
static GtkWidget *ds_wobblefreq = NULL;

/** \brief  Datasette wobble amplitude spin button */
static GtkWidget *ds_wobbleamp = NULL;

/** \brief  Datasette align spin button */
static GtkWidget *ds_align = NULL;

/** \brief  Datasette sound emulation toggle button */
static GtkWidget *ds_sound = NULL;

/** \brief  F83 RTC toggle button */
static GtkWidget *f83_rtc = NULL;

/** \brief  Tapecart save-when-changed toggle button */
static GtkWidget *tapecart_update = NULL;

/** \brief  Tapecart optimize-when-saving toggle button */
static GtkWidget *tapecart_optimize = NULL;

/** \brief  Tapecart log level radiogroup */
static GtkWidget *tapecart_loglevel = NULL;

/** \brief  Tapecart image file browser */
static GtkWidget *tapecart_filename = NULL;

/** \brief  Tapecart flush button */
static GtkWidget *tapecart_flush = NULL;


/** \brief  Determine if current machine supports tapecart
 *
 * \return  `TRUE` if tapecart emulation is available
 */
static gboolean machine_has_tapecart(void)
{
    switch (machine_class) {
        case VICE_MACHINE_C64:      /* fall through */
        case VICE_MACHINE_C64SC:    /* fall through */
        case VICE_MACHINE_C128:
            return TRUE;
        default:
            return FALSE;
    }
}

/** \brief  Determine if the current machine has a second tape port
 *
 * \return  `TRUE` if the machine has a second tape port
 */
static gboolean machine_has_second_tape_port(void)
{
    /* only a PET has a second tape port */
    return (gboolean)(machine_class == VICE_MACHINE_PET);
}

/** \brief  Set Datasette widget active/inactive
 *
 * \param[in]   state   status
 */
static void set_datasette_active(gboolean state)
{
    gtk_widget_set_sensitive(ds_reset,      state);
    gtk_widget_set_sensitive(ds_zerogap,    state);
    gtk_widget_set_sensitive(ds_speed,      state);
    gtk_widget_set_sensitive(ds_wobblefreq, state);
    gtk_widget_set_sensitive(ds_wobbleamp,  state);
    gtk_widget_set_sensitive(ds_align,      state);
    gtk_widget_set_sensitive(ds_sound,      state);
    /* xPET does not have device traps right now, grey out the selection */
    if (machine_class == VICE_MACHINE_PET) {
        state = FALSE;
    }
    gtk_widget_set_sensitive(ds_traps1, state);
    if (machine_has_second_tape_port()) {
        gtk_widget_set_sensitive(ds_traps2, state);
    }
}

/** \brief Set CP Clock F83 widget active/inactive
 *
 * \param[in]   state   status
 */
static void set_f83_active(gboolean state)
{
    gtk_widget_set_sensitive(f83_rtc, state);
}

/** \brief  Set tapecart active/inactive
 *
 * \param[in]   state   status
 */
static void set_tapecart_active(gboolean state)
{
    if (machine_has_tapecart()) {
        gtk_widget_set_sensitive(tapecart_update,   state);
        gtk_widget_set_sensitive(tapecart_optimize, state);
        gtk_widget_set_sensitive(tapecart_loglevel, state);
        gtk_widget_set_sensitive(tapecart_filename, state);
        gtk_widget_set_sensitive(tapecart_flush,    state);
    }
}

/** \brief  Set individual options active/inactive
 *
 * \param[in]   id      id of active device
 */
static void set_options_widgets_sensitivity(int id)
{
    set_datasette_active(id == TAPEPORT_DEVICE_DATASETTE);
    set_f83_active(id == TAPEPORT_DEVICE_CP_CLOCK_F83);
    set_tapecart_active(id == TAPEPORT_DEVICE_TAPECART);
}

#if 0
/** \brief  Callback for the tapecart file chooser dialog
 *
 * \param[in,out]   dialog      file chooser dialog
 * \param[in,out]   filename    filename (NULL cancels)
 * \param[in]       data        extra data (unused)
 *
 * TODO:    Replace with resourcebrowser.c
 */
static void browse_filename_callback(GtkDialog *dialog,
                                     gchar     *filename,
                                     gpointer   data)
{
    if (filename != NULL) {
        vice_gtk3_resource_entry_full_set(tapecart_filename, filename);
        g_free(filename);
    }
    gtk_widget_destroy(GTK_WIDGET(dialog));
}

/** \brief  Handler for the 'clicked' event of the tapecart browse button
 *
 * \param[in]   widget      tapecart browse button
 * \param[in]   user_data   unused
 *
 * TODO:    Replace with resourcebrowser.c
 */
static void on_tapecart_browse_clicked(GtkWidget *widget, gpointer user_data)
{
    GtkWidget *dialog;

    /* TODO: use existing filename, if any */
    dialog = vice_gtk3_open_file_dialog("Select tapecart file",
                                        NULL,
                                        NULL,
                                        NULL,
                                        browse_filename_callback,
                                        NULL);
    gtk_widget_show(dialog);
}
#endif

/** \brief  Handler for the 'clicked' event of the tapecart flush button
 *
 * \param[in]   widget  button (unused)
 * \param[in]   data    extra event data (unused)
 */
static void on_tapecart_flush_clicked(GtkWidget *widget, gpointer data)
{
    tapecart_flush_tcrt();
}

/** \brief  Create widgets for the datasette
 *
 * TODO:    Someone needs to check the spin button bounds and steps for sane
 *          values.
 *
 * \return  GtkGrid
 */
static GtkWidget *create_datasette_widget(void)
{
    GtkWidget *grid;
    GtkWidget *label;
    int        row = 1;

    grid = vice_gtk3_grid_new_spaced_with_label(8, 0, "Datasette C2N", 4);
    vice_gtk3_grid_set_title_margin(grid, 8);

    /* device traps for datasette #1 and #2 */
    ds_traps1 = vice_gtk3_resource_check_button_new("VirtualDevice1",
                                                    "Virtual Device #1 (required for t64)");
    gtk_widget_set_margin_start(ds_traps1, 8);
    gtk_grid_attach(GTK_GRID(grid), ds_traps1, 0, row, 2, 1);
    if (machine_has_second_tape_port()) {
        ds_traps2 = vice_gtk3_resource_check_button_new("VirtualDevice2",
                                                        "Virtual Device #2 (required for t64)");
        gtk_grid_attach(GTK_GRID(grid), ds_traps2, 2, row, 2, 1);
    }
    row++;

    ds_reset = vice_gtk3_resource_check_button_new("DatasetteResetWithCPU",
                                                   "Reset datasette with CPU");
    gtk_widget_set_margin_start(ds_reset, 8);
    gtk_widget_set_margin_bottom(ds_reset, 8);
    gtk_grid_attach(GTK_GRID(grid), ds_reset, 0, row, 4, 1);
    ds_sound = vice_gtk3_resource_check_button_new("DatasetteSound",
                                                   "Datasette sound");
    gtk_widget_set_margin_bottom(ds_sound, 8);
    gtk_grid_attach(GTK_GRID(grid), ds_sound, 2, row, 2, 1);
    row++;

    label = gtk_label_new("Zero gap delay:");
    gtk_widget_set_margin_start(label, 8);
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    ds_zerogap = vice_gtk3_resource_spin_int_new("DatasetteZeroGapDelay",
                                                 0, 50000, 100);
    gtk_widget_set_margin_bottom(ds_zerogap, 8);
    gtk_grid_attach(GTK_GRID(grid), label, 0, row, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), ds_zerogap, 1, row, 1, 1);

    label = gtk_label_new("TAP v0 gap speed tuning:");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    ds_speed = vice_gtk3_resource_spin_int_new("DatasetteSpeedTuning",
                                               0, 50, 1);
    gtk_widget_set_margin_bottom(ds_speed, 8);
    gtk_grid_attach(GTK_GRID(grid), label, 2, row, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), ds_speed, 3, row, 1, 1);
    row++;

    label = gtk_label_new("Wobble frequency:");
    gtk_widget_set_margin_start(label, 8);
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    ds_wobblefreq = vice_gtk3_resource_spin_int_new("DatasetteTapeWobbleFrequency",
                                                    0, 5000, 10);
    gtk_widget_set_margin_bottom(ds_wobblefreq, 8);
    gtk_grid_attach(GTK_GRID(grid), label, 0, row, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), ds_wobblefreq, 1, row, 1, 1);

    label = gtk_label_new("Alignment error:");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    ds_align = vice_gtk3_resource_spin_int_new("DatasetteTapeAzimuthError",
                                               0, 25000, 100);
    gtk_grid_attach(GTK_GRID(grid), label, 2, row, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), ds_align, 3, row, 1, 1);
    row++;

    label = gtk_label_new("Wobble amplitude:");
    gtk_widget_set_margin_start(label, 8);
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    ds_wobbleamp = vice_gtk3_resource_spin_int_new("DatasetteTapeWobbleAmplitude",
                                                   0, 5000, 10);
    gtk_grid_attach(GTK_GRID(grid), label, 0, row, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), ds_wobbleamp, 1, row, 1, 1);

    return grid;
}

/** \brief  Create widget to handler the Cassette Port Clock F83 resources
 *
 * \return  GtkGrid
 */
static GtkWidget *create_cpcf83_widget(void)
{
    GtkWidget *grid;

    grid    = vice_gtk3_grid_new_spaced_with_label(8, 8, "Cassette Port Clock F83", 1);
    f83_rtc = vice_gtk3_resource_check_button_new("CPClockF83Save",
                                                   "Save RTC data when changed");
    gtk_widget_set_margin_start(f83_rtc, 8);
    gtk_grid_attach(GTK_GRID(grid), f83_rtc, 0, 1, 1, 1);
    gtk_widget_show_all(grid);
    return grid;
}

/** \brief  Create widget to handle the tapecart resources
 *
 * \return  GtkGrid
 */
static GtkWidget *create_tapecart_widget(void)
{
    GtkWidget  *grid;
    GtkWidget  *label;
    GtkWidget  *wrapper;
    const char *patterns[] = { "*.tcrt", NULL };
    int         row = 1;

    grid = vice_gtk3_grid_new_spaced_with_label(8, 0, "Tapecart", 3);
    vice_gtk3_grid_set_title_margin(grid, 8);

    /* TapecartTCRTFilename */
    label = gtk_label_new("TCRT Filename:");
    gtk_widget_set_margin_start(label, 8);
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), label, 0, row, 1, 1);
    tapecart_filename = vice_gtk3_resource_browser_new("TapecartTCRTFilename",
                                                       patterns,
                                                       "Tapecart images",
                                                       "Select a tapecart image",
                                                       NULL,
                                                       NULL);
    gtk_widget_set_hexpand(tapecart_filename, TRUE);
    /* expand the GtkEntry inside the filename widget */
    gtk_widget_set_hexpand(gtk_grid_get_child_at(GTK_GRID(tapecart_filename), 0, 0),
                           TRUE);
    gtk_grid_attach(GTK_GRID(grid), tapecart_filename, 1, row, 1, 1);

    tapecart_flush = gtk_button_new_with_label("Save image");
    gtk_widget_set_hexpand(tapecart_flush, FALSE);
    gtk_grid_attach(GTK_GRID(grid), tapecart_flush, 2, row, 1, 1);
    g_signal_connect(tapecart_flush,
                     "clicked",
                     G_CALLBACK(on_tapecart_flush_clicked),
                     NULL);
    row++;

    /* TCRT log level */
    label = gtk_label_new("Log level:");
    gtk_widget_set_margin_start(label, 8);
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), label, 0, row, 1, 1);
    tapecart_loglevel = vice_gtk3_resource_combo_int_new("TapecartLogLevel",
                                                             tcrt_loglevels);
    gtk_widget_set_margin_top(tapecart_loglevel, 8);
    gtk_widget_set_margin_bottom(tapecart_loglevel, 8);
    gtk_grid_attach(GTK_GRID(grid), tapecart_loglevel, 1, row, 2, 1);
    row++;

    /* wrapper for update/optimize check buttons */
    wrapper = gtk_grid_new();
    gtk_grid_set_column_homogeneous(GTK_GRID(wrapper), TRUE);
    /* TapecartUpdateTCRT */
    tapecart_update = vice_gtk3_resource_check_button_new("TapecartUpdateTCRT",
                                                          "Save data when changed");
    gtk_widget_set_margin_start(tapecart_update, 8);
    gtk_grid_attach(GTK_GRID(wrapper), tapecart_update, 0, 0, 1, 1);
    /* TapecartOptimizeTCRT */
    tapecart_optimize = vice_gtk3_resource_check_button_new("TapecartOptimizeTCRT",
                                                            "Optimize data when changed");
    gtk_grid_attach(GTK_GRID(wrapper), tapecart_optimize, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), wrapper, 0, row, 3, 1);

    gtk_widget_show_all(grid);
    return grid;
}

/** \brief  Handler for the 'changed' event of the device combobox
 *
 * Sets the active tapeport device via the "TapePort[12]Device" resource.
 *
 * \param[in]   combo       device combo box
 * \param[in]   portnum     tape port number
 */
static void on_device_changed(GtkComboBox *combo, gpointer portnum)
{
    GtkTreeModel *model;
    GtkTreeIter   iter;

    model = gtk_combo_box_get_model(combo);
    if (gtk_combo_box_get_active_iter(combo, &iter)) {
        gint   id;
        gchar *name = NULL;
        gint   port = GPOINTER_TO_INT(portnum);

        gtk_tree_model_get(model,
                           &iter,
                           COL_DEVICE_ID, &id,
                           COL_DEVICE_NAME, &name,
                           -1);
        resources_set_int_sprintf("TapePort%iDevice", id, port);
        set_options_widgets_sensitivity(id);

        g_free(name);
    }
}

/** \brief  Set tapeport device ID
 *
 * Sets the currently selected combobox item via device ID.
 *
 * To avoid updating the related resource via the combobox' event handler, use
 * the \a blocked argument.
 *
 * \param[in]   combo   device combo box
 * \param[in]   id      device ID
 * \param[in]   blocked block 'changed' signal handler
 */
static gboolean set_device_id(GtkComboBox *combo, gint id, gboolean blocked)
{
    GtkTreeModel *model;
    GtkTreeIter   iter;
    gulong        handler_id;
    gboolean      result = FALSE;

    /* do we need to block the 'changed' event handler? */
    if (blocked) {
        /* look up handler ID by callback */
        handler_id = g_signal_handler_find(combo,
                                           G_SIGNAL_MATCH_FUNC,
                                           0,       /* signal_id */
                                           0,       /* detail */
                                           NULL,    /* closure */
                                           on_device_changed,   /* func */
                                           NULL);
        if (handler_id > 0) {
            g_signal_handler_block(combo, handler_id);
        }
    }

    /* iterate the model until we find the device ID */
    model = gtk_combo_box_get_model(combo);
    if (gtk_tree_model_get_iter_first(model, &iter)) {
        do {
            gint current;

            gtk_tree_model_get(model, &iter, COL_DEVICE_ID, &current, -1);
            if (id == current) {
                gtk_combo_box_set_active_iter(combo, &iter);
                result = TRUE;
                break;
            }
        } while (gtk_tree_model_iter_next(model, &iter));
    }

    /* set options checkboxes "greyed-out" state */
    set_options_widgets_sensitivity(id);

    /* unblock signal, if blocked */
    if (blocked) {
        g_signal_handler_unblock(combo, handler_id);
    }

    return result;
}

/** \brief  Create model for the device combobox
 *
 * Create a model with (dev-id, dev-name, dev-type-id, dev-type-desc).
 *
 * \param[in]   port    tape port number (1 or 2 (PET))
 *
 * \return  model
 */
static GtkListStore *create_device_model(int port)
{
    GtkListStore    *model;
    tapeport_desc_t *devices;
    tapeport_desc_t *dev;

    model = gtk_list_store_new(4,
                               G_TYPE_INT,      /* ID */
                               G_TYPE_STRING,   /* name */
                               G_TYPE_INT,      /* type ID */
                               G_TYPE_STRING    /* type description */
                               );
    devices = tapeport_get_valid_devices(port - 1 /*index, not port */, TRUE);
    for (dev = devices; dev->name != NULL; dev++) {
        GtkTreeIter iter;

        gtk_list_store_append(model, &iter);
        gtk_list_store_set(model,
                           &iter,
                           COL_DEVICE_ID, dev->id,
                           COL_DEVICE_NAME, dev->name,
                           COL_DEVICE_TYPE_ID, dev->device_type,
                           COL_DEVICE_TYPE_DESC, tapeport_get_device_type_desc(dev->device_type),
                           -1);
    }
    lib_free(devices);

    return model;
}

/** \brief  Create combobox for the tapeport devices
 *
 * Create a combobox with valid tapeport devices for current machine.
 *
 * The model of the combobox contains device ID, name and type, of which name
 * is shown and ID is used to set the related resource.
 *
 * \param[in]   port    tape port number (1 or 2 (PET only))
 *
 * \return  GtkComboBox
 *
 * \todo    Try using the device type to create little headers in the combobox,
 *          grouping the devices by type. Might be overkill for some machines
 *          that only have a few tapeport devices, we'll see.
 *          I tried using a second column for the device type description, and
 *          althought it doesn't look bad in the popup list, when the popup
 *          isn't active it looks weird ;)
 *          So for now the device type isn't used.
 */
static GtkWidget *create_device_combobox(int port)
{
    GtkWidget       *combo;
    GtkListStore    *model;
    GtkCellRenderer *name_renderer;
#if 0
    GtkCellRenderer *type_renderer;
#endif

    /* TODO:    Check if the model can be shared for both ports, perhaps the
     *          second tape port has a different set of supported devices
     */
    model = create_device_model(port);

    /* create combobox with a single cell renderer for the device name column */
    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(model));
    name_renderer = gtk_cell_renderer_text_new();
#if 0
    type_renderer = gtk_cell_renderer_text_new();
#endif
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),
                               name_renderer,
                               TRUE);
    gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(combo),
                                   name_renderer,
                                   "text", COL_DEVICE_NAME,
                                   NULL);
#if 0
    gtk_cell_layout_pack_end(GTK_CELL_LAYOUT(combo),
                             type_renderer,
                             TRUE);
    gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(combo),
                                   type_renderer,
                                   "text", COL_DEVICE_TYPE_DESC,
                                   NULL);
#endif

    g_signal_connect(combo,
                     "changed",
                     G_CALLBACK(on_device_changed),
                     GINT_TO_POINTER(port));

    return combo;
}


/** \brief  Create combobox(es) to select device type for port 1 (and 2 for PET)
 *
 * \return  GtkGrid
 */
static GtkWidget *create_device_types_widget(void)
{
    GtkWidget *grid;
    GtkWidget *label;

    grid = vice_gtk3_grid_new_spaced_with_label(8, 0, "Tape port device types", 4);
    vice_gtk3_grid_set_title_margin(grid, 8);

    /* first tape port */
    label = gtk_label_new("Tape port #1:");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    gtk_widget_set_hexpand(label, FALSE);
    gtk_widget_set_margin_start(label, 8);
    port1_type = create_device_combobox(TAPEPORT_UNIT_1);
    gtk_widget_set_hexpand(port1_type, TRUE);
    gtk_grid_attach(GTK_GRID(grid), label, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), port1_type, 1, 1, 1, 1);

    /* PET has a second tape port */
    if (machine_has_second_tape_port()) {
        label = gtk_label_new("Tape port #2:");
        gtk_widget_set_halign(label, GTK_ALIGN_START);
        gtk_widget_set_hexpand(label, FALSE);
        gtk_widget_set_margin_start(label, 8);
        port2_type = create_device_combobox(TAPEPORT_UNIT_2);
        gtk_widget_set_hexpand(port2_type, TRUE);
        gtk_grid_attach(GTK_GRID(grid), label, 2, 1, 1, 1);
        gtk_grid_attach(GTK_GRID(grid), port2_type, 3, 1, 1, 1);
    }

    gtk_widget_show_all(grid);
    return grid;
}


/** \brief  Create widget to select/control tape port devices
 *
 * \param[in]   parent  parent widget (ignored)
 *
 * \return  GtkGrid
 */
GtkWidget *settings_tapeport_widget_create(GtkWidget *parent)
{
    GtkWidget *grid;
    GtkWidget *devices;
    GtkWidget *datasette;
    GtkWidget *cpcf83;
    int        device_id = 0;
    int        row = 0;

    grid = vice_gtk3_grid_new_spaced(8, 8);

    /* comboboxes with the tapeport devices */
    devices = create_device_types_widget();
    gtk_widget_set_margin_bottom(devices, 8);
    gtk_grid_attach(GTK_GRID(grid), devices, 0, row, 1, 1);
    row++;

    /* datasette device settings */
    datasette = create_datasette_widget();
    gtk_widget_set_margin_bottom(datasette, 8);
    gtk_grid_attach(GTK_GRID(grid), datasette, 0, row, 1, 1);
    row++;

    /* Cassette Port Clock F83 */
    cpcf83 = create_cpcf83_widget();
    gtk_grid_attach(GTK_GRID(grid), cpcf83, 0, row, 1, 1);
    row++;

    /* TapeCart settings */
    if (machine_has_tapecart()) {
        gtk_grid_attach(GTK_GRID(grid), create_tapecart_widget(), 0, row, 1, 1);
        row++;
    }

    /* these need to happen here, after the above widgets are created */
    /* set port1 type using the resource */
    resources_get_int("TapePort1Device", &device_id);
    set_device_id(GTK_COMBO_BOX(port1_type), device_id, TRUE);
    if (machine_has_second_tape_port()) {
        /* set port2 type using the resource */
        resources_get_int("TapePort2Device", &device_id);
        set_device_id(GTK_COMBO_BOX(port2_type), device_id, TRUE);
    }

    gtk_widget_show_all(grid);
    return grid;
}
