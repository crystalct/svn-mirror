/*
 * plus4io.c - Plus4 io handling ($FD00-$FEFF).
 *
 * Written by
 *  Marco van den Heuvel <blackystardust68@yahoo.com>
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

#include "vice.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "cartio.h"
#include "cartridge.h"
#include "cmdline.h"
#include "lib.h"
#include "log.h"
#include "monitor.h"
#include "plus4mem.h"
#include "resources.h"
#include "types.h"
#include "uiapi.h"
#include "util.h"

/* #define IODEBUG */
/* #define IODEBUGRW */

#ifdef IODEBUG
#define DBG(x) printf x
#else
#define DBG(x)
#endif

#ifdef IODEBUGRW
#define DBGRW(x) printf x
#else
#define DBGRW(x)
#endif

/* ---------------------------------------------------------------------------------------------------------- */

static int io_source_collision_handling = 0;
static unsigned int order = 0;

/* ---------------------------------------------------------------------------------------------------------- */

static io_source_list_t plus4io_fd00_head = { NULL, NULL, NULL };
static io_source_list_t plus4io_fe00_head = { NULL, NULL, NULL };

static void io_source_detach(io_source_detach_t *source)
{
    switch (source->det_id) {
        case IO_DETACH_CART:
            if (source->det_cartid != CARTRIDGE_NONE) {
#ifdef IODEBUG
                if (source->det_cartid == 0) {
                    DBG(("IO: cart id in io struct is 0, it should be updated! name: %s\n", source->det_devname));
                } else {
                    DBG(("IO: io_source_detach id:%d name: %s\n", source->det_cartid, source->det_devname));
                }
#endif
                assert(source->det_cartid != CARTRIDGE_CRT); /* CARTRIDGE_CRT is not allowed at this point */
                cartridge_detach_image(source->det_cartid);
            }
            break;
        case IO_DETACH_RESOURCE:
            resources_set_int(source->det_name, 0);
            break;
    }
}

/*
    amount is 2 or more
*/
static void io_source_msg_detach_all(uint16_t addr, int amount, io_source_list_t *start)
{
    io_source_detach_t *detach_list = lib_malloc(sizeof(io_source_detach_t) * amount);
    io_source_list_t *current = start;
    char *old_msg = NULL;
    char *new_msg = NULL;
    int found = 0;
    int i = 0;

    current = current->next;

    DBG(("IO: check %d sources for addr %04x\n", amount, addr));
    while (current) {
        /* DBG(("IO: check '%s'\n", current->device->name)); */
        if (current->device->io_source_valid &&
            addr >= current->device->start_address &&
            addr <= current->device->end_address &&
            current->device->io_source_prio == IO_PRIO_NORMAL) {
            /* found a conflict */
            detach_list[found].det_id = current->device->detach_id;
            detach_list[found].det_name = current->device->resource_name;
            detach_list[found].det_devname = current->device->name;
            detach_list[found].det_cartid = current->device->cart_id;
            DBG(("IO: found #%d: '%s'\n", found, current->device->name));

            /* first part of the message "read collision at x from" */
            if (found == 0) {
                old_msg = lib_strdup("I/O read collision at %X from ");
                new_msg = util_concat(old_msg, current->device->name, NULL);
                lib_free(old_msg);
            }
            if ((found != amount - 1) && (found != 0)) {
                old_msg = new_msg;
                new_msg = util_concat(old_msg, ", ", current->device->name, NULL);
                lib_free(old_msg);
            }
            if (found == amount - 1) {
                old_msg = new_msg;
                new_msg = util_concat(old_msg, " and ", current->device->name, ".\nAll the named devices will be detached.", NULL);
                lib_free(old_msg);
            }
            found++;
            if (found == amount) {
                break;
            }
        }
        current = current->next;
    }

    if (found) {
        log_message(LOG_DEFAULT, new_msg, addr);
        ui_error(new_msg, addr);
        lib_free(new_msg);

        DBG(("IO: found %d items to detach\n", found));
        for (i = 0; i < found; i++) {
            DBG(("IO: detach #%d id:%d name: %s\n", i, detach_list[i].det_cartid, detach_list[i].det_devname));
            io_source_detach(&detach_list[i]);
        }
    }
    lib_free(detach_list);
}

