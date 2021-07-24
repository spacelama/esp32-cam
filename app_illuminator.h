#ifndef _APP_ILLUMINATOR_H_
#define _APP_ILLUMINATOR_H_

#define CONFIG_LED_ILLUMINATOR_ENABLED 1
#define CONFIG_LED_MAX_INTENSITY 255
#define CONFIG_LED_LEDC_PIN GPIO_NUM_4
#define CONFIG_LED_LEDC_TIMER LEDC_TIMER_3
#define CONFIG_LED_LEDC_CHANNEL LEDC_CHANNEL_6

#ifdef CONFIG_LED_ILLUMINATOR_ENABLED
#ifdef CONFIG_LED_LEDC_LOW_SPEED_MODE
#define CONFIG_LED_LEDC_SPEED_MODE LEDC_LOW_SPEED_MODE
#else
#define CONFIG_LED_LEDC_SPEED_MODE LEDC_HIGH_SPEED_MODE
#endif
    
void app_illuminator_startup();
void app_illuminator_shutdown();
void app_illuminator_set_led_intensity(uint8_t duty);
#endif

#endif
