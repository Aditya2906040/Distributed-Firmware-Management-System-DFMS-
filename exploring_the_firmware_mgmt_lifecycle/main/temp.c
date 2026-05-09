#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"

#include "esp_ota_ops.h"
#include "esp_system.h"

#define LED_GPIO GPIO_NUM_2

void app_main(void)
{
    // ------------------------------------------------
    // GET CURRENT RUNNING PARTITION
    // ------------------------------------------------

    const esp_partition_t *running =
        esp_ota_get_running_partition();

    printf("Running partition: %s\n",
           running->label);

    // ------------------------------------------------
    // CHECK THE STATE OF CURRENT BOOT PARTITION
    // ------------------------------------------------
    esp_ota_img_states_t ota_state;

    esp_err_t ota_result =
        esp_ota_get_state_partition(running,
                                    &ota_state);

    if (ota_result == ESP_OK)
    {
        printf("OTA state retrieved successfully\n");

        switch (ota_state)
        {
        case ESP_OTA_IMG_VALID:
            printf("Firmware state: VALID\n");
            break;

        case ESP_OTA_IMG_PENDING_VERIFY:
            printf("Firmware state: PENDING VERIFY\n");
            break;

        case ESP_OTA_IMG_INVALID:
            printf("Firmware state: INVALID\n");
            break;

        case ESP_OTA_IMG_ABORTED:
            printf("Firmware state: ABORTED\n");
            break;

        case ESP_OTA_IMG_UNDEFINED:
            printf("Firmware state: UNDEFINED\n");
            break;

        default:
            printf("Firmware state: UNKNOWN\n");
            break;
        }
    }
    else
    {
        printf("Failed to retrieve OTA state of %s\n", running->label);
    }

    // ------------------------------------------------
    // GET NEXT OTA UPDATE PARTITION
    // ------------------------------------------------

    const esp_partition_t *update_partition =
        esp_ota_get_next_update_partition(NULL);

    printf("Next update partition: %s\n",
           update_partition->label);

    // ------------------------------------------------
    // FACTORY FIRMWARE IDENTIFICATION
    // ------------------------------------------------

    printf("OTA_0 firmware\n");

    // ------------------------------------------------
    // CHANGE NEXT BOOT PARTITION
    // ------------------------------------------------

    esp_err_t boot_result =
        esp_ota_set_boot_partition(update_partition);

    // ------------------------------------------------
    // CHECK THE STATE OF NEXT BOOT PARTITION
    // ------------------------------------------------
  
    printf("Checking the state of %s\n", update_partition->label);
    
    if (boot_result == ESP_OK)
    {
        printf("Boot partition updated successfully\n");

        printf("Restarting in 2 seconds...\n");

        vTaskDelay(pdMS_TO_TICKS(2000));

        esp_restart();
    }
    else
    {
        printf("Failed to update boot partition\n");
    }

    // ------------------------------------------------
    // GPIO CONFIGURATION
    // ------------------------------------------------

    gpio_reset_pin(LED_GPIO);

    gpio_set_direction(LED_GPIO,
                       GPIO_MODE_OUTPUT);

    // ------------------------------------------------
    // MAIN LOOP
    // ------------------------------------------------

    while (1)
    {
        gpio_set_level(LED_GPIO, 1);

        printf("LED ON\n");

        vTaskDelay(pdMS_TO_TICKS(500));

        gpio_set_level(LED_GPIO, 0);

        printf("LED OFF\n");

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
