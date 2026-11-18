#include "webpanel.hpp"

#include <esp_log.h>
#include <mdns.h>

#include <PsychicHttp.h>

// dashboard/index.html embedded via CMake target_add_binary_data
extern const uint8_t index_html_start[] asm("_binary_index_html_start");
extern const uint8_t index_html_end[] asm("_binary_index_html_end");

namespace panel {
namespace {
const char *TAG = "webpanel";

PsychicHttpServer server;
const Board *s_board = nullptr;

void mdns_start(const char *hostname) {
  ESP_ERROR_CHECK(mdns_init());
  ESP_ERROR_CHECK(mdns_hostname_set(hostname));
  ESP_ERROR_CHECK(mdns_instance_name_set("ESP Dashboard"));
  ESP_ERROR_CHECK(mdns_service_add(nullptr, "_http", "_tcp", 80, nullptr, 0));
  ESP_LOGI(TAG, "mDNS ready: http://%s.local/", hostname);
}

void server_setup() {
  server.on("/", HTTP_GET,
            [](PsychicRequest *request, PsychicResponse *response) {
              const size_t len =
                  static_cast<size_t>(index_html_end - index_html_start);
              response->setCode(200);
              response->setContentType("text/html");
              response->setContent(index_html_start, len);
              return response->send();
            });

  server.on("/api/layout", HTTP_GET,
            [](PsychicRequest *request, PsychicResponse *response) {
              char body[512];
              const size_t n = s_board->serializeLayout(body, sizeof(body));
              if (n == 0 || n >= sizeof(body)) {
                response->setCode(500);
                return response->send();
              }

              response->setCode(200);
              response->setContentType("application/json");
              response->setContent(body);
              return response->send();
            });

  server.on("/api/metrics", HTTP_GET,
            [](PsychicRequest *request, PsychicResponse *response) {
              char body[256];
              const size_t n = s_board->serializeValues(body, sizeof(body));
              if (n == 0 || n >= sizeof(body)) {
                response->setCode(500);
                return response->send();
              }

              response->setCode(200);
              response->setContentType("application/json");
              response->setContent(body);
              return response->send();
            });

  server.begin();
  ESP_LOGI(TAG, "HTTP server listening on port 80");
}
} // namespace

void init(const char *hostname, const Board &board) {
  s_board = &board;
  mdns_start(hostname);
  server_setup();
}
} // namespace panel