/*
    amount is 2 or more
*/
static void io_source_msg_detach_last(uint16_t addr, int amount, io_source_list_t *start, unsigned int lowest)
{
    io_source_detach_t *detach_list = lib_malloc(sizeof(io_source_detach_t) * amount);
    io_source_list_t *current = start;
    char *old_msg = NULL;
    char *new_msg = NULL;
    char *first_cart = NULL;
    int found = 0;
    int i = 0;

    current = current->next;

    DBG(("IO: check %d sources for addr %04x\n", real_amount, addr));
    while (current) {
        /* DBG(("IO: check '%s'\n", current->device->name)); */
        if (current->device->io_source_valid &&
            addr >= current->device->start_address &&
            addr <= current->device->end_address &&
            current->device->io_source_prio == IO_PRIO_NORMAL) {
            /* found a conflict */
            detach_list[found].det_id = current->device->detach_id;
            detach_list[found].det_name = current->device->resource_name;
            detach_list[found].det_devname = current->device->name;
            detach_list[found].det_cartid = current->device->cart_id;
            detach_list[found].order = current->device->order;
            DBG(("IO: found #%d: '%s'\n", found, current->device->name));

            if (current->device->order == lowest) {
                first_cart = current->device->name;
            }

            /* first part of the message "read collision at x from" */
            if (found == 0) {
                old_msg = lib_strdup("I/O read collision at %X from ");
                new_msg = util_concat(old_msg, current->device->name, NULL);
                lib_free(old_msg);
            }
            if ((found != amount - 1) && (found != 0)) {
                old_msg = new_msg;
                new_msg = util_concat(old_msg, ", ", current->device->name, NULL);
                lib_free(old_msg);
            }
            if (found == amount - 1) {
                old_msg = new_msg;
                new_msg = util_concat(old_msg, " and ", current->device->name, ".\nAll devices except ", first_cart, " will be detached.", NULL);
                lib_free(old_msg);
            }
            found++;
            if (found == amount) {
                break;
            }
        }
        current = current->next;
    }

    if (found) {
        log_message(LOG_DEFAULT, new_msg, addr);
        ui_error(new_msg, addr);
        lib_free(new_msg);

        DBG(("IO: found %d items to detach\n", found));
        for (i = 0; i < found; i++) {
            if (detach_list[i].order != lowest) {
                DBG(("IO: detach #%d id:%d name: %s\n", i, detach_list[i].det_cartid, detach_list[i].det_devname));
                io_source_detach(&detach_list[i]);
            }
        }
    }
    lib_free(detach_list);
}

/*
    amount is 2 or more
*/
static void io_source_log_collisions(uint16_t addr, int amount, io_source_list_t *start)
{
    io_source_list_t *current = start;
    char *old_msg = NULL;
    char *new_msg = NULL;
    int found = 0;

    current = current->next;

    DBG(("IO: check %d sources for addr %04x\n", amount, addr));
    while (current) {
        /* DBG(("IO: check '%s'\n", current->device->name)); */
        if (current->device->io_source_valid &&
            addr >= current->device->start_address &&
            addr <= current->device->end_address &&
            current->device->io_source_prio == IO_PRIO_NORMAL) {
            /* found a conflict */
            DBG(("IO: found #%d: '%s'\n", found, current->device->name));

            /* first part of the message "read collision at x from" */
            if (found == 0) {
                old_msg = lib_strdup("I/O read collision at %X from ");
                new_msg = util_concat(old_msg, current->device->name, NULL);
                lib_free(old_msg);
            }
            if ((found != amount - 1) && (found != 0)) {
                old_msg = new_msg;
                new_msg = util_concat(old_msg, ", ", current->device->name, NULL);
                lib_free(old_msg);
            }
            if (found == amount - 1) {
                old_msg = new_msg;
                new_msg = util_concat(old_msg, " and ", current->device->name, NULL);
                lib_free(old_msg);
            }
            found++;
            if (found == amount) {
                break;
            }
        }
        current = current->next;
    }

    if (found) {
        log_message(LOG_DEFAULT, new_msg, addr);
        lib_free(new_msg);
    }
}

