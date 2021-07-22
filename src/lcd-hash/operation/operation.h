#ifndef _OPERATION_H
#define _OPERATION_H

extern uint32_t key_count;
extern uint8_t tmp[153608];
extern uint8_t pic_download;

uint8_t start_page_operation();
uint8_t connect_server_operation();

uint8_t wifi_login_operation();
uint8_t pic_download_operation();
uint8_t open_picture_operation();
uint8_t error_operation();

#endif