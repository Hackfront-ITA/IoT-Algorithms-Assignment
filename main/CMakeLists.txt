idf_component_register(
	SRCS
		"adapt.c"
		"adc.c"
		"fft.c"
		"main.c"
		"utils.c"
		"mqtt.c"
		"wifi.c"
	INCLUDE_DIRS
		"."
	PRIV_REQUIRES
		esp_adc
		esp-dsp
		esp_netif
		esp_wifi
		mqtt
		nvs_flash
)
