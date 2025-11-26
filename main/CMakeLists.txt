idf_component_register(SRCS "PrincessPiTracker.c"
                    INCLUDE_DIRS "."
                    PRIV_REQUIRES nvs_flash bt esp_driver_usb_serial_jtag
)
