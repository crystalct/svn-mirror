/*
 * c64cartsystem.h -- C64 cartridge emulation, internal stuff
 *
 * Written by
 *  groepaz <groepaz@gmx.net>
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

#ifndef VICE_C64CARTSYSTEM_H
#define VICE_C64CARTSYSTEM_H

#define CART_READ_THROUGH_NO_ULTIMAX    -2
#define CART_READ_C64MEM                -1
#define CART_READ_THROUGH               0
#define CART_READ_VALID                 1

/*
    these are the functions which are ONLY shared internally by the cartridge
    system, meaning c64cart.c, c64cartmem.c, c64carthooks.c, c64export.c and the
    individual cartridge implementations themselves.

    - all functions should start with a cart_ prefix
    - all functions which are related to a certain slot should get a proper
      postfix (_slot0, _slot1, _slotmain, _slotio)
*/

#include "types.h"

/* from c64cart.c */
int cart_attach_cmdline(const char *param, void *extra_param);

void cart_trigger_nmi(void);

void cart_unset_alarms(void);
void cart_power_off(void);

void cart_attach_from_snapshot(int type);

void cart_detach_slotmain(void);
int cart_getid_slotmain(void); /* returns ID of cart in "Main Slot" */
int cart_getid_slot0(void);
int cart_getid_slot1(void);

/* from c64carthooks.c */
void cart_nmi_alarm(CLOCK offset, void *data);
int cart_freeze_allowed(void);
void cart_undump_alarms(void);

extern CLOCK cart_nmi_alarm_time;
extern CLOCK cart_freeze_alarm_time;

void cart_init(void);
int cart_resources_init(void);
void cart_resources_shutdown(void);
int cart_cmdline_options_init(void);

int cart_can_get_file_name(int type);
const char *cart_get_file_name(int type);
int cart_is_slotmain(int type); /* returns 1 if cart of given type is in "Main Slot" */
int cart_type_enabled(int type);

void cart_attach(int type, uint8_t *rawcart);
int cart_bin_attach(int type, const char *filename, uint8_t *rawcart);
void cart_detach(int type);
void cart_detach_all(void);

void cart_detach_conflicting(int type);

/* from c64cartmem.c */
void cart_reset_memptr(void);

/* mode_phiN bit 0,1 control exrom/game */

/* FIXME: EXROM is inverted in these constants, while GAME is not */
#define CMODE_8KGAME    0
#define CMODE_16KGAME   1
#define CMODE_RAM       2
#define CMODE_ULTIMAX   3

const char *cart_config_string(uint8_t mode); /* convert above mode into human readable string */

/* mode_phiN other bits select bank (main slot only!) */
#define CMODE_BANK_SHIFT 2
#define CMODE_BANK_MASK 0x3f                    /* 64 Banks, meaning 512K max */

/* bits for wflag */
#define CMODE_READ  0
#define CMODE_WRITE 1                           /* config changes during a write access */
#define CMODE_RELEASE_FREEZE 2                  /* cartridge releases NMI condition */
#define CMODE_PHI2_RAM 4                        /* vic always sees RAM if set */
#define CMODE_EXPORT_RAM 8                      /* (main slot only!) RAM connected to expansion port */
#define CMODE_TRIGGER_FREEZE_NMI_ONLY 16        /* Trigger NMI after config changed */
/* shift value for the above */
#define CMODE_RW_SHIFT  0
#define CMODE_RELEASE_FREEZE_SHIFT 1
#define CMODE_PHI2_RAM_SHIFT 2
#define CMODE_EXPORT_RAM_SHIFT 3
#define CMODE_TRIGGER_FREEZE_NMI_ONLY_SHIFT 4

#ifdef CARTRIDGE_INCLUDE_SLOT0_API

void cart_config_changed_slot0(uint8_t mode_phi1, uint8_t mode_phi2, unsigned int wflag);
void cart_set_port_exrom_slot0(int n);
void cart_set_port_game_slot0(int n);
void cart_port_config_changed_slot0(void);

#endif /* CARTRIDGE_INCLUDE_SLOT0_API */

#ifdef CARTRIDGE_INCLUDE_SLOT1_API

void cart_config_changed_slot1(uint8_t mode_phi1, uint8_t mode_phi2, unsigned int wflag);
void cart_set_port_exrom_slot1(int n);
void cart_set_port_game_slot1(int n);
void cart_port_config_changed_slot1(void);

#endif /* CARTRIDGE_INCLUDE_SLOT1_API */

#ifdef CARTRIDGE_INCLUDE_SLOTMAIN_API

/* these are for the "Main Slot" only */
void cart_romhbank_set_slotmain(unsigned int bank);
void cart_romlbank_set_slotmain(unsigned int bank);

/* FIXME: these are shared between all "main slot" carts,
          individual cart implementations should get reworked to use local buffers */
extern uint8_t *roml_banks, *romh_banks, *export_ram0;
int rombanks_resources_init(void);
void rombanks_resources_shutdown(void);

extern int roml_bank, romh_bank, export_ram; /* "Main Slot" ROML/ROMH/RAM banking.  */

void cart_config_changed_slotmain(uint8_t mode_phi1, uint8_t mode_phi2, unsigned int wflag);
void cart_set_port_exrom_slotmain(int n);
void cart_set_port_game_slotmain(int n);
void cart_set_port_phi1_slotmain(int n);
void cart_set_port_phi2_slotmain(int n);
void cart_port_config_changed_slotmain(void);

#endif /* CARTRIDGE_INCLUDE_SLOTMAIN_API */

void cart_passthrough_changed(void);

#endif
