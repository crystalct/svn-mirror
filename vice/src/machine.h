/*
 * machine.h - Interface to machine-specific implementations.
 *
 * Written by
 *  Ettore Perazzoli <ettore@comm2000.it>
 *  Andreas Boose <viceteam@t-online.de>
 *
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

#ifndef VICE_MACHINE_H
#define VICE_MACHINE_H

#include "types.h"

/* The following stuff must be defined once per every emulated CBM machine.  */

/* Name of the machine.  */
extern const char machine_name[];

/* A little handier way to identify the machine: */
#define VICE_MACHINE_NONE       0

#define VICE_MACHINE_C64        (1U<<0)
#define VICE_MACHINE_C128       (1U<<1)
#define VICE_MACHINE_VIC20      (1U<<2)
#define VICE_MACHINE_PET        (1U<<3)
#define VICE_MACHINE_CBM5x0     (1U<<4)
#define VICE_MACHINE_CBM6x0     (1U<<5)
#define VICE_MACHINE_PLUS4      (1U<<6)
#define VICE_MACHINE_C64DTV     (1U<<7)
#define VICE_MACHINE_C64SC      (1U<<8)
#define VICE_MACHINE_VSID       (1U<<9)
#define VICE_MACHINE_SCPU64     (1U<<10)

#define VICE_MACHINE_ALL        (VICE_MACHINE_C64|VICE_MACHINE_C64SC|VICE_MACHINE_C64DTV|VICE_MACHINE_SCPU64|VICE_MACHINE_C128|VICE_MACHINE_VIC20|VICE_MACHINE_PLUS4|VICE_MACHINE_PET|VICE_MACHINE_CBM5x0|VICE_MACHINE_CBM6x0|VICE_MACHINE_VSID)

/* Sync factors (changed to positive 2016-11-07, BW)  */
#define MACHINE_SYNC_PAL     1
#define MACHINE_SYNC_NTSC    2
#define MACHINE_SYNC_NTSCOLD 3
#define MACHINE_SYNC_PALN    4

struct machine_timing_s {
    unsigned int cycles_per_line;
    long cycles_per_rfsh;
    long cycles_per_sec;
    unsigned int power_freq;   /* mains power frequency in hz */
    double rfsh_per_sec;
    unsigned int screen_lines;
};
typedef struct machine_timing_s machine_timing_t;

extern int machine_class;
extern int console_mode;
extern int video_disabled_mode;
extern int help_requested;

#define MACHINE_JAM_ACTION_DIALOG       0
#define MACHINE_JAM_ACTION_CONTINUE     1
#define MACHINE_JAM_ACTION_MONITOR      2
#define MACHINE_JAM_ACTION_RESET        3
#define MACHINE_JAM_ACTION_HARD_RESET   4
#define MACHINE_JAM_ACTION_QUIT         5
#define MACHINE_NUM_JAM_ACTIONS         6

/* Initialize the machine's resources.  */
int machine_common_resources_init(void);
int machine_resources_init(void);
void machine_common_resources_shutdown(void);
void machine_resources_shutdown(void);

/* Initialize the machine's command-line options.  */
int machine_common_cmdline_options_init(void);
int machine_cmdline_options_init(void);

/* Initialize the machine.  */
void machine_setup_context(void);
int machine_init(void);
int machine_specific_init(void);
void machine_early_init(void);

/* Initialize the main CPU of the machine.  */
void machine_maincpu_init(void);

/* Shutdown the main CPU of the machine. */
void machine_maincpu_shutdown(void);

/* Reset the machine.  */
#define MACHINE_RESET_MODE_SOFT 0
#define MACHINE_RESET_MODE_HARD 1

void machine_trigger_reset(const unsigned int reset_mode);
void machine_reset(void);
void machine_specific_reset(void);
void machine_reset_event_playback(CLOCK offset, void *data);

/* Power-up the machine.  */
void machine_specific_powerup(void);

/* Shutdown the emachine.  */
void machine_shutdown(void);
void machine_specific_shutdown(void);

/* Set the state of the RESTORE key (!=0 means pressed) */
void machine_set_restore_key(int v);

/* returns 1 if key is present */
int machine_has_restore_key(void);

/* Get the number of CPU cylces per second.  This is used in various parts.  */
long machine_get_cycles_per_second(void);

/* Get the number of CPU cylces per frame. */
long machine_get_cycles_per_frame(void);

/* Set the screen refresh rate, as this is variable in the CRTC.  */
void machine_set_cycles_per_frame(long cpf);

/* Get current line and cycle. */
void machine_get_line_cycle(unsigned int *line, unsigned int *cycle, int *half_cycle);

/* Write a snapshot.  */
int machine_write_snapshot(const char *name, int save_roms, int save_disks, int even_mode);

/* Read a snapshot.  */
int machine_read_snapshot(const char *name, int even_mode);

/* handle pending interrupts - needed by libsid.a.  */
void machine_handle_pending_alarms(CLOCK num_write_cycles);

/* Autodetect PSID file.  */
int machine_autodetect_psid(const char *name);
void machine_play_psid(int tune);

/* Check the base address for the second sid chip.  */
int machine_sid2_check_range(unsigned int sid_adr);

/* Check the base address for the third sid chip.  */
int machine_sid3_check_range(unsigned int sid_adr);

/* Check the base address for the fourth sid chip.  */
int machine_sid4_check_range(unsigned int sid_adr);

/* Check the base address for the fifth sid chip.  */
int machine_sid5_check_range(unsigned int sid_adr);

/* Check the base address for the sixth sid chip.  */
int machine_sid6_check_range(unsigned int sid_adr);

/* Check the base address for the seventh sid chip.  */
int machine_sid7_check_range(unsigned int sid_adr);

/* Check the base address for the eighth sid chip.  */
int machine_sid8_check_range(unsigned int sid_adr);

/* Change the timing parameters of the maching (for example PAL/NTSC).  */
void machine_change_timing(int timeval, int border_mode);

/* Get screenshot data.  */
struct screenshot_s;
struct video_canvas_s;
struct canvas_refresh_s;

int machine_screenshot(struct screenshot_s *screenshot, struct video_canvas_s *canvas);
int machine_canvas_async_refresh(struct canvas_refresh_s *ref, struct video_canvas_s *canvas);

#define JAM_NONE       0
#define JAM_RESET      1
#define JAM_HARD_RESET 2
#define JAM_MONITOR    3

unsigned int machine_jam(const char *format, ...) VICE_ATTR_PRINTF;
bool machine_is_jammed(void);
char *machine_jam_reason(void);

/* Update memory pointers if memory mapping has changed. */
void machine_update_memory_ptrs(void);

extern int machine_keymap_index;
extern char *machine_keymap_file_list[];

int machine_num_keyboard_mappings(void);

struct image_contents_s;
struct image_contents_s *machine_diskcontents_bus_read(unsigned int unit);

/* Romset handling.  */
void machine_romset_init(void);
int machine_romset_file_load(const char *filename);
int machine_romset_file_save(const char *filename);
char *machine_romset_file_list(void);
int machine_romset_archive_item_create(const char *romset_name);

uint8_t machine_tape_type_default(void);
uint8_t machine_tape_behaviour(void);

/* Check if address is in RAM (for autostart) */
int machine_addr_in_ram(unsigned int addr);

/* Get "real" name for machine. May differ from machine_name.  */
const char *machine_get_name(void);

/* Get keymap res name with range checking */
char *machine_get_keymap_res_name(int val);

/* mapping info for GUIs */
typedef struct {
    char *name;
    int type;
    unsigned int flags;
} kbdtype_info_t;

int machine_get_num_keyboard_types(void);
kbdtype_info_t *machine_get_keyboard_info_list(void);

int machine_get_keyboard_type(void);
char *machine_get_keyboard_type_name(int type);

int machine_register_userport(void);

#endif
