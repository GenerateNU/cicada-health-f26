#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/led_strip.h>
#include <zephyr/logging/log.h>
#include "led_strip.h"

LOG_MODULE_REGISTER(led_strip, LOG_LEVEL_INF);

#define LED_STRIP_NODE DT_NODELABEL(led_strip)
#define NUM_LEDS 4
#define LEVEL_DELAY_MS 5000


static const struct device *led_strip_dev;

int led_strip_init(void)
{
    led_strip_dev = DEVICE_DT_GET(LED_STRIP_NODE);

    if (!device_is_ready(led_strip_dev)) {
        LOG_ERR("LED strip device not ready");
        return -ENODEV;
    }

    LOG_INF("LED strip device found: %s", led_strip_dev->name);
    LOG_INF("LED strip initialized with %zu LEDs", led_strip_get_length());

    // Test pattern to verify it's working
    LOG_INF("Testing LED strip...");
    led_strip_set_all(100, 100, 100); // White
    k_sleep(K_MSEC(500));
    led_strip_clear();
    LOG_INF("LED strip test complete");

    return 0;
}

int led_strip_set_all(uint8_t r, uint8_t g, uint8_t b)
{
    if (!led_strip_dev) {
        LOG_ERR("LED strip device not initialized");
        return -ENODEV;
    }

    size_t num_pixels = led_strip_get_length();
    struct led_rgb pixels[64]; // Fixed size array for safety

    if (num_pixels > 64) {
        LOG_WRN("LED strip has %zu LEDs, limiting to 64", num_pixels);
        num_pixels = 64; // Limit to prevent overflow
    }

    LOG_INF("Setting all %zu LEDs to RGB(%d, %d, %d)", num_pixels, r, g, b);

    for (size_t i = 0; i < num_pixels; i++) {
        pixels[i].r = r;
        pixels[i].g = g;
        pixels[i].b = b;
    }

    int ret = led_strip_update_rgb(led_strip_dev, pixels, num_pixels);
    if (ret < 0) {
        LOG_ERR("Failed to update LED strip: %d", ret);
    } else {
        LOG_INF("LED strip updated successfully");
    }

    return ret;
}

int led_strip_set_pixels(struct led_rgb *pixels, size_t num_pixels)
{
    if (!led_strip_dev) {
        return -ENODEV;
    }

    if (num_pixels > led_strip_get_length()) {
        return -EINVAL;
    }

    return led_strip_update_rgb(led_strip_dev, pixels, num_pixels);
}

int led_strip_clear(void)
{
    return led_strip_set_all(0, 0, 0);
}

size_t led_strip_get_length(void)
{
    if (!led_strip_dev) {
        return 0;
    }

    return led_strip_length(led_strip_dev);
}

int led_strip_rainbow(uint8_t brightness)
{
    if (!led_strip_dev) {
        return -ENODEV;
    }

    size_t num_pixels = led_strip_get_length();
    struct led_rgb pixels[64]; // Fixed size array for safety

    if (num_pixels > 64) {
        num_pixels = 64; // Limit to prevent overflow
    }

    // Predefined rainbow colors for simplicity
    const struct led_rgb rainbow_colors[] = {
        {255, 0, 0},     // Red
        {255, 127, 0},   // Orange
        {255, 255, 0},   // Yellow
        {0, 255, 0},     // Green
        {0, 0, 255},     // Blue
        {75, 0, 130},    // Indigo
        {148, 0, 211}    // Violet
    };
    const size_t num_colors = sizeof(rainbow_colors) / sizeof(rainbow_colors[0]);

    for (size_t i = 0; i < num_pixels; i++) {
        // Map pixel index to rainbow color
        size_t color_index = (i * num_colors) / num_pixels;
        if (color_index >= num_colors) {
            color_index = num_colors - 1;
        }

        pixels[i].r = (uint8_t)((rainbow_colors[color_index].r * brightness) / 255);
        pixels[i].g = (uint8_t)((rainbow_colors[color_index].g * brightness) / 255);
        pixels[i].b = (uint8_t)((rainbow_colors[color_index].b * brightness) / 255);
    }

    return led_strip_update_rgb(led_strip_dev, pixels, num_pixels);
}

