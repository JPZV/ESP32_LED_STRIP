/*  ---------------------------------------------------------------------------
    File: led_strip.h
    Author(s):  Lucas Bruder <LBruder@me.com>
    Date Created: 11/23/2016
    Last modified: 11/26/2016

    Description: 
    This library can drive led strips through the RMT module on the ESP32.
    ------------------------------------------------------------------------ */

#ifndef LED_STRIP_H
#define LED_STRIP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <driver/rmt.h>
#include <driver/gpio.h>
#include <string.h>
#include <stddef.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "esp_log.h"

typedef enum effect_type_t {
    RGB = 0,
	TIMED_ON_FADE_OFF,
    COLOR,
	CLEAR,
	TIMED_FADE_IN_OFF,
    BORDER_TO_CENTER,
	CENTER_TO_BORDER,
	NONE,
}effect_type_t;

enum rgb_led_type_t {
    RGB_LED_TYPE_WS2812 = 0,
    RGB_LED_TYPE_SK6812 = 1,
    RGB_LED_TYPE_APA106 = 2,

    RGB_LED_TYPE_MAX,
};


enum rgb_effect_states_t{
    ALL_RED,
    ALL_GREEN,
    ALL_BLUE,
};

/**
 * RGB LED colors
 */
struct led_color_t {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
};

struct led_strip_t {
    enum rgb_led_type_t rgb_led_type;
    uint32_t led_strip_length;
    rmt_channel_t rmt_channel;				    					// RMT peripheral settings
    gpio_num_t gpio; 												// Must be less than GPIO_NUM_33
    bool showing_buf_1;												// Double buffering elements
    struct led_color_t *led_strip_buf_1;
    struct led_color_t *led_strip_buf_2; 

    SemaphoreHandle_t access_semaphore;
};

/* Structures specific for each effect types */

/* Static color effect arguments */
struct effect_static_color_args_t {
    struct led_color_t effect_color;
};

/* RGB effect arguments */
struct effect_rgb_args_t {
	enum rgb_effect_states_t rgb_effect_state;
    uint8_t speed;
};

struct effect_timed_on_fade_off_args_t {
	uint16_t counter;
	uint16_t off_time_ms;
	uint16_t fade_off_speed;
	uint8_t step_counter;
	uint8_t fade_step;
    struct led_color_t effect_color;
    struct led_color_t step_color;
};

/* General structure for effect handler*/
struct led_strip_effect_t {
	struct led_strip_t *led_strip;
	effect_type_t effect_type;
	void *effect_args;
};

extern TaskHandle_t led_strip_effect_task_handle;
extern xQueueHandle effect_queue_handle;


bool led_strip_init(struct led_strip_t *led_strip);

/**
 * Sets the pixel at pixel_num to color.
 */
bool led_strip_set_pixel_color(struct led_strip_t *led_strip, uint32_t pixel_num, struct led_color_t *color);
bool led_strip_set_pixel_rgb(struct led_strip_t *led_strip, uint32_t pixel_num, uint8_t red, uint8_t green, uint8_t blue);
/**
 * Get the pixel color at pixel_num for the led strip that is currently being shown! 
 * NOTE: If you call set_pixel_color then get_pixel_color for the same pixel_num, you will not 
 * get back the same pixel value. This gets you the color of the pixel currently being shown, not the one
 * being updated
 *
 * If there is an invalid argument, color will point to NULL and this function will return false.
 */
bool led_strip_get_pixel_color(struct led_strip_t *led_strip, uint32_t pixel_num, struct led_color_t *color);

/**
 * Updates the led buffer to be shown using double buffering.
 */
bool led_strip_show(struct led_strip_t *led_strip);

/**
 * Clears the LED strip.
 */
bool led_strip_clear(struct led_strip_t *led_strip);

/**
  * @brief     	Initialize task to handle LED strip effects
  *
  * @param 		pointer to led_strip strucutre
  * @param 		effect_type enum for pre-defined effects
  * @param 		arguments of the effect type
  *
  * @return
  *      -ESP_OK 	On success
  *      -ESP_FAIL 	Generic code indicating failure
  *      -ESP_ERR_INVALID_STATE if task (and led strip) has been already initialized
  *
  **/
esp_err_t led_strip_init_effect_handler(struct led_strip_t *led_strip, effect_type_t effect_type, void *effect_arg);


/**
  * @brief     	Initialize task to create pre-defined effects
  *
  * @param 		led_strip_effect pointer to LED effect context
  * @param 		effect_type enum for pre-defined effects
  * @param 		effect_speed overall effect speed (based on visual effect for each pre-defined effect)
  * @param 		red color of effect from 0 to 255
  * @param 		green color of effect from 0 to 255
  * @param 		blue color of effect from 0 to 255
  *
  * @return
  *      -ESP_OK 	On success
  *      -ESP_FAIL 	Generic code indicating failure
  *
  **/
esp_err_t led_strip_set_effect(struct led_strip_t *led_strip, effect_type_t effect_type, void *effect_arg);

#ifdef __cplusplus
}
#endif

#endif // LED_STRIP_H
