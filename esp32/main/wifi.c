#include "wifi.h"

#define SSID "esp32"
#define MAX_STA_CONN 10
#define DEFAULT_PWD "12345678"
#define WIFI_CONNECTED_BIT BIT0

static int server_socket = 0;
static struct sockaddr_in server_addr;
static struct sockaddr_in client_addr;
static unsigned int socklen = sizeof(client_addr);
static int connect_socket = 0;
bool g_rxtx_need_restart = false;

EventGroupHandle_t tcp_event_group;

static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    switch (event->event_id) {
    case SYSTEM_EVENT_STA_START:
        esp_wifi_connect();
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        esp_wifi_connect();
        xEventGroupClearBits(tcp_event_group, WIFI_CONNECTED_BIT);
        break;
    case SYSTEM_EVENT_STA_CONNECTED:
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        xEventGroupSetBits(tcp_event_group, WIFI_CONNECTED_BIT);
        break;
    case SYSTEM_EVENT_AP_STACONNECTED:
        ESP_LOGI("test", "station:"MACSTR" join,AID=%d\n",
                 MAC2STR(event->event_info.sta_connected.mac),
                 event->event_info.sta_connected.aid);
        xEventGroupSetBits(tcp_event_group, WIFI_CONNECTED_BIT);
        break;
    case SYSTEM_EVENT_AP_STADISCONNECTED:
        ESP_LOGI("test", "station:"MACSTR"leave,AID=%d\n",
                 MAC2STR(event->event_info.sta_disconnected.mac),
                 event->event_info.sta_disconnected.aid);
        xEventGroupClearBits(tcp_event_group, WIFI_CONNECTED_BIT);
        break;
    default:
        break;
    }
    return ESP_OK;
}

void wifi_init_softap()
{
    tcp_event_group = xEventGroupCreate();

    tcpip_adapter_init();
    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    wifi_config_t wifi_config = {
        .ap = {
            .ssid = SSID,
            .ssid_len = 0,
            .max_connection = MAX_STA_CONN,
            .password = DEFAULT_PWD,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK
        },
    };
    if (strlen(DEFAULT_PWD) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI("test", "wifi_init_softap finished.SSID:%s password:%s \n",
             SSID, DEFAULT_PWD);
}

esp_err_t create_tcp_server()
{
    ESP_LOGI("test", "server socket....port=%d\n", 80);
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {

        return ESP_FAIL;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(80);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {

        close(server_socket);
        return ESP_FAIL;
    }
    if (listen(server_socket, 5) < 0) {

        close(server_socket);
        return ESP_FAIL;
    }
    //connect_socket = accept(server_socket, (struct sockaddr *)&client_addr, &socklen);
    //if (connect_socket < 0) {

      //  close(server_socket);
      //  return ESP_FAIL;
    //}
    /*connection established，now can send/recv*/
    //ESP_LOGI("test", "tcp connection established!");
    return ESP_OK;
}

void check_client(){
	while(1){
		static int tmp_socket = 0; //connect_socket
		tmp_socket = accept(server_socket, (struct sockaddr *)&client_addr, &socklen);
		if (connect_socket >= 0) {
			//vTaskDelete(tcp_task_hd);
			closesocket(connect_socket);
			connect_socket = tmp_socket;
			ESP_LOGI("test", "tcp connection established!");
		}
	}
}

void wait_for_client(){
	xEventGroupWaitBits(tcp_event_group, WIFI_CONNECTED_BIT, false, true, portMAX_DELAY);
}

int get_client_socket(){
	return connect_socket;
}
