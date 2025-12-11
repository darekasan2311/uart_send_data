#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/usb_serial_jtag.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_event.h"
#include "driver/gpio.h"

#define BUF_SIZE (1024)
static const char *TAG = "USB_SERIAL";

// GPIO Configuration
#define OUTPUT_PIN         (GPIO_NUM_2)  // Change to your desired pin

static int blink_freq_hz = 0;
static bool led_on = false;

static void configure_gpio(void)
{
    // Configure GPIO as output
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << OUTPUT_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf);
    gpio_set_level(OUTPUT_PIN, 0);  // Start with LOW
    
}

void usb_serial_task(void *arg)
{
    uint8_t *data = (uint8_t *) malloc(BUF_SIZE);
    
    // Configure USB Serial/JTAG
    usb_serial_jtag_driver_config_t usb_serial_config = {
        .rx_buffer_size = BUF_SIZE,
        .tx_buffer_size = BUF_SIZE,
    };
    
    ESP_ERROR_CHECK(usb_serial_jtag_driver_install(&usb_serial_config));
    
    static char buffer[256];
    static int bufIndex = 0;

    while (1) {
        int len = usb_serial_jtag_read_bytes(data, BUF_SIZE - 1, 100 / portTICK_PERIOD_MS);
        if (len > 0) {
            data[len] = '\0';

            // Process each character
            for (int i = 0; i < len; i++) {
                char c = data[i];

                if (c == '\n') {
                    // Complete message received
                    buffer[bufIndex] = '\0';
                    ESP_LOGI(TAG, "Complete message: %s", buffer);

                    // Parse the message
                    char mode[10];
                    int freq;

                    if (sscanf(buffer, "%[^,],%d", mode, &freq) == 2) {
                        ESP_LOGI(TAG, "Parsed: mode=%s, freq=%d", mode, freq);
                        blink_freq_hz = freq;

                        if (strcmp(mode, "ON") == 0) {
                            ESP_LOGI(TAG, "LED ON");
                            led_on = 1;
                        } else if (strcmp(mode, "OFF") == 0) {
                            ESP_LOGI(TAG, "LED OFF");
                            led_on = 0;
                        }
                    } else {
                        ESP_LOGW(TAG, "Parse error");
                    }

                    bufIndex = 0;  // Reset buffer
                } else if (bufIndex < sizeof(buffer) - 1) {
                    buffer[bufIndex++] = c;
                } else {
                    // Buffer overflow - reset
                    ESP_LOGW(TAG, "Buffer overflow - resetting");
                    bufIndex = 0;
                }
            }
        }
    }
    
    free(data);
}

static void blink_task(void *arg)
{
    while (1) {
        if (blink_freq_hz > 0 && led_on) {
            gpio_set_level(OUTPUT_PIN, 1);
            vTaskDelay(pdMS_TO_TICKS(blink_freq_hz));
            gpio_set_level(OUTPUT_PIN, 0);
            vTaskDelay(pdMS_TO_TICKS(blink_freq_hz));
        } else if (led_on) {
            // Solid ON
            gpio_set_level(OUTPUT_PIN, 1);
            vTaskDelay(pdMS_TO_TICKS(100));
        } else {
            // OFF
            gpio_set_level(OUTPUT_PIN, 0);
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "Configuring GPIO");
    configure_gpio();
    ESP_LOGI(TAG, "USB Serial/JTAG initialized, waiting for data...");
    xTaskCreate(usb_serial_task, "usb_serial_task", 4096, NULL, 10, NULL);
    xTaskCreate(blink_task, "blink_task", 2048, NULL, 5, NULL);
}
