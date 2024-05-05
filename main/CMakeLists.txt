idf_component_register(
	SRCS
		"main.c"
		"wifi.c"
	INCLUDE_DIRS "."
	PRIV_REQUIRES
		esp_netif
		esp_wifi
		mqtt
		nvs_flash
)