static inline uint8_t io_read(io_source_list_t *list, uint16_t addr)
{
    io_source_list_t *current = list->next;
    int io_source_counter = 0;
    int io_source_valid = 0;
    uint8_t realval = 0;
    uint8_t retval = 0;
    uint8_t firstval = 0;
    unsigned int lowest_order = 0xffffffff;

    while (current) {
        if (current->device->read != NULL) {
            if ((addr >= current->device->start_address) && (addr <= current->device->end_address)) {
                retval = current->device->read((uint16_t)(addr & current->device->address_mask));
                if (current->device->io_source_valid) {
                    /* high prio always overrides others, return immediatly */
                    if (current->device->io_source_prio == IO_PRIO_HIGH) {
                        return retval;
                    }
                    if (io_source_valid == 0) {
                        /* on first valid read, initialize intermediate values */
                        firstval = realval = retval;
                        lowest_order = current->device->order;
                        /* do not count low prio, as it will always be overridden by others */
                        if (current->device->io_source_prio != IO_PRIO_LOW) {
                            io_source_counter++;
                        }
                        io_source_valid = 1;
                    } else {
                        /* ignore low prio reads when a real value is present already */
                        if (current->device->io_source_prio == IO_PRIO_LOW) {
                            retval = realval;
                        }
                        if (io_source_collision_handling == IO_COLLISION_METHOD_DETACH_LAST) {
                            if (current->device->order < lowest_order) {
                                lowest_order = current->device->order;
                                realval = retval;
                            }
                        } else if (io_source_collision_handling == IO_COLLISION_METHOD_AND_WIRES) {
                            realval &= retval;
                        }
                        /* do not count low prio, as it will always be overridden by others */
                        if (current->device->io_source_prio != IO_PRIO_LOW) {
                            /* if the nth read returns the same as the first read don't see it as a conflict */
                            if (retval != firstval) {
                                io_source_counter++;
                            }
                        }
                    }
                }
            }
        }
        current = current->next;
    }

    /* no valid I/O source was read, return phi1 */
    if (io_source_valid == 0) {
        return read_unused(addr);
    }
    /* only one valid I/O source was read, return value */
    if (!(io_source_counter > 1)) {
        return retval;
    }
    /* more than one I/O source was read, handle collision */
    if (io_source_collision_handling == IO_COLLISION_METHOD_DETACH_ALL) {
        io_source_msg_detach_all(addr, io_source_counter, list);
        return read_unused(addr);
    } else if (io_source_collision_handling == IO_COLLISION_METHOD_DETACH_LAST) {
        io_source_msg_detach_last(addr, io_source_counter, list, lowest_order);
        return realval;
    } else if (io_source_collision_handling == IO_COLLISION_METHOD_AND_WIRES) {
        io_source_log_collisions(addr, io_source_counter, list);
        return realval;
    }
    return read_unused(addr);
}

/* peek from I/O area with no side-effects */
static inline uint8_t io_peek(io_source_list_t *list, uint16_t addr)
{
    io_source_list_t *current = list->next;

    while (current) {
        if (addr >= current->device->start_address && addr <= current->device->end_address) {
            if (current->device->peek) {
                return current->device->peek((uint16_t)(addr & current->device->address_mask));
            } else if (current->device->read) {
                return current->device->read((uint16_t)(addr & current->device->address_mask));
            }
        }
        current = current->next;
    }

    return read_unused(addr);
}

static inline void io_store(io_source_list_t *list, uint16_t addr, uint8_t value)
{
    int writes = 0;
    uint16_t addy = 0xffff;
    io_source_list_t *current = list->next;
    void (*store)(uint16_t address, uint8_t data) = NULL;

    while (current) {
        if (current->device->store != NULL) {
            if (addr >= current->device->start_address && addr <= current->device->end_address) {
                /* delay mirror writes, ensuring real device writes in mirror area */
                if (current->device->io_source_prio != IO_PRIO_LOW) {
                    current->device->store((uint16_t)(addr & current->device->address_mask), value);
                    writes++;
                } else {
                    addy = (uint16_t)(addr & current->device->address_mask);
                    store = current->device->store;
                }
            }
        }
        current = current->next;
    }
    /* if a mirror write needed to be done and no real device write was done */
    if (store && !writes && addy != 0xffff) {
        store(addy, value);
    }
}

/* ---------------------------------------------------------------------------------------------------------- */

io_source_list_t *io_source_register(io_source_t *device)
{
    io_source_list_t *current = NULL;
    io_source_list_t *retval = lib_malloc(sizeof(io_source_list_t));

    assert(device != NULL);
    DBG(("IO: register id:%d name:%s\n", device->cart_id, device->name));

    switch (device->start_address & 0xff00) {
        case 0xfd00:
            current = &plus4io_fd00_head;
            break;
        case 0xfe00:
            current = &plus4io_fe00_head;
            break;
    }

    while (current->next != NULL) {
        current = current->next;
    }
    current->next = retval;
    retval->previous = current;
    retval->device = device;
    retval->next = NULL;
    retval->device->order = order++;

    return retval;
}

