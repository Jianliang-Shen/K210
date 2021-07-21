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

#define MAX_WIFI_LOG_SIZE 1000
#define WIFI_TIME_OUT 1
#define MAX_WIFI_NUM 8

void wifi_module_init();
void wifi_send_cmd(char *cmd);
void server_connect(uint8_t *wifi_name,
                    uint8_t *wifi_passwd,
                    uint8_t *server_port,
                    uint8_t *server_ip,
                    uint8_t *wifi_log_clear_flag,
                    uint8_t *wifi_log,
                    uint8_t *connect_server);
void server_disconnect(uint8_t *connect_server);

#endif