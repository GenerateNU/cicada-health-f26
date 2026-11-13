#include <zephyr/kernel.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/uuid.h>
#include <ble/ble.h>
#include <ble/storage.h>

/*Servie UUID: 117173dc-c238-48d4-9b70-88b48f3134b1*/
#define BT_UUID_PMIC_SVC_VAL  BT_UUID_128_ENCODE(0x117173dc, 0xc238, 0x48d4, 0x9b70, 0x88b48f3134b)
#define BT_UUID_PMIC_SVC      BT_UUID_DECLARE_128(BT_UUID_PMIC_SVC_VAL)

/*PMIC characteristic UUID: 637a4ded-0584-4456-9735-ee98f4564d57 */
#define BT_UUID_PMIC_CHAR_VAL BT_UUID_128_ENCODE(0x637a4ded, 0x0584, 0x4456, 0x9735, 0xee98f4564d57)
#define BT_UUID_PMIC_CHAR     BT_UUID_DECLARE_128(BT_UUID_PMIC_CHAR_VAL)

/* CTRL UUID: 6c50dfe2-646c-4117-b977-b3855939271b*/
#define BT_UUID_PMIC_CTRL_VAL BT_UUID_128_ENCODE(0x6c50dfe2, 0x646c, 0x4117, 0xb977, 0xb3855939271b)
#define BT_UUID_PMIC_CTRL     BT_UUID_DECLARE_128(BT_UUID_PMIC_CTRL_VAL)

#define NOTIFY_INTERVAL 500

static bool notify_enabled;
static struct bt_conn *cur_conn;

struct pack_ctx {
    uint8_t pkt[244];
    uint16_t len;
    uint16_t max;
};

static const struct bt_data test_data[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR),
    BT_DATA(BT_DATA_NAME_COMPLETE, CONFIG_BT_DEVICE_NAME, sizeof(CONFIG_BT_DEVICE_NAME) - 1),
};

static ssize_t read_pmic(struct bt_conn *conn, const struct bt_gatt_attr *attr,
                         void *buf, uint16_t len, uint16_t offset)
{
    return bt_gatt_attr_read(conn, attr, buf, len, offset, &test_data, sizeof(test_data));
}

K_MSGQ_DEFINE(cmd_q, sizeof(uint8_t), 4, 1);

static ssize_t ctrl_write(struct bt_conn *conn, const struct bt_gatt_attr *attr,
                          const void *buf, uint16_t len, uint16_t offset, uint8_t flags)
{
    k_msgq_put(&cmd_q, buf, K_NO_WAIT);
    return len;
}

static void data_ccc_changed(const struct bt_gatt_attr *attr, uint16_t value){
    printk("enable notfications \n");
    notify_enabled = (value == BT_GATT_CCC_NOTIFY);
}

BT_GATT_SERVICE_DEFINE(
    pmic_service, BT_GATT_PRIMARY_SERVICE(BT_UUID_PMIC_SVC),

    //notify from device to phone
    BT_GATT_CHARACTERISTIC(BT_UUID_PMIC_CHAR,
                           BT_GATT_CHRC_NOTIFY,
                           BT_GATT_PERM_NONE,
                           NULL, NULL, NULL),
    BT_GATT_CCC(data_ccc_changed, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),

    //phone to device
    BT_GATT_CHARACTERISTIC(BT_UUID_PMIC_CTRL,
                           BT_GATT_CHRC_WRITE,
                           BT_GATT_PERM_WRITE,
                           NULL, ctrl_write, NULL),
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

BT_CONN_CB_DEFINE(conn_callbacks) = {
    .connected = connected,
    .disconnected = disconnected,
    .recycled = recycled,
};

int ble_wait_for_command(uint8_t *cmd, k_timeout_t timeout){
    return k_msgq_get(&cmd_q, cmd, timeout);
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

    return 0;
}

int send_storage_data(void) {
    struct fcb_entry loc = {0};
    uint8_t buf[32];
    uint16_t len;
    int err = 0;
    int count = 0;

    while (err == 0 && storage_read_next(&loc, buf, sizeof(buf), &len) == 0){
        struct pmic_record r;
        char str[64];
        int message;
        
        memcpy(&r, buf, sizeof(r));
        message = snprintk(str, sizeof(str), "t=%u v=%u i=%d",
                     r.timestamp_ms, r.vbat_mv, r.ibat_ma);
        do{
            err = bt_gatt_notify(cur_conn, &pmic_service.attrs[1], str, message);
            if (err == -ENOMEM) {
                k_sleep(K_MSEC(10));
            }
        } while (err == -ENOMEM);

        if (err == 0) {
            count++;
        }
    }

    printk("sent %d records (err %d) \n", count, err);
    return err;
}