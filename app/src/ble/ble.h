#ifndef BLE_H
#define BLE_H

int ble_init(void);

int ble_wait_for_command(uint8_t *cmd, k_timeout_t timeout);
int ble_init(void);
int send_storage_data(void);

#endif // BLE_H