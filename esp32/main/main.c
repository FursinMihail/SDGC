#include"includs.h"
#define BUFF_SIZE 16000

QueueHandle_t UART_TX, TCP_TX;

void tcp_task(){
	int len = 0;
	char *databuff = (char *)malloc(BUFF_SIZE * sizeof(char));
	while (1) {
		len = recv(get_client_socket(), databuff, BUFF_SIZE, 20 / portTICK_PERIOD_MS);
	    if (len > 0) {
	    	for(int i = 0; i < len; i++){
	    		xQueueSendToBackFromISR(UART_TX, &databuff[i], NULL);
	    	}
	    	//ESP_LOGI("test", "tcp get data!");
	    }
	}
}

void uart_task(){
	ESP_LOGI("test", "uart task start!");

	//ESP_LOGI("test", "uart init!");
	int len = 0;
	uint8_t *databuff = (uint8_t *)malloc(BUFF_SIZE);
	while (1) {
		//ESP_LOGI("test", "uart read!");
		len = uart_read_bytes(UART_NUM_1, databuff, BUFF_SIZE, 20 / portTICK_PERIOD_MS);
		//send(get_client_socket(), databuff, len, 0);

		if (len > 0) {
		   	for(int i = 0; i < len; i++){
		   		xQueueSendToBackFromISR(TCP_TX, &databuff[i], NULL);
		   	}
		}

	    /*if(!xQueueIsQueueEmptyFromISR(TCP_TX)){
	    	char tmp;
	    	xQueueReceiveFromISR(TCP_TX,&tmp,NULL);
	    	uart_write_bytes(UART_NUM_1, &tmp, 1);
	    	//ESP_LOGI("test", "send to uart!");
	    }*/
	}
}

void app_main()
{
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES) {
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK( ret );

	wifi_init_softap();

	UART_TX = xQueueCreate(BUFF_SIZE, sizeof(char));
	TCP_TX = xQueueCreate(BUFF_SIZE, sizeof(char));


		ESP_LOGI("test", "BEGIN!!!");
		int socket_ret = ESP_FAIL;

		if (socket_ret == ESP_FAIL) {
			vTaskDelay(300 / portTICK_RATE_MS);
			socket_ret = create_tcp_server();
		}

		if (socket_ret == ESP_FAIL) {
			ESP_LOGI("test", "create tcp socket error,stop.");
			return;
		}

		uart_config_t uart_config = {
					.baud_rate = 115200,
			        .data_bits = UART_DATA_8_BITS,
			        .parity    = UART_PARITY_DISABLE,
			        .stop_bits = UART_STOP_BITS_1,
			        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
			};
		uart_param_config(UART_NUM_1, &uart_config);
		uart_set_pin(UART_NUM_1, 22, 23, UART_PIN_NO_CHANGE , UART_PIN_NO_CHANGE );
		uart_driver_install(UART_NUM_1, BUFF_SIZE * 2, BUFF_SIZE * 2, 0, NULL, 0);

		xTaskCreate(uart_task, "uart_task", 4096, NULL, 10, NULL);
		xTaskCreate(check_client, "check_client", 4096, NULL, 10, NULL);
		xTaskCreate(tcp_task, "tcp_task", 4096, NULL, 10, NULL);
		//send(get_client_socket(), databuff, len, 0);
		while(1){//socket_ret != ESP_FAIL
			if(!xQueueIsQueueEmptyFromISR(UART_TX)){
			    	char tmp;
			    	xQueueReceiveFromISR(UART_TX,&tmp,NULL);
			    	uart_write_bytes(UART_NUM_1, &tmp, 1);
			    	//ESP_LOGI("test", "send to uart!");
			}
			if(!xQueueIsQueueEmptyFromISR(TCP_TX)){
			    	char tmp;
			    	xQueueReceiveFromISR(TCP_TX,&tmp,NULL);
			    	send(get_client_socket(), &tmp, 1, 0);
			    	//ESP_LOGI("test", "send to uart!");
			}
		}
		//wait_for_client();
		//esp_restart();

}

