// SPDX-License-Identifier: GPL-2.0-or-later
#include <hal.h>
#include <string.h>
#include "hal_efl.h"

// ST RM0383: the 512 KiB F411 has a final 128 KiB sector at 0x08060000.
// Check the whole erase unit before first use, even though the journal is smaller.
#define SECTOR_OFFSET 0x60000U
#define SECTOR_BYTES  0x20000U
static const uint8_t owner[16] = "Chalice Vial v1";
#define OWNER_OFFSET (SECTOR_OFFSET + SECTOR_BYTES - sizeof(owner))

_Static_assert(WEAR_LEVELING_EFL_FIRST_SECTOR == 7, "Unexpected settings sector");
_Static_assert(WEAR_LEVELING_BACKING_SIZE <= SECTOR_BYTES - sizeof(owner), "Settings journal overlaps marker");

bool __real_backing_store_unlock(void);
bool __real_backing_store_erase(void);

static bool mark_sector(void) {
    return flashProgram((BaseFlash *)&EFLD1, OWNER_OFFSET, sizeof(owner), owner) == FLASH_NO_ERROR;
}

bool __wrap_backing_store_unlock(void) {
    const uint8_t *tag = (const uint8_t *)(FLASH_BASE + OWNER_OFFSET);
    if (memcmp(tag, owner, sizeof(owner)) == 0) {
        return __real_backing_store_unlock();
    }

    // A partially written marker can be finished after loss of power, but
    // the rest of the sector must still be blank in that case.
    for (const uint8_t *p = (const uint8_t *)(FLASH_BASE + SECTOR_OFFSET); p < tag; ++p) {
        if (*p != 0xFF) {
            return false;
        }
    }
    for (size_t i = 0; i < sizeof(owner); ++i) {
        if ((tag[i] & owner[i]) != owner[i]) {
            return false;
        }
    }
    if (!__real_backing_store_unlock()) {
        return false;
    }
    if (mark_sector()) {
        return true;
    }
    eflStop(&EFLD1);
    return false;
}

bool __wrap_backing_store_erase(void) {
    return __real_backing_store_erase() && mark_sector();
}
