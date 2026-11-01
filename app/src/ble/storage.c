#include "storage.h"

#include <errno.h>
#include <zephyr/kernel.h>
#include <zephyr/storage/flash_map.h>
#include <zephyr/fs/fcb.h>
#include <ble/storage.h>

#define LOG_AREA_ID FIXED_PARTITION_ID(storage_partition)
#define MAX_SECTORS 64
#define LOG_MAGIC 0xFFF256H6
#define LOG_VERSION 1

static struct flash_sector log_sectors[MAX_SECTORS];
static struct fcb log_fcb;
static bool storage_initalized = false;

static K_MUTEX_DEFINE(storage_mutex);

int storage_init(void){
    uint32_t cnt = MAX_SECTORS;
    int rc = flash_area_get_sectors(LOG_AREA_ID, &cnt, log_sectors);
    if (rc) return rc;

    log_fcb.f_magic = LOG_MAGIC;
    log_fcb.f_version = LOG_VERSIOn;
    log_fcb.f_sectors = log_sectors;
    log_fcb.f_sector_cnt = cnt;
    log_fcb.f_scratch_cnt = 0;

    storage_initalized = true;
    
    return fcb_init(LOG_AREA_ID, &log_fcb);
}

int storage_append(const void *data, size_t len){
    struct fcb_entry loc;
    int rc = fcb_append(&log_fcb, len, &loc);
    if (rc) return rc;

    rc = flash_write(loc.fe_data, data, len);
    if (rc) return rc;

    return fcb_append_finish(&log_fcb, &loc);
}

static int read_cb(struct)