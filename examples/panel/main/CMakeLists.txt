idf_component_register(
    SRCS "main.cpp"
    INCLUDE_DIRS "."
    REQUIRES
        idf-webpanel
        nvs_flash
        esp_event
        esp_netif
        protocol_examples_common
)
