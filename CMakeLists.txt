idf_component_register(
    SRCS "src/board.cpp" "src/webpanel.cpp"
    INCLUDE_DIRS "src"
    REQUIRES
        nvs_flash
        esp_event
        esp_netif
        esp_http_server
        espressif__mdns
        bblanchon__arduinojson
)

# Embed static files under dashboard/ into the firmware
target_add_binary_data(${COMPONENT_LIB} "${COMPONENT_DIR}/dashboard/index.html" BINARY)
