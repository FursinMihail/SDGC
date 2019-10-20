deps_config := \
	/home/Green_Fox/esp-idf/components/app_trace/Kconfig \
	/home/Green_Fox/esp-idf/components/aws_iot/Kconfig \
	/home/Green_Fox/esp-idf/components/bt/Kconfig \
	/home/Green_Fox/esp-idf/components/esp32/Kconfig \
	/home/Green_Fox/esp-idf/components/ethernet/Kconfig \
	/home/Green_Fox/esp-idf/components/fatfs/Kconfig \
	/home/Green_Fox/esp-idf/components/freertos/Kconfig \
	/home/Green_Fox/esp-idf/components/heap/Kconfig \
	/home/Green_Fox/esp-idf/components/libsodium/Kconfig \
	/home/Green_Fox/esp-idf/components/log/Kconfig \
	/home/Green_Fox/esp-idf/components/lwip/Kconfig \
	/home/Green_Fox/esp-idf/components/mbedtls/Kconfig \
	/home/Green_Fox/esp-idf/components/openssl/Kconfig \
	/home/Green_Fox/esp-idf/components/pthread/Kconfig \
	/home/Green_Fox/esp-idf/components/spi_flash/Kconfig \
	/home/Green_Fox/esp-idf/components/spiffs/Kconfig \
	/home/Green_Fox/esp-idf/components/tcpip_adapter/Kconfig \
	/home/Green_Fox/esp-idf/components/wear_levelling/Kconfig \
	/home/Green_Fox/esp-idf/components/bootloader/Kconfig.projbuild \
	/home/Green_Fox/esp-idf/components/esptool_py/Kconfig.projbuild \
	/home/Green_Fox/esp-idf/components/partition_table/Kconfig.projbuild \
	/home/Green_Fox/esp-idf/Kconfig

include/config/auto.conf: \
	$(deps_config)


$(deps_config): ;
