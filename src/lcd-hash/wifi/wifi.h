#ifndef _WIFI_H_
#define _WIFI_H_

#include "string.h"

// #define WIFI_NAME jianliang
// #define WIFI_PASSWD 95898063
// #define WIFI_CONNECT "AT+CWJAP_DEF=\" + WIFI_NAME + \",\" + WIFI_PASSWD + \"\n"


void wifi_module_init();
void wifi_send_cmd_and_printf_log(char *cmd);
#endif