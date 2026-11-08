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
	.f_scratch_cnt = 1,
	.f_sectors = storage_sectors,

};

struct read_ctx {
	void *buf;
	size_t len;
};

int storage_init(void){
    uint32_t count = ARRAY_SIZE(storage_sectors);

	int rc = flash_area_get_sectors(STORAGE_ID, &count, storage_sectors);
	if (rc) {
		return rc;
	}

	storage_fcb.f_sector_cnt = count;
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

static int read_cb(struct fcb_entry_ctx *ctx, void *arg){
	struct read_ctx *r = arg;
	size_t len = MIN(ctx->loc.fe_data_len, r->len);

	return flash_area_read(ctx->fap, FCB_ENTRY_FA_DATA_OFF(ctx->loc), r->buf, len);
}

static int print_cb(struct fcb_entry_ctx *ctx, void *arg){
	uint8_t buf[32];
	uint16_t len = MIN(ctx->loc.fe_data_len, sizeof(buf));
	int rc;

	rc = flash_area_read(ctx->fap, FCB_ENTRY_FA_DATA_OFF(ctx->loc), buf, len);
	if (rc) {
		return rc;
	}

	printk("entry @0x%x, %u bytes:", (unsigned int)FCB_ENTRY_FA_DATA_OFF(ctx->loc),
	       ctx->loc.fe_data_len);
	for (int i = 0; i < len; i++) {
		printk(" %02x", buf[i]);
	}
	printk("\n");
	return 0;
}

int storage_read(void *buf, size_t len){
	struct read_ctx ctx = {
		.buf = buf,
		.len = len,
	};

	return fcb_walk(&storage_fcb, NULL, print_cb, &ctx);
}





