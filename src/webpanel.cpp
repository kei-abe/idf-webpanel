#include "webpanel.hpp"

#include <esp_http_server.h>
#include <esp_log.h>
#include <mdns.h>

// dashboard/index.html embedded via CMake target_add_binary_data
extern const uint8_t index_html_start[] asm("_binary_index_html_start");
extern const uint8_t index_html_end[] asm("_binary_index_html_end");

namespace panel {
namespace {
const char *TAG = "webpanel";

httpd_handle_t s_server = nullptr;
const Board *s_board = nullptr;

void mdns_start(const char *hostname) {
  ESP_ERROR_CHECK(mdns_init());
  ESP_ERROR_CHECK(mdns_hostname_set(hostname));
  ESP_ERROR_CHECK(mdns_instance_name_set("ESP Dashboard"));
  ESP_ERROR_CHECK(mdns_service_add(nullptr, "_http", "_tcp", 80, nullptr, 0));
  ESP_LOGI(TAG, "mDNS ready: http://%s.local/", hostname);
}

esp_err_t handle_index(httpd_req_t *req) {
  const size_t len = static_cast<size_t>(index_html_end - index_html_start);
  httpd_resp_set_type(req, "text/html");
  return httpd_resp_send(req, reinterpret_cast<const char *>(index_html_start),
                         len);
}

esp_err_t handle_layout(httpd_req_t *req) {
  char body[512];
  const size_t n = s_board->serializeLayout(body, sizeof(body));
  if (n == 0 || n >= sizeof(body)) {
    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
                        "layout serialize failed");
    return ESP_FAIL;
  }

  httpd_resp_set_type(req, "application/json");
  return httpd_resp_send(req, body, n);
}

esp_err_t handle_metrics(httpd_req_t *req) {
  char body[256];
  const size_t n = s_board->serializeValues(body, sizeof(body));
  if (n == 0 || n >= sizeof(body)) {
    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
                        "metrics serialize failed");
    return ESP_FAIL;
  }

  httpd_resp_set_type(req, "application/json");
  return httpd_resp_send(req, body, n);
}

void server_setup() {
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.server_port = 80;
  config.lru_purge_enable = true;

  ESP_ERROR_CHECK(httpd_start(&s_server, &config));

  httpd_uri_t uri_index = {};
  uri_index.uri = "/";
  uri_index.method = HTTP_GET;
  uri_index.handler = handle_index;

  httpd_uri_t uri_layout = {};
  uri_layout.uri = "/api/layout";
  uri_layout.method = HTTP_GET;
  uri_layout.handler = handle_layout;

  httpd_uri_t uri_metrics = {};
  uri_metrics.uri = "/api/metrics";
  uri_metrics.method = HTTP_GET;
  uri_metrics.handler = handle_metrics;

  ESP_ERROR_CHECK(httpd_register_uri_handler(s_server, &uri_index));
  ESP_ERROR_CHECK(httpd_register_uri_handler(s_server, &uri_layout));
  ESP_ERROR_CHECK(httpd_register_uri_handler(s_server, &uri_metrics));

  ESP_LOGI(TAG, "HTTP server listening on port 80");
}
} // namespace

void init(const char *hostname, const Board &board) {
  s_board = &board;
  mdns_start(hostname);
  server_setup();
}
} // namespace panel
