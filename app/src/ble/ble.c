#include <zephyr/kernel.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/uuid.h>
#include <src/ble/ble.h>

/*PMIC characteristic UUID: 637a4ded-0584-4456-9735-ee98f4564d57 */
#define BT_UUID_PMIC_CHAR_VAL \
    BT_UUID_128_ENCODE(0x637a4ded, 0x0584, 0x4456, 0x9735, 0xee98f4564d57)
#define BT_UUID_PMIC_CHAR BT_UUID_DECLARE_128(BT_UUID_PMIC_CHAR_VAL)

static const struct bt_data test_data[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR),
    BT_DATA_BYTES(BT_DATA_NAME_COMPLETE, 't', 'e', 's', 't'),
};

static ssize_t read_pmic(struct bt_conn *conn, const struct bt_gatt_attr *attr,
                         void *buf, uint16_t len, uint16_t offset)
{
    return bt_gatt_attr_read(conn, attr, buf, len, offset, &test_data, sizeof(test_data));
}

BT_GATT_SERVICE_DEFINE(
    pmic_service, BT_GATT_PRIMARY_SERVICE(BT_UUID_PMIC_CHAR_VAL),
    BT_GATT_CHARACTERISTIC(BT_UUID_PMIC_CHAR_VAL,
                           BT_GATT_CHRC_READ,
                           BT_GATT_PERM_READ,
                           read_pmic, NULL, NULL),
)

int ble_init(void)
{
    int err = bt_enable(NULL);

    print("Starting BLE \n");

    if (err) {
        printk("BLE init failed (err %d) \n", err);
        return 0;
    }

    print("BLE initalized \n");

    printk("BLE initialized \n");

    err = bt_le_adv_start(BT_LE_ADV_NCONN, test_data, ARRAY_SIZE(test_data), NULL, 0);
    if (err) {
        printk("Advertising failed to start (err %d) \n", err);
        return 0;
    }

    printk("Test is advertising \n");

    return 0;
}