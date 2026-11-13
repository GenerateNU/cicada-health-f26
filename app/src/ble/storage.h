#ifndef STORAGE_H_
#define STORAGE_H_

#include <stdint.h>
#include <stddef.h>
#include <zephyr/fs/fcb.h>

struct pmic_record {
    uint32_t timestamp_ms;   // when the sample was taken
    uint16_t vbat_mv;        // battery voltage in millivolts
    int16_t  ibat_ma;        // battery current in mA (negative = discharging)
} __packed;

int storage_init(void);
int storage_write(void *data, size_t len);
int storage_clear(void);
int storage_read(void);
int storage_read_next(struct fcb_entry *loc, uint8_t *buf, uint16_t buf_size, uint16_t *len_out);
int storage_log_data(void);

#endif /* STORAGE_H_ */