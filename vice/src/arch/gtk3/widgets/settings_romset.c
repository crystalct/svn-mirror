/** \file   settings_romset.c
 * \brief   GTK3 ROM set widget
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 */

/*
 * FIXME:   Most resources seem to be correct, but need further checking
 *
 * $VICERES KernalName          x64 x64sc xvic xplus4 xcbm5x0 xcbm2 xpet
 * $VICERES BasicName           x64 x64sc xscpu64 xvic xplus4 xcbm5x0 xcbm2 xpet
 * $VICERES ChargenName         x64 x64sc xscpu64 xvic xcbm5x0 xcbm2 xpet
 * $VICERES SCPU64Name          xscpu64
 * $VICERES KernalIntName       x128
 * $VICERES KernalDEName        x128
 * $VICERES KernalFIName        x128
 * $VICERES KernalFRName        x128
 * $VICERES KernalITName        x128
 * $VICERES KernalNOName        x128
 * $VICERES KernalSEName        x128
 * $VICERES KernalCHName        x128
 * $VICERES BasicLoName         x128
 * $VICERES BasicHiName         x128
 * $VICERES Kernal64Name        x128
 * $VICERES Basic64Name         x128
 * $VICERES ChargenIntName      x128
 * $VICERES ChargenDEName       x128
 * $VICERES ChargenFRName       x128
 * $VICERES ChargenSEName       x128
 * $VICERES ChargenCHName       x128
 * $VICERES ChargenNOName       x128
 * $VICERES FunctionLowName     xplus4
 * $VICERES FunctionHighName    xplus4
 * $VICERES c1loName            xplus4
 * $VICERES c1hiName            xplus4
 * $VICERES c2loName            xplus4
 * $VICERES c2hiName            xplus4
 * $VICERES Cart1Name           xcbm5x0 xcbm2
 * $VICERES Cart2Name           xcbm5x0 xcbm2
 * $VICERES Cart4Name           xcbm5x0 xcbm2
 * $VICERES Cart6Name           xcbm5x0 xcbm2
 * $VICERES EditorName          xpet
 * $VICERES Basic1              xpet
 *
 * Drive ROMS:
 *
 * $VICERES DosName1540         x64 x64sc xscpu64 xvic x128 xplus4
 * $VICERES DosName1541         x64 x64sc xscpu64 xvic x128 xplus4
 * $VICERES DosName1541ii       x64 x64sc xscpu64 xvic x128 xplus4
 * $VICERES DosName1551         xplus4
 * $VICERES DosName1570         x64 x64sc xscpu64 xvic x128 xplus4
 * $VICERES DosName1571         x64 x64sc xscpu64 xvic x128 xplus4
 * $VICERES DosName1571cr       x128
 * $VICERES DosName1581         x64 x64sc xscpu64 xvic x128 xplus4
 * $VICERES DosName2000         x64 x64sc xscpu64 xvic x128 xplus4
 * $VICERES DosName4000         x64 x64sc xscpu64 xvic x128 xplus4
 * $VICERES DosNameCMDHD        x64 x64sc xscpu64 xvic x128 xplus4
 * $VICERES DosName2031         x64 x64sc xscpu64 xvic x128 xcbm5x0 xcbm2 xpet
 * $VICERES DosName2040         x64 x64sc xscpu64 xvic x128 xcbm5x0 xcbm2 xpet
 * $VICERES DosName3040         x64 x64sc xscpu64 xvic x128 xcbm5x0 xcbm2 xpet
 * $VICERES DosName4040         x64 x64sc xscpu64 xvic x128 xcbm5x0 xcbm2 xpet
 * $VICERES DosName1001         x64 x64sc xscpu64 xvic x128 xcbm5x0 xcbm2 xpet
 * $VICERES DosName9000         x64 x64sc xscpu64 xvic x128 xcbm5x0 xcbm2 xpet
 * $VICERES DriveProfDOS1571Name    x64 x64sc xscpu64 x128
 * $VICERES DriveSuperCardName      x64 x64sc xscpu64 x128
 * $VICERES DriveStarDosName        x64 x64sc xscpu64 x128
 *
 * User-defined ROMs:
 *
 * $VICERES RomModule9Name      xpet
 * $VICERES RomModuleAName      xpet
 * $VICERES RomModuleBName      xpet
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
 */

#include "vice.h"

#include <gtk/gtk.h>

