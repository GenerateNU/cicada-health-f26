// #include <stdio.h>
#include <zephyr/kernel.h>
// zephyr/device.h is included eventually through kernel.h
#include <zephyr/drivers/gpio.h>
#include "led_strip.h"
#include "ble/ble.h"
#include "ble/storage.h"

// This is accessing the DeviceTree node through its label (can see the entire devicetree in generated zephyr.dts file in the build directory)
#define LED2 DT_ALIAS(led0)

// 'gpios' refers to the property name within the led0 node, which is itself inside the leds parent node in the devicetree
// you can see this^^ in code if you search for the 'zephyr/boards/nordic/nrf54l15dk/nrf54l15dk_common.dtsi' file
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED2, gpios);

int main(void)
{
    printk("booting  \n");

    //intialize flash
    int ret = storage_init();
    if (ret < 0){
        printk("Flash init failed: %d", ret);
    }

    printk("flash init successful \n");

    storage_read();
    storage_clear();

    //write fake data into flash
    printk("start logging \n");
    for (int i = 0; i < 10; i++){
        storage_log_data();
        k_sleep(K_SECONDS(1));
    }
    printk("wrote to flash \n");

    //intialize ble
    ret = ble_init();
    if (ret < 0){
        printk("BLE init failed: %d", ret);
    }

    printk("ble init successful \n");
    k_msleep(3000);

    //transfer data and then clear flash
    while (1) {
        uint8_t cmd;
        ble_wait_for_command(&cmd, K_FOREVER);
        printk("waiting for command \n");

        if (cmd == 0x01) {
            printk("send data \n");
            send_storage_data();
            printk("sent data \n");
        } else if (cmd == 0x02) {
            printk("clear \n");
            storage_clear();
        }
    }

    storage_read();
    printk("cleared flash \n");

    return 0;
}