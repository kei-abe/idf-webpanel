#include <esp_event.h>
#include <esp_log.h>
#include <esp_netif.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <nvs_flash.h>

#include "protocol_examples_common.h"
#include "webpanel.hpp"

static const char *TAG = "idf-webpanel";

panel::Board s_panel = panel::Board{
    "Sample Board",
    {
        {"t_1", "T 1 Temperature", "°C", 1},
        {"h_1", "H 1 Humidity", "%", 1},
        {"v_1", "V 1 Voltage", "V", 1},
        {"i_1", "I 1 Current", "A", 1},
    },
};

static void metrics_task(void *arg) {
  uint32_t tick = 0;
  while (true) {
    tick++;

    s_panel.updateCard("t_1", static_cast<float>(tick % 100));
    s_panel.updateCard("h_1", static_cast<float>(tick % 100));
    s_panel.updateCard("v_1", static_cast<float>(tick % 100));
    s_panel.updateCard("i_1", static_cast<float>(tick % 100));

    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

extern "C" void app_main(void) {
  ESP_ERROR_CHECK(nvs_flash_init());
  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());

  xTaskCreate(metrics_task, "metrics", 3072, nullptr, 5, nullptr);

  if (example_connect() != ESP_OK) {
    ESP_LOGE(TAG,
             "Wi-Fi not connected; HTTP server will not be reachable on LAN");
    return;
  }

  panel::init("sample", s_panel);
}