#include "vice_gtk3.h"
#include "archdep.h"
#include "debug_gtk3.h"
#include "resourcehelpers.h"
#include "machine.h"
#include "diskimage.h"
#include "romset.h"
#include "lib.h"
#include "petrom.h"
#include "ui.h"
#include "romsetmanagerwidget.h"

#include "settings_romset.h"


/** \brief  Default ROM set name for C64/C64DTV/SCUP64/C128/VIC20/PLUS4 */
#define ROMSET_DEFAULT_C64      "default.vrs"

/** \brief  Default ROM set name for CBM-II 5x0 models */
#define ROMSET_DEFAULT_CBM5x0   "rom500.vrs"


/** \brief  Predefined ROM sets for CBM-II 6x0
 */
static const vice_gtk3_combo_entry_str_t predefs_cbm6x0[] = {
    { "Basic 128K, low chars",      "rom128l.vrs" },
    { "Basic 256K, low chars",      "rom256l.vrs" },
    { "Basic 128K, high chars",     "rom128h.vrs" },
    { "Basic 256K, high chars",     "rom256h.vrs" },
    { NULL,                         NULL }
};


/** \brief  Predefined ROM sets for PET
 */
static const vice_gtk3_combo_entry_str_t predefs_pet[] = {
    { "Basic 1",                        "rom1g.vrs" },
    { "Basic 2, graphics",              "rom2g.vrs" },
    { "Basic 2, business",              "rom2b.vrs" },
    { "Basic 4, 40 cols, graphics",     "rom4g40.vrs" },
    { "Basic 4, 40 cols, business",     "rom4b40.vrs" },
    { "Basic 4, 80 cols, business",     "rom4b80.vrs" },
    { NULL,                             NULL }
};



/** \brief  Machine ROM types
 *
 * This probably needs a lot of updating once I get to the PET. CBM-II etc.
 */
typedef enum rom_type_e {
    ROM_BASIC,      /**< Basic ROM */
    ROM_KERNAL,     /**< Kernal ROM */
    ROM_CHARGEN     /**< Character set ROM */
} rom_type_t;


/** \brief  ROM info object
 */
typedef struct romset_entry_s {
    const char  *resource;  /**< resource name */
    const char  *label;     /**< label */
    void        (*callback)(GtkWidget *, gpointer); /**< optional extra callback
                                                         (currently unused) */
} romset_entry_t;


/** \brief  List of C64/VIC20 machine ROMs
 */
static const romset_entry_t c64_vic20_machine_roms[] = {
    { "KernalName",     "Kernal",   NULL },
    { "BasicName",      "Basic",    NULL },
    { "ChargenName",    "Chargen",  NULL },
    { NULL,             NULL,       NULL }
};


/** \brief  List of SCPU64 machine ROMs
 */
static const romset_entry_t scpu64_machine_roms[] = {
    { "SCPU64Name",     "Kernal",   NULL },
    { "ChargenName",    "Chargen",  NULL },
    { NULL,             NULL,       NULL }
};


/** \brief  List of C128 machine ROMs
 *
 * Kernals and Basic only to avoid the dialog getting too large
 *
 */
static const romset_entry_t c128_machine_roms[] = {
    { "KernalIntName",  "International Kernal",     NULL },
    { "KernalDEName",   "German Kernal",            NULL },
    { "KernalFIName",   "Finnish Kernal",           NULL },
    { "KernalFRName",   "French Kernal",            NULL },
    { "KernalITName",   "Italian Kernal",           NULL },
    { "KernalNOName",   "Norwegian Kernal",         NULL },
    { "KernalSEName",   "Swedish Kernal",           NULL },
    { "KernalCHName",   "Swiss Kernal",             NULL },
    { "BasicLoName",    "Basic Lo ROM",             NULL },
    { "BasicHiName",    "Basic Hi ROM",             NULL },
    { "Kernal64Name",   "C64 Kernal ROM",           NULL },
    { "Basic64Name",    "C64 Basic ROM",            NULL },
    { NULL,             NULL,                       NULL }
};


/** \brief  List of C128 chargen ROMs
 *
 * CHARGENs only to avoid the dialog getting too large.
 */
static const romset_entry_t c128_chargen_roms[] = {
    { "ChargenIntName", "International Chargen",    NULL },
    { "ChargenDEName",  "German Chargen",           NULL },
    { "ChargenFRName",  "French Chargen",           NULL },
    { "ChargenSEName",  "Swedish Chargen",          NULL },
    { "ChargenCHName",  "Swiss Chargen",            NULL },
    { "ChargenNOName",  "Norwegian Chargen",        NULL },
    { NULL,             NULL,                       NULL }
};


