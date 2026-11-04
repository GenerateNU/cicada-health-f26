#ifndef BLE_H
#define BLE_H

int ble_init(void);
static ssize_t read_pmic(struct bt_conn *conn, const struct bt_gatt_attr *attr,
                         void *buf, uint16_t len, uint16_t offset);

static void adv_restart(struct k_work *work);

static void connected(struct bt_conn *conn, uint8_t err);
static void disconnected(struct bt_conn *conn, uint8_t reason);
static void recycled(struct bt_conn *conn, uint8_t reason);

int ble_init(void);

#endif // BLE_H