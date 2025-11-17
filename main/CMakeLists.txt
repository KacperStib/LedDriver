set(srcs "main.c")

idf_component_register(SRCS ${srcs}
                    INCLUDE_DIRS "."
                    REQUIRES i2c sht40 ina219 ledcDriver pir myNvs webserver esp_netif esp_wifi nvs_flash
                    )
