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

}

int storage_init(void){
    uint32_t count = ARRAY_SIZE(storage_sectors);

	int rc = flash_get_sectors(STORAGE_ID, storage_sectors, &count);
	if (rc) {
		return rc;
	}

	storage_fcb.f_sector_count = count;
	return fcb_init(&storage_fcb, STORAGE_ID);
}

int storage_write(void *data, size_t len){
    return fcb_append(&storage_fcb, len, data);
}

int storage_erase(void){
	return fcb_rotate(&storage_fcb);
}