void io_source_unregister(io_source_list_t *device)
{
    io_source_list_t *prev;

    assert(device != NULL);
    DBG(("IO: unregister id:%d name:%s\n", device->device->cart_id, device->device->name));

    prev = device->previous;
    prev->next = device->next;

    if (device->next) {
        device->next->previous = prev;
    }

    if (device->device->order == order - 1) {
        if (order != 0) {
            order--;
        }
    }

    lib_free(device);
}

void cartio_shutdown(void)
{
    io_source_list_t *current;

    current = plus4io_fd00_head.next;
    while (current) {
        io_source_unregister(current);
        current = plus4io_fd00_head.next;
    }

    current = plus4io_fe00_head.next;
    while (current) {
        io_source_unregister(current);
        current = plus4io_fe00_head.next;
    }
}

void cartio_set_highest_order(unsigned int nr)
{
    order = nr;
}

/* ---------------------------------------------------------------------------------------------------------- */

uint8_t plus4io_fd00_read(uint16_t addr)
{
    DBGRW(("IO: io-fd00 r %04x\n", addr));
    return io_read(&plus4io_fd00_head, addr);
}

uint8_t plus4io_fd00_peek(uint16_t addr)
{
    DBGRW(("IO: io-fd00 p %04x\n", addr));
    return io_peek(&plus4io_fd00_head, addr);
}

void plus4io_fd00_store(uint16_t addr, uint8_t value)
{
    DBGRW(("IO: io-fd00 w %04x %02x\n", addr, value));
    io_store(&plus4io_fd00_head, addr, value);
}

uint8_t plus4io_fe00_read(uint16_t addr)
{
    DBGRW(("IO: io-fe00 r %04x\n", addr));
    return io_read(&plus4io_fe00_head, addr);
}

uint8_t plus4io_fe00_peek(uint16_t addr)
{
    DBGRW(("IO: io-fe00 p %04x\n", addr));
    return io_peek(&plus4io_fe00_head, addr);
}

void plus4io_fe00_store(uint16_t addr, uint8_t value)
{
    DBGRW(("IO: io-fe00 w %04x %02x\n", addr, value));
    io_store(&plus4io_fe00_head, addr, value);
}

/* ---------------------------------------------------------------------------------------------------------- */

static void io_source_ioreg_add_onelist(struct mem_ioreg_list_s **mem_ioreg_list, io_source_list_t *current)
{
    uint16_t end;

    while (current) {
        end = current->device->end_address;
        if (end > current->device->start_address + current->device->address_mask) {
            end = current->device->start_address + current->device->address_mask;
        }

        mon_ioreg_add_list(mem_ioreg_list, current->device->name, current->device->start_address, end, current->device->dump, NULL);
        current = current->next;
    }
}

/* add all registered I/O devices to the list for the monitor */
void io_source_ioreg_add_list(struct mem_ioreg_list_s **mem_ioreg_list)
{
    io_source_ioreg_add_onelist(mem_ioreg_list, plus4io_fd00_head.next);
    io_source_ioreg_add_onelist(mem_ioreg_list, plus4io_fe00_head.next);
}

/* ---------------------------------------------------------------------------------------------------------- */

static int set_io_source_collision_handling(int val, void *param)
{
    switch (val) {
        case IO_COLLISION_METHOD_DETACH_ALL:
        case IO_COLLISION_METHOD_DETACH_LAST:
        case IO_COLLISION_METHOD_AND_WIRES:
            break;
        default:
            return -1;
    }
    io_source_collision_handling = val;

    return 0;
}

static const resource_int_t resources_int[] = {
    { "IOCollisionHandling", IO_COLLISION_METHOD_DETACH_ALL, RES_EVENT_STRICT, (resource_value_t)0,
      &io_source_collision_handling, set_io_source_collision_handling, NULL },
    RESOURCE_INT_LIST_END
};

int cartio_resources_init(void)
{
    return resources_register_int(resources_int);
}

static const cmdline_option_t cmdline_options[] =
{
    { "-iocollision", SET_RESOURCE, CMDLINE_ATTRIB_NEED_ARGS,
      NULL, NULL, "IOCollisionHandling", NULL,
      "<method>", "Select the way the I/O collisions should be handled, (0: error message and detach all involved carts, 1: error message and detach last attached involved carts, 2: warning in log and 'AND' the valid return values" },
    CMDLINE_LIST_END
};

int cartio_cmdline_options_init(void)
{
    return cmdline_register_options(cmdline_options);
}
