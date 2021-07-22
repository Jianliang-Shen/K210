#ifndef _WIFI_H_
#define _WIFI_H_

#include <stdio.h>
#include "bsp.h"
#include "ft6236u.h"
#include "gpiohs.h"
#include "lcd.h"
#include "pin_config.h"
#include "sleep.h"
#include "st7789.h"
#include "string.h"
#include "sysctl.h"
#include "ui.h"

#define MAX_WIFI_LOG_SIZE 2048
#define WIFI_TIME_OUT 1
#define MAX_WIFI_NUM 8

extern uint8_t wifi_searched;
extern uint8_t wifi_num;
extern uint8_t wifi_name[16];

extern uint8_t connect_server;

// uint8_t passwd[20] = {'\0'};
// uint8_t passwd_idx = 0;

// uint8_t server_ip[15] = {'\0'};
// uint8_t server_ip_idx = 0;

// uint8_t server_port[5] = {'\0'};
// uint8_t server_port_idx = 0;

extern uint8_t passwd[20];
extern uint8_t passwd_idx;

extern uint8_t server_ip[16];
extern uint8_t server_ip_idx;

extern uint8_t server_port[6];
extern uint8_t server_port_idx;

extern uint8_t wifi_login_enter_flag;

extern uint8_t wifi_log[MAX_WIFI_LOG_SIZE];
extern uint8_t wifi_log_clear_flag;

void wifi_module_init();
void wifi_send_cmd(char *cmd);
void server_connect();
void server_disconnect();

#endif