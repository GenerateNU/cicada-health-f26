#include "storage.h"

#include <zephyr/kernel.h>
#include <zephyr/storage/flash_map.h>
#include <zephyr/fs/fcb.h>
#include <ble/storage.h>


#define STORAGE_ID FIXED_PARTITION_ID(storage_partition)
#define MAX_SECTORS 10

static struct flash_sector storage_sectors[MAX_SECTORS];

struct fcb storage_fcb = {
	.f_magic = 0xC1CADA11,
	.f_version = 1,
	.f_sector_count = 0,
	.f_scratch_cnt = 1,
	.f_sectors = storage_sectors,

};

int storage_init(void){
    uint32_t count = ARRAY_SIZE(storage_sectors);

	int rc = flash_area_get_sectors(STORAGE_ID, &count, storage_sectors);
	if (rc) {
		return rc;
	}

	storage_fcb.f_sector_count = count;
	return fcb_init(STORAGE_ID, &storage_fcb);
}

int storage_write(void *data, size_t len){
	struct fcb_entry loc;
	int rc;

	if (data == NULL || len == 0) {
		return -EINVAL;
	}

	rc = fcb_append(&storage_fcb, len, &loc);
	if (rc == -ENOSPC) {
		rc = fcb_rotate(&storage_fcb);
		if (rc == 0) {
			rc = fcb_append(&storage_fcb, len, &loc);
		}
	}

	if (rc){
		return rc;
	}

	rc = flash_area_write(storage_fcb.fap, FCB_ENTRY_FA_DATA_OFF(loc), data, len);
	if (rc) {
		return rc;
	}

    return fcb_append_finish(&storage_fcb, &loc);
}

int storage_erase_oldest(void){
	return fcb_rotate(&storage_fcb);
}