/** \brief  Plus4 machine ROMs
 */
static const romset_entry_t plus4_machine_roms[] = {
    { "KernalName",         "Kernal",           NULL },
    { "BasicName",          "Basic",            NULL },
    { "FunctionLowName",    "3 Plus 1 LO ROM",  NULL },
    { "FunctionHighName",   "3 Plus 1 HI ROM",  NULL },
/*  { "c1loName",           "c1 LO ROM",        NULL },
    { "c1hiName",           "c1 HI ROM",        NULL }, */
    { "c2loName",           "c2 LO ROM",        NULL },
    { "c2hiName",           "c2 HI ROM",        NULL },
    { NULL,                 NULL,               NULL }
};


/** \brief  CBM-II machine ROMs
 */
static const romset_entry_t cbm2_machine_roms[] = {
    { "KernalName",         "Kernal",           NULL },
    { "BasicName",          "Basic",            NULL },
    { "ChargenName",        "Chargen",          NULL },
    { "Cart1Name",          "$1000-$1FFF ROM",  NULL },
    { "Cart2Name",          "$2000-$3FFF ROM",  NULL },
    { "Cart4Name",          "$4000-$5FFF ROM",  NULL },
    { "Cart6Name",          "$6000-$7FFF ROM",  NULL },
    { NULL,                 NULL,               NULL }
};


/** \brief  Machine ROMs for 'normal' PETs
 */
static const romset_entry_t pet_machine_roms[] = {
    { "KernalName",         "Kernal",           NULL },
    { "BasicName",          "Basic",            NULL },
    { "EditorName",         "Editor",           NULL },
    /* this one must come last for the 'load original/German charset' buttons
     * to make sense: */
    { "ChargenName",        "Chargen",          NULL },
    { NULL,                 NULL,               NULL }
};


/** \brief  List of drive ROMs for unsupported machines
 */
static const romset_entry_t unsupported_drive_roms[] = {
    { NULL, NULL, NULL }
};


/** \brief  List of drive ROMs supported by C64/VIC20
 */
static const romset_entry_t c64_vic20_drive_roms[] = {
    { "DosName1540",    "1540",     NULL },
    { "DosName1541",    "1541",     NULL },
    { "DosName1541ii",  "1541-II",  NULL },
    { "DosName1570",    "1570",     NULL },
    { "DosName1571",    "1571",     NULL },
    { "DosName1581",    "1581",     NULL },
    { "DosName2000",    "2000",     NULL },
    { "DosName4000",    "4000",     NULL },
    { "DosNameCMDHD",   "CMD HD",   NULL },
    { "DosName2031",    "2031",     NULL },
    { "DosName2040",    "2040",     NULL },
    { "DosName3040",    "3040",     NULL },
    { "DosName4040",    "4040",     NULL },
    { "DosName1001",    "1001",     NULL },
    { "DosName9000",    "D9090/60", NULL },
    { NULL,         NULL,           NULL }
};


/** \brief  List of drive ROMs supported by C128
 */
static const romset_entry_t c128_drive_roms[] = {
    { "DosName1540",    "1540",     NULL },
    { "DosName1541",    "1541",     NULL },
    { "DosName1541ii",  "1541-II",  NULL },
    { "DosName1570",    "1570",     NULL },
    { "DosName1571",    "1571",     NULL },
    { "DosName1571cr",  "1571CR",   NULL },
    { "DosName1581",    "1581",     NULL },
    { "DosName2000",    "2000",     NULL },
    { "DosName4000",    "4000",     NULL },
    { "DosNameCMDHD",   "CMD HD",   NULL },
    { "DosName2031",    "2031",     NULL },
    { "DosName2040",    "2040",     NULL },
    { "DosName3040",    "3040",     NULL },
    { "DosName4040",    "4040",     NULL },
    { "DosName1001",    "1001",     NULL },
    { "DosName9000",    "D9090/60", NULL },
    { NULL,         NULL,           NULL }
};


/** \brief  Drive expansion ROMs for C64/C128
 *
 * Expansion roms for 1540, 1541, 1541-II and 1571.
 */
