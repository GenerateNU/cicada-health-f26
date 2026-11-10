#ifndef BLE_H
#define BLE_H

int ble_init(void);

static void adv_restart(struct k_work *work);

static void connected(struct bt_conn *conn, uint8_t err);
static void disconnected(struct bt_conn *conn, uint8_t reason);
static void recycled(struct bt_conn *conn, uint8_t reason);

int ble_init(void);
int send_storage_data(const uint16_t *data, uint16_t len, void *user);

#endif // BLE_H