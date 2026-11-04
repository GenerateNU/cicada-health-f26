#include "storage.h"

#include <errno.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <devicetree.h>
#include <zephyr/drivers/flash.h>
#include <zephyr/sys/util.h>
#include <zephyr/sys/crc.h>
#include <zephyr/storage/flash_map.h>
#include <zephyr/fs/fcb.h>
#include <ble/storage.h>

#define TEST_PARTITION storage_partition
#define TEST_PARTITION_OFFSET PARTITION_OFFSET(storage_partition)
#define TEST_PARTITION_DEVICE PARTITION_DEVICE(storage_partition)

#if defined(CONFIG_SOC_NRF54H20)
#define FLASH_PAGE_SIZE 2048    //use flash_get_page_info_by_offs() to get page size
#else
#define FLASH_PAGE_SIZE 4096
#endif

#define TEST_DATA_SIZE_IN_BYTES 128
#define FLASH_TEST_OFFSET2      0x41234
#define FLASH_TEST_PAGE_IDX     37

#if defined(CONFIG_FLASH_HAS_EXPLICIT_ERASE) &&	\
	defined(CONFIG_FLASH_HAS_NO_EXPLICIT_ERASE)
#define FLASH_PE_RUNTIME_CHECK(cond) (cond)
#elif defined(CONFIG_FLASH_HAS_EXPLICIT_ERASE)
#define FLASH_PE_RUNTIME_CHECK(cond) (true)
#else
/* Assumed IS_ENABLED(CONFIG_FLASH_HAS_NO_EXPLICIT_ERASE) */
#define FLASH_PE_RUNTIME_CHECK(cond) (false)
#endif


static K_MUTEX_DEFINE(storage_mutex);

int storage_init(void){
    uint32_t offset;
    const struct device *flash_dev = DEVICE_DT_GET(TEST_PARTITION_DEVICE);
    struct flash_parameters flash_params;

    uint8_t test_data[TEST_DATA_SIZE_IN_BYTES];

    if (!device_is_ready(flash_dev)) {
        printk("Flash device is not ready \n");
        return 0;
    }

    printk("Internal storage erases page at offset 0x%x\n", TEST_PARTITION_OFFSET);
    erase_when_needed(flash_dev,
			  flash_params_get_erase_cap(&flash_params) & FLASH_ERASE_C_EXPLICIT,
			  TEST_PARTITION_OFFSET, FLASH_PAGE_SIZE);
}

int storage_write(const struct device *flash_dev, uint8_t *test_data, uint8_t write_block_size, uint8_t offset){
    uint32_t i, offset;
}

static void erase_when_needed(const struct device *dev, bool condition,
			     uint32_t off, uint32_t size)
{
	/*
	 * Alwayes invoke erase when there are only erase requiring devices,
	 * never invoke erase when there are no devices requiring erase,
	 * always check condition if there are both kind of devices.
	 */
	if (FLASH_PE_RUNTIME_CHECK(condition)) {
		if (flash_erase(dev, off, size) != 0) {
			printf("   Erase failed!\n");
		} else {
			printf("   Erase succeeded!\n");
		}
	} else {
		(void)condition;
		printf("   Erase not required by device\n");
	}
}