static const romset_entry_t c64_c128_drive_exp_roms[] = {
    { "DriveProfDOS1571Name",   "ProfDOS 1571", NULL },
    { "DriveSuperCardName",     "Supercard",    NULL },
    { "DriveStarDosName",       "StarDOS",      NULL },
    { NULL,                     NULL,           NULL }
};


/** \brief  List of drive ROMs supported by PET/CBM-II (5x0 + 6x0/7x0)
 */
static const romset_entry_t pet_cbm2_drive_roms[] = {
    { "DosName2031",    "2031",     NULL },
    { "DosName2040",    "2040",     NULL },
    { "DosName3040",    "3040",     NULL },
    { "DosName4040",    "4040",     NULL },
    { "DosName1001",    "1001",     NULL },
    { "DosName9000",    "D9090/60", NULL },
    { NULL,         NULL,           NULL }
};


/** \brief  List of drive ROMs supported by Plus/4
 */
static const romset_entry_t plus4_drive_roms[] = {
    { "DosName1540",    "1540",     NULL },
    { "DosName1541",    "1541",     NULL },
    { "DosName1541ii",  "1541-II",  NULL },
    { "DosName1551",    "1551",     NULL },
    { "DosName1570",    "1570",     NULL },
    { "DosName1571",    "1571",     NULL },
    { "DosName1581",    "1581",     NULL },
    { "DosName2000",    "2000",     NULL },
    { "DosName4000",    "4000",     NULL },
    { "DosNameCMDHD",   "CMD HD",   NULL },
    { NULL,         NULL,           NULL }
};



/** \brief  Layout grid */
static GtkWidget *layout = NULL;

/** \brief  Stack widget for the various 'pages' */
static GtkWidget *stack = NULL;

/** \brief  Stack switcher to select the 'pages' */
static GtkWidget *switcher = NULL;

/** \brief  Stack child containing the machine ROMs widget */
static GtkWidget *child_machine_roms = NULL;

/** \brief  Stack child containing CHARGEN widgets (x128)
 *
 * Used to avoid making the dialog too large due to the number of C128 CHARGENs
 */
static GtkWidget *child_chargen_roms = NULL;

/** \brief  Stack child for the drive ROMs widget */
static GtkWidget *child_drive_roms = NULL;

/** \brief  Stack child for the drive expansion ROMS
 *
 * For x64, x64sc, xscpu64 and x128.
 */
static GtkWidget *child_drive_exp_roms = NULL;

/** \brief  Stack child for the ROM archives widgets */
static GtkWidget *child_rom_archives = NULL;


/** \brief  Handler for the 'clicked' event of the PET CHARGEN button
 *
 * \param[in]   widget  PET chargen button
 * \param[in]   data    CHARGEN name
 */
static void on_pet_select_chargen(GtkWidget *widget, gpointer data)
{
    const char *chargen = (const char*)data;
    GtkWidget *browser;

    browser = gtk_grid_get_child_at(GTK_GRID(child_machine_roms), 1, 3);
    if (GTK_IS_GRID(browser)) {
        vice_gtk3_resource_browser_set(browser, chargen);
    }
}


/** \brief  Create stack switcher and attach to \a grid
 *
 * \param[in,out]   grid    ROM sets widget
 */
static void create_stack_switcher(GtkWidget *grid)
{
    stack = gtk_stack_new();
    switcher = gtk_stack_switcher_new();

    gtk_stack_set_transition_type(GTK_STACK(stack),
            GTK_STACK_TRANSITION_TYPE_SLIDE_LEFT_RIGHT);
    gtk_stack_set_transition_duration(GTK_STACK(stack), 500);
    gtk_stack_set_homogeneous(GTK_STACK(stack), TRUE);
    gtk_stack_switcher_set_stack(GTK_STACK_SWITCHER(switcher),
            GTK_STACK(stack));
    gtk_widget_set_halign(switcher, GTK_ALIGN_CENTER);
    gtk_widget_set_hexpand(switcher, TRUE);

    /* switcher goes first, otherwise it ends up a the bottom of the widget,
     * which we don't want, although maybe in a few years having the 'tabs'
     * at the bottom suddenly becomes popular, in which case we simply swap
     * the row number of the stack and the switcher :) */
    gtk_grid_attach(GTK_GRID(grid), switcher, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), stack, 0, 1, 1, 1);

    gtk_widget_show(switcher);
    gtk_widget_show(stack);
}


/** \brief  Add a child widget to the stack
 *
 * \param[in]   child   child widget
 * \param[in]   title   displayed title
 * \param[in]   name    ID to use when referencing the stack's children
 *
 */
static void add_stack_child(GtkWidget *child,
                            const gchar *title,
                            const gchar *name)
{
    gtk_stack_add_titled(GTK_STACK(stack), child, name, title);
}


/** \brief  Create a list of ROM selection widgets from \a roms
 *
 * \param[in]   roms    ROMs list
 * \param[in]   path    directory to append in the browser when only a filename
 *                      is present in the resource (optional)
 * \return  GtkGrid
 */
static GtkWidget* create_roms_widget(const romset_entry_t *roms, const char *path)
{
    GtkWidget *grid;
    int row;

    /* needs 0 row spacing to avoid having the dialog get too large */
    grid = vice_gtk3_grid_new_spaced(VICE_GTK3_DEFAULT, 0);

    for (row = 0; roms[row].resource != NULL; row++) {
        GtkWidget *label;
        GtkWidget *browser;

        label = gtk_label_new(roms[row].label);
        gtk_widget_set_halign(label, GTK_ALIGN_START);
        browser = vice_gtk3_resource_browser_new(roms[row].resource,
                NULL, NULL, "Select ROM file",
                NULL /* no label, so the labels get aligned properly */,
                NULL);
        /* set append-dir if given */
        if (path != NULL) {
            vice_gtk3_resource_browser_set_directory(browser, path);
        }
        gtk_grid_attach(GTK_GRID(grid), label, 0, row, 1,  1);
        gtk_grid_attach(GTK_GRID(grid), browser, 1, row, 1, 1);

    }

    gtk_widget_show_all(grid);
    return grid;

}


/** \brief  Create layout for C64 and VIC20
 *
 * \return  GtkGrid
 */
static GtkWidget *create_c64_vic20_roms_widget(void)
{
    GtkWidget *grid;
    char *path;

    path = archdep_get_vice_machinedir();
    grid = create_roms_widget(c64_vic20_machine_roms, path);
    lib_free(path);
    return grid;
}


/** \brief  Create layout for SCPU64
 *
 * \return  GtkGrid
 */
static GtkWidget *create_scpu64_roms_widget(void)
{
    GtkWidget *grid;
    char *path;

    path = archdep_get_vice_machinedir();
    grid = create_roms_widget(scpu64_machine_roms, path);
    lib_free(path);
    return grid;
}


/** \brief  Create layout for C128
 *
 * \return  GtkGrid
 */
static GtkWidget *create_c128_roms_widget(void)
{
    GtkWidget *grid;
    char *path;

    path = archdep_get_vice_machinedir();
    grid = create_roms_widget(c128_machine_roms, path);
    lib_free(path);
    return grid;
}


/** \brief  Create CHARGEN widget for C128
 *
 * \return  GtkGrid
 */
static GtkWidget *create_c128_chargen_widget(void)
{
    GtkWidget *grid;
    char *path;

    path = archdep_get_vice_machinedir();
    grid = create_roms_widget(c128_chargen_roms, path);
    lib_free(path);
    return grid;
}


/** \brief  Create layout for Plus4
 *
 * \return  GtkGrid
 */
static GtkWidget *create_plus4_roms_widget(void)
{
    GtkWidget *grid;
    char *path;

    path = archdep_get_vice_machinedir();
    grid = create_roms_widget(plus4_machine_roms, path);
    lib_free(path);
    return grid;
}


/** \brief  Create layout for CBM-II
 *
 * \return  GtkGrid
 */
static GtkWidget *create_cbm2_roms_widget(void)
{
    GtkWidget *grid;
    char *path;

    path = archdep_get_vice_machinedir();
    grid = create_roms_widget(cbm2_machine_roms, path);
    lib_free(path);
    return grid;
}


/** \brief  Unload a PET ROM
 *
 * \param[in]       widget  'unload' button
 * \param[in,out]   data    resource browser widget
 */
static void unload_pet_rom(GtkWidget *widget, gpointer data)
{
    GtkWidget *browser = data;
    vice_gtk3_resource_browser_set(browser, NULL);
}


/** \brief  Create machine ROMs widget for PET/SuperPET
 *
 * \return  GtkGrid
 *
 * \todo    Refactor code, too convoluted
 */
static GtkWidget *create_pet_roms_widget(void)
{
    GtkWidget *grid;
    GtkWidget *button;
    GtkWidget *wrapper;
    GtkWidget *basic1;
    GtkWidget *unload;
    GtkWidget *label;
    GtkWidget *browser;

    char *path;
    int i;

    const char *modules[3] = {
        "RomModule9Name",
        "RomModuleAName",
        "RomModuleBName"
    };

    path = archdep_get_vice_machinedir();
    grid = create_roms_widget(pet_machine_roms, path);
    lib_free(path);

    /* FIXME: what are those buttons about? why do they exist, and why only for
     * the german charset? this should be something handled by the model switching */
    /* add original/German charset buttons */
    wrapper = gtk_grid_new();
    gtk_widget_set_hexpand(wrapper, TRUE);
    button = gtk_button_new_with_label("Load original charset");
    gtk_widget_set_hexpand(button, TRUE);
    g_signal_connect(button, "clicked", G_CALLBACK(on_pet_select_chargen),
            (gpointer)(PET_CHARGEN2_NAME));
    gtk_grid_attach(GTK_GRID(wrapper), button, 0, 0, 1, 1);
    button = gtk_button_new_with_label("Load German charset");
    gtk_widget_set_hexpand(button, TRUE);
    g_signal_connect(button, "clicked", G_CALLBACK(on_pet_select_chargen),
            (gpointer)(PET_CHARGEN_DE_NAME));
    gtk_grid_attach(GTK_GRID(wrapper), button, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), wrapper, 1, 4, 1, 1);

    basic1 = vice_gtk3_resource_check_button_new("Basic1",
            "Patch Kernal v1 to make the IEEE488 interface work");
    gtk_widget_set_margin_top(basic1, 8);
    gtk_grid_attach(GTK_GRID(grid), basic1, 0, 5, 2, 1);


    for (i = 0; i < 3; i++) {
        const char *mod = modules[i];
        gchar text[256];

        g_snprintf(text, sizeof(text), "$%c000-$%cFFF ROM:", mod[9], mod[9]);
        label = gtk_label_new(text);
        browser = vice_gtk3_resource_browser_new(mod,
                NULL, NULL, "Attach new ROM", NULL, NULL);
        unload = gtk_button_new_with_label("Unload");
        g_signal_connect(unload, "clicked", G_CALLBACK(unload_pet_rom),
                (gpointer)browser);

        gtk_grid_attach(GTK_GRID(grid), label, 0, 6 + i, 1, 1);
        wrapper = gtk_grid_new();
        gtk_grid_attach(GTK_GRID(wrapper), browser, 0, 0, 1, 1);
        gtk_grid_attach(GTK_GRID(wrapper), unload, 1, 0, 1, 1);
        gtk_grid_attach(GTK_GRID(grid), wrapper, 1, 6 + i, 1, 1);
    }

    return grid;
}


/** \brief  Create machine ROMS widget
 *
 * \return  GtkGrid
 */
static GtkWidget *create_machine_roms_widget(void)
{
    GtkWidget *grid;

    switch (machine_class) {
        case VICE_MACHINE_C64:      /* fall through */
        case VICE_MACHINE_C64SC:    /* fall through */
        case VICE_MACHINE_C64DTV:   /* fall through */
        case VICE_MACHINE_VIC20:
            grid = create_c64_vic20_roms_widget();
            break;
        case VICE_MACHINE_SCPU64:
            grid = create_scpu64_roms_widget();
            break;
        case VICE_MACHINE_C128:
            grid = create_c128_roms_widget();
            break;
        case VICE_MACHINE_PLUS4:
            grid = create_plus4_roms_widget();
            break;
        case VICE_MACHINE_CBM5x0:
        case VICE_MACHINE_CBM6x0:
            grid = create_cbm2_roms_widget();
            break;
        case VICE_MACHINE_PET:
            grid = create_pet_roms_widget();
            break;
        default:
            grid = vice_gtk3_grid_new_spaced(VICE_GTK3_DEFAULT, VICE_GTK3_DEFAULT);
            gtk_grid_attach(GTK_GRID(grid),
                    gtk_label_new("Not supported yet, sorry!"),
                    0, 0, 1, 1);
            break;
    }

    gtk_widget_show_all(grid);
    return grid;
}


/** \brief  Create a stack widget with widgets for each supported drive ROM
 *
 * \return  GtkGrid
 */
