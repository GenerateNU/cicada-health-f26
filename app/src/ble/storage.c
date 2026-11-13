#include "storage.h"

#include <zephyr/kernel.h>
#include <zephyr/storage/flash_map.h>
#include <zephyr/fs/fcb.h>
#include <ble/storage.h>
#include <zephyr/random/random.h>


#define STORAGE_ID FIXED_PARTITION_ID(storage_partition)
#define MAX_SECTORS 10

static struct flash_sector storage_sectors[MAX_SECTORS];
struct fcb_entry loc;
static struct pmic_record rec;

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
	int rc;
	struct fcb_entry loc;

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

int storage_clear(void){
	return fcb_clear(&storage_fcb);
}

static int read_cb(struct fcb_entry_ctx *ctx, void *arg){
	struct read_ctx *r = arg;
	size_t len = MIN(ctx->loc.fe_data_len, r->len);

	return flash_area_read(ctx->fap, FCB_ENTRY_FA_DATA_OFF(ctx->loc), r->buf, len);
}

static int print_cb(struct fcb_entry_ctx *ctx, void *arg){
    if (ctx->loc.fe_data_len == sizeof(rec)) {
        flash_area_read(ctx->fap, FCB_ENTRY_FA_DATA_OFF(ctx->loc), &rec, sizeof(rec));
        printk("t=%u ms  vbat=%u mV  ibat=%d mA \n",
               rec.timestamp_ms, rec.vbat_mv, rec.ibat_ma);
    }
    return 0;
}

int storage_read(void){
	printk("\n---------------------------------\n");
	int rc = fcb_walk(&storage_fcb, NULL, print_cb, NULL);
	printk("-------------- end --------------\n\n");
	return rc;
}

int storage_read_next(struct fcb_entry *loc, uint8_t *buf, uint16_t buf_size, uint16_t *len_out){
	int rc = -ENOENT;
	bool target = false; //flag to check if a record has been found

	while (!target && fcb_getnext(&storage_fcb, loc) == 0){
		uint16_t len = loc->fe_data_len;

		if(len <= buf_size){
			rc = flash_area_read(storage_fcb.fap, FCB_ENTRY_FA_DATA_OFF((*loc)), buf, len);
			if (rc == 0){
				*len_out = len;
			}
			target = true;
		}
	}

	return rc;
}

static void fake_pmic_data(struct pmic_record *rec){
    static uint16_t vbat = 4200;     // start at a full Li-ion cell

    if (vbat > 3300) {
        vbat -= 1;                   // drop 1 mV per sample
    }

    rec->timestamp_ms = k_uptime_get_32();
    rec->vbat_mv  = vbat + (sys_rand32_get() % 5);          // +0..4 mV noise
    rec->ibat_ma  = -5 - (int16_t)(sys_rand32_get() % 3);   // about -5 to -7 mA
}

int storage_log_data(void){
	fake_pmic_data(&rec);
	return storage_write(&rec, sizeof(rec));
}






