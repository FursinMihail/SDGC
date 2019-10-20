#ifndef WIFI
#define WIFI
#include"includs.h"

void check_client();
void wifi_init_softap();
void wait_for_client();
esp_err_t create_tcp_server();
int get_client_socket();

#endif
