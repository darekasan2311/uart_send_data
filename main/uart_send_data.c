#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/usb_serial_jtag.h"
#include "esp_log.h"

#define BUF_SIZE (1024)
static const char *TAG = "USB_SERIAL_EXAMPLE";

void usb_serial_task(void *arg)
{
    uint8_t *data = (uint8_t *) malloc(BUF_SIZE);
    
    // Configure USB Serial/JTAG
    usb_serial_jtag_driver_config_t usb_serial_config = {
        .rx_buffer_size = BUF_SIZE,
        .tx_buffer_size = BUF_SIZE,
    };
    
    ESP_ERROR_CHECK(usb_serial_jtag_driver_install(&usb_serial_config));
    
    while (1) {
        int len = usb_serial_jtag_read_bytes(data, BUF_SIZE - 1, 100 / portTICK_PERIOD_MS);
        if (len > 0) {
            data[len] = '\0';
            ESP_LOGI(TAG, "Received %d bytes: %s", len, data);
        }
    }
    
    free(data);
}

void app_main(void)
{
    ESP_LOGI(TAG, "USB Serial/JTAG initialized, waiting for data...");
    xTaskCreate(usb_serial_task, "usb_serial_task", 4096, NULL, 10, NULL);
}