static GtkWidget *create_drive_roms_widget(void)
{
    GtkWidget *widget;
    const romset_entry_t *entries;
    char *path;

    switch (machine_class) {
        case VICE_MACHINE_C64:      /* fall through */
        case VICE_MACHINE_C64SC:    /* fall through */
        case VICE_MACHINE_C64DTV:   /* fall through */
        case VICE_MACHINE_SCPU64:   /* fall through */
        case VICE_MACHINE_VIC20:
            entries = c64_vic20_drive_roms;
            break;
        case VICE_MACHINE_C128:
            entries = c128_drive_roms;
            break;
        case VICE_MACHINE_PET:      /* fall through */
        case VICE_MACHINE_CBM5x0:  /* fall through */
        case VICE_MACHINE_CBM6x0:
            entries = pet_cbm2_drive_roms;
            break;
        case VICE_MACHINE_PLUS4:
            entries = plus4_drive_roms;
            break;
        default:
            entries = unsupported_drive_roms;
            break;
    }

    /* create path to $VICE_DATADIR/DRIVES */
    path = archdep_get_vice_drivesdir();
    widget = create_roms_widget(entries, path);
    lib_free(path);
    return widget;
}


/** \brief  Create a stack widget with widgets for drive expansion ROMS
 *
 * Only valid on C64/C128.
 *
 * \return  GtkGrid
 */
static GtkWidget *create_drive_exp_roms_widget(void)
{
    GtkWidget *widget;
    char *path;

    path = archdep_get_vice_machinedir();
    widget = create_roms_widget(c64_c128_drive_exp_roms, path);
    lib_free(path);
    return widget;
}


/** \brief  Create widget to manipulate ROM archives
 *
 * \param[in]   predefs list of predefines ROMsets
 *
 * \return  GtkGrid
 */
static GtkWidget *create_rom_archives_widget(
        const vice_gtk3_combo_entry_str_t *predefs)
{
    GtkWidget *grid;
    GtkWidget *manager;

    grid = vice_gtk3_grid_new_spaced(VICE_GTK3_DEFAULT, VICE_GTK3_DEFAULT);
    manager = romset_manager_widget_create(predefs);
    gtk_grid_attach(GTK_GRID(grid), manager, 0, 0, 1, 1);
    gtk_widget_show_all(grid);
    return grid;
}


/** \brief  Create the main ROM settings widget
 *
 * \param[in]   parent  parent widget (unused)
 *
 * \return  GtkGrid
 */
GtkWidget *settings_romset_widget_create(GtkWidget *parent)
{
    const vice_gtk3_combo_entry_str_t *predefs;

    layout = vice_gtk3_grid_new_spaced(VICE_GTK3_DEFAULT, VICE_GTK3_DEFAULT);

    create_stack_switcher(layout);
    child_machine_roms = create_machine_roms_widget();
    if (machine_class == VICE_MACHINE_C128) {
        child_chargen_roms = create_c128_chargen_widget();
    }
    child_drive_roms = create_drive_roms_widget();

    if (machine_class == VICE_MACHINE_C64
            || machine_class == VICE_MACHINE_C64SC
            || machine_class == VICE_MACHINE_SCPU64
            || machine_class == VICE_MACHINE_C128) {
        child_drive_exp_roms = create_drive_exp_roms_widget();
    }

    /* determine predefined ROM sets, if any */
    switch (machine_class) {
        case VICE_MACHINE_CBM6x0:
            predefs = predefs_cbm6x0;
            break;
        case VICE_MACHINE_PET:
            predefs = predefs_pet;
            break;

        default:
            predefs = NULL;
    }
    child_rom_archives = create_rom_archives_widget(predefs);

    if (machine_class == VICE_MACHINE_C128) {
        add_stack_child(child_machine_roms, "Kernal/Basic", "machine");
        add_stack_child(child_chargen_roms, "Chargen ROMS", "chargen");
    } else {
        add_stack_child(child_machine_roms, "Machine ROMs", "machine");
    }
    add_stack_child(child_drive_roms, "Drive ROMs", "drive");

    if (machine_class == VICE_MACHINE_C64
            || machine_class == VICE_MACHINE_C64SC
            || machine_class == VICE_MACHINE_SCPU64
            || machine_class == VICE_MACHINE_C128) {
        add_stack_child(child_drive_exp_roms, "Drive exp. ROMs", "drive-exp");
    }

    add_stack_child(child_rom_archives, "ROM archives", "archive");
    gtk_widget_show_all(layout);
    return layout;
}
