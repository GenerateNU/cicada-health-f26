#ifndef LED_STRIP_H_
#define LED_STRIP_H_

#include <zephyr/drivers/led_strip.h>

/**
 * @brief Initialize the LED strip
 *
 * @return 0 on success, negative errno on failure
 */
int led_strip_init(void);

/**
 * @brief Set all LEDs to a single color
 *
 * @param r Red component (0-255)
 * @param g Green component (0-255)
 * @param b Blue component (0-255)
 * @return 0 on success, negative errno on failure
 */
int led_strip_set_all(uint8_t r, uint8_t g, uint8_t b);

/**
 * @brief Set individual LED colors
 *
 * @param pixels Array of RGB values for each LED
 * @param num_pixels Number of pixels to set
 * @return 0 on success, negative errno on failure
 */
int led_strip_set_pixels(struct led_rgb *pixels, size_t num_pixels);

/**
 * @brief Clear all LEDs (turn them off)
 *
 * @return 0 on success, negative errno on failure
 */
int led_strip_clear(void);

/**
 * @brief Get the number of LEDs in the strip
 *
 * @return Number of LEDs
 */
size_t led_strip_get_length(void);

/**
 * @brief Create a rainbow effect across the LED strip
 *
 * @param brightness Brightness multiplier (0-255)
 * @return 0 on success, negative errno on failure
 */
int led_strip_rainbow(uint8_t brightness);

/**
 * @brief Create a chasing light effect
 *
 * @param r Red component (0-255)
 * @param g Green component (0-255)
 * @param b Blue component (0-255)
 * @param delay_ms Delay between updates in milliseconds
 * @return 0 on success, negative errno on failure
 */
int led_strip_chase(uint8_t r, uint8_t g, uint8_t b, uint32_t delay_ms);

/**
 * @brief Different levels of LED colors and count based after certain time
 *
 * @return 0 on success, negative errno on failure
 */
int led_strip_main(void);

int led_strip_charge(void);

/**
 * @brief Test LED strip with a simple pattern
 *
 * @return 0 on success, negative errno on failure
 */
int led_strip_test(void);

#endif /* LED_STRIP_H_ */