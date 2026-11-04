#include <zephyr/kernel.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/uuid.h>
#include <ble/ble.h>

/*PMIC characteristic UUID: 637a4ded-0584-4456-9735-ee98f4564d57 */
#define BT_UUID_PMIC_CHAR_VAL BT_UUID_128_ENCODE(0x637a4ded, 0x0584, 0x4456, 0x9735, 0xee98f4564d57)
#define BT_UUID_PMIC_CHAR BT_UUID_DECLARE_128(BT_UUID_PMIC_CHAR_VAL)

#define BT_UUID_PMIC_SVC  BT_UUID_DECLARE_128(BT_UUID_PMIC_CHAR_VAL)
#define BT_UUID_PMIC_CHAR BT_UUID_DECLARE_128(BT_UUID_PMIC_CHAR_VAL)

#define NOTIFY_INTERVAL 500

static uint16_t pmic_value = 0;

static const struct bt_data test_data[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR),
    BT_DATA(BT_DATA_NAME_COMPLETE, CONFIG_BT_DEVICE_NAME, sizeof(CONFIG_BT_DEVICE_NAME) - 1),
};

static ssize_t read_pmic(struct bt_conn *conn, const struct bt_gatt_attr *attr,
                         void *buf, uint16_t len, uint16_t offset)
{
    return bt_gatt_attr_read(conn, attr, buf, len, offset, &test_data, sizeof(test_data));
}

BT_GATT_SERVICE_DEFINE(
    pmic_service, BT_GATT_PRIMARY_SERVICE(BT_UUID_PMIC_SVC),
    BT_GATT_CHARACTERISTIC(BT_UUID_PMIC_CHAR,
                           BT_GATT_CHRC_READ,
                           BT_GATT_PERM_READ,
                           read_pmic, NULL, NULL),
);

static void adv_restart(struct k_work *work)
{
    int err = bt_le_adv_start(BT_LE_ADV_CONN_FAST_1, test_data, ARRAY_SIZE(test_data), NULL, 0);
    if (err) {
        printk("Advertising failed to restart (err %d) \n", err);
    } else {
        printk("Advertising restarted \n");
    }
}
static K_WORK_DEFINE(adv_work, adv_restart);

static void connected(struct bt_conn *conn, uint8_t err)
{
    if (err) {
        printk("Connection failed (err %u)\n", err);
    } else {
        printk("Connected\n");
    }
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
    printk("Disconnected (reason %u)\n", reason);
}

static void recycled(struct bt_conn *conn, uint8_t reason)
{
    printk("Recycled (reason %u)\n", reason);
}

static void 
BT_CONN_CB_DEFINE(conn_callbacks) = {
    .connected = connected,
    .disconnected = disconnected,
    .recycled = recycled,
};

static void test_data(void) {
    int value = 0;
    value++;
    if (value == 200){
        value = 100;
    }
}

void send_data_thread(void) {
    while(1){
        test_data();
        send_test_data(value);
        k_sleep(K_MSEC(NOTIFY_INTERVAL));
    }
}

int ble_init(void)
{
    printk("Starting BLE \n");

    int err = bt_enable(NULL);

    if (err) {
        printk("BLE init failed (err %d) \n", err);
        return 0;
    }

    printk("BLE initalized \n");

    err = bt_le_adv_start(BT_LE_ADV_CONN_FAST_1, test_data, ARRAY_SIZE(test_data), NULL, 0);
    if (err) {
        printk("Advertising failed to start (err %d) \n", err);
        return 0;
    }

    printk("Test is advertising \n");

    return 0;
}