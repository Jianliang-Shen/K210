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

#define MAX_WIFI_LOG_SIZE 500
#define WIFI_TIME_OUT 1

void wifi_module_init();
void wifi_send_cmd(char *cmd);
#endif