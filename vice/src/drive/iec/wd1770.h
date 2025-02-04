/*
 * wd1770.h - WD1770/1772 emulation for the 1571 and 1581 disk drives.
 *
 * Rewritten by
 *  Kajtar Zsolt <soci@c64.rulez.org>
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

#ifndef VICE_WD1770_H
#define VICE_WD1770_H

#include "types.h"

struct disk_image_s;
struct diskunit_context_s;
struct snapshot_s;
typedef struct wd1770_s wd1770_t;

/* FIXME: whats the deal with the different prefixes? */
void wd1770d_init(struct diskunit_context_s *drv);
void wd1770_shutdown(wd1770_t *drv);

int wd1770_attach_image(struct disk_image_s *image, unsigned int unit);
int wd1770_detach_image(struct disk_image_s *image, unsigned int unit);

int wd1770_disk_change(wd1770_t *drv);
void wd1770d_store(struct diskunit_context_s *drv, uint16_t addr, uint8_t byte);
uint8_t wd1770d_read(struct diskunit_context_s *drv, uint16_t addr);
uint8_t wd1770d_peek(struct diskunit_context_s *drv, uint16_t addr);
void wd1770_reset(wd1770_t *drv);
void wd1770_set_side(wd1770_t *drv, int side);
void wd1770_set_motor(wd1770_t *drv, int on);

int wd1770_snapshot_write_module(wd1770_t *drv, struct snapshot_s *s);
int wd1770_snapshot_read_module(wd1770_t *drv, struct snapshot_s *s);

#endif