int led_strip_chase(uint8_t r, uint8_t g, uint8_t b, uint32_t delay_ms)
{
    if (!led_strip_dev) {
        return -ENODEV;
    }

    size_t num_pixels = led_strip_get_length();
    struct led_rgb pixels[64]; // Fixed size array for safety

    if (num_pixels > 64) {
        num_pixels = 64; // Limit to prevent overflow
    }

    // Initialize all pixels to off
    memset(pixels, 0, sizeof(pixels));

    for (size_t i = 0; i < num_pixels; i++) {
        // Turn off the previous LED
        if (i > 0) {
            pixels[i - 1].r = 0;
            pixels[i - 1].g = 0;
            pixels[i - 1].b = 0;
        }

        // Turn on the current LED
        pixels[i].r = r;
        pixels[i].g = g;
        pixels[i].b = b;

        int ret = led_strip_update_rgb(led_strip_dev, pixels, num_pixels);
        if (ret < 0) {
            return ret;
        }

        k_sleep(K_MSEC(delay_ms));
    }

    // Turn off the last LED
    pixels[num_pixels - 1].r = 0;
    pixels[num_pixels - 1].g = 0;
    pixels[num_pixels - 1].b = 0;

    return led_strip_update_rgb(led_strip_dev, pixels, num_pixels);
}

int led_strip_power (void) {
    struct level{
        size_t count;
        uint8_t r, g, b;
    };

    static const struct level levels[] = {
        {1, 255, 0, 0 }, //1 red
        {2, 255, 180, 0 }, //2 yellow
        {3, 0, 255, 0 }, //3 green
        {4, 0, 255, 0 } //4 green
    };

    struct led_rgb pixels[NUM_LEDS];

    for (size_t i = 0; i < ARRAY_SIZE(levels); i++) {
        memset(pixels, 0, sizeof(pixels)); //function turns pixels off

        for (size_t led = 0; led < levels[i].count; led++){
            pixels[led].r = levels[i].r;
            pixels[led].g = levels[i].g;
            pixels[led].b = levels[i].b;
        }

        int ret = led_strip_set_pixels(pixels, NUM_LEDS);
        if(ret < 0){
            return ret;
        }
        k_sleep(K_MSEC(LEVEL_DELAY_MS));
    }
    return 0;
}

int led_strip_clean (void) {
    #define NUM_LEDS 4
    #define LEVEL_DELAY_MS 3000
    #define BLINK_MS    300

    struct level{
        size_t count;
        uint8_t r, g, b;
    };

    static const struct level levels[] = {
        {1, 0, 0, 255 }, //1 red
        {2, 0, 0, 255 }, //2 blue
        {3, 0, 0, 255 }, //3 green
        {4, 0, 0, 255 }, //4 blue
    };

    struct led_rgb pixels[NUM_LEDS];
    int ret;

    for (size_t i = 0; i < ARRAY_SIZE(levels); i++){
        memset(pixels, 0, sizeof(pixels)); //function turns pixels off

        //on mode
        for (uint32_t t = 0; t < LEVEL_DELAY_MS; t += 2 * BLINK_MS){
            for (size_t led = 0; led < levels[i].count; led++){
                pixels[led].r = levels[i].r;
                pixels[led].g = levels[i].g;
                pixels[led].b = levels[i].b;
            }
            ret = led_strip_set_pixels(pixels, NUM_LEDS);
            if (ret < 0) {
                return ret;
            }
            k_sleep(K_MSEC(BLINK_MS));

            //off mode
            memset(pixels, 0, sizeof(pixels));
            ret = led_strip_set_pixels(pixels, NUM_LEDS);
            if (ret < 0) {
                return ret;
            }
            k_sleep(K_MSEC(BLINK_MS));
        }
    }
}

int led_strip_test(void)
{
    LOG_INF("Running LED strip test...");

    // Test red
    LOG_INF("Testing red...");
    led_strip_set_all(255, 0, 0);
    k_sleep(K_MSEC(1000));

    // Test green
    LOG_INF("Testing green...");
    led_strip_set_all(0, 255, 0);
    k_sleep(K_MSEC(1000));

    // Test blue
    LOG_INF("Testing blue...");
    led_strip_set_all(0, 0, 255);
    k_sleep(K_MSEC(1000));

    // Test white
    LOG_INF("Testing white...");
    led_strip_set_all(255, 255, 255);
    k_sleep(K_MSEC(1000));

    // Test individual LEDs
    LOG_INF("Testing individual LEDs...");
    struct led_rgb pixels[8] = {0};
    for (int i = 0; i < 8; i++) {
        pixels[i].r = 255;  // Red
        led_strip_update_rgb(led_strip_dev, pixels, 8);
        k_sleep(K_MSEC(200));
        pixels[i].r = 0;
    }

    // Clear
    LOG_INF("Clearing LEDs...");
    led_strip_clear();

    LOG_INF("LED strip test complete");
    return 0;
}