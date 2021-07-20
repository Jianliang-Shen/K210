#ifndef _UI_H_
#define _UI_H_
#include "lcd.h"

#define START_PAGE 0
#define CONNECT_SERVER_PAGE 1

#define BUTTON_HEIGHT 60
#define BUTTON_BOUNDARY_WIDTH 4
#define BUTTON_HEIGHT_DISTANCE 10

#define BUTTON_1_X1 35
#define BUTTON_1_Y1 30
#define BUTTON_1_X2 (LCD_X_MAX - BUTTON_1_X1)
#define BUTTON_1_Y2 (BUTTON_1_Y1 + BUTTON_HEIGHT)

#define BUTTON_2_X1 35
#define BUTTON_2_Y1 100
#define BUTTON_2_X2 (LCD_X_MAX - BUTTON_2_X1)
#define BUTTON_2_Y2 (BUTTON_2_Y1 + BUTTON_HEIGHT)

#define BUTTON_3_X1 35
#define BUTTON_3_Y1 170
#define BUTTON_3_X2 (LCD_X_MAX - BUTTON_3_X1)
#define BUTTON_3_Y2 (BUTTON_3_Y1 + BUTTON_HEIGHT)

#define BACK_BUTTON_X1 8
#define BACK_BUTTON_Y1 202
#define BACK_BUTTON_X2 312
#define BACK_BUTTON_Y2 232

#define BUTTON_BOUNDARY_COLOR DARKGREY
#define BUTTON_NORMAL_COLOR LIGHTGREY
#define BUTTON_TRIGGER_COLOR WHITE
#define BUTTON_UNUSE_COLOR LIGHTGREY
#define BUTTON_CHAR_COLOR BLACK
#define BACKGROUND_COLOR WHITE
#define TITLE_COLOR BLACK

#define CONNECT_SERVER_OK 1
#define PIC_DONWLOAD_OK 1

#define START_PAGE 0
#define CONNECT_SERVER_PAGE 1
#define PIC_DOWNLOAD_PAGE 2
#define OPEN_PICTURE_PAGE 3
#define WIFI_LOGIN_PAGE 4

#define ERROR_PAGE 255
void draw_wifi_list(uint8_t *wifi_searched,
                    uint8_t wifi_updated,
                    uint8_t *info,
                    uint8_t choose_idx,
                    uint8_t *wifi_num,
                    uint8_t *wifi_name);
int draw_button(uint16_t x1, uint16_t y1,
                uint16_t x2, uint16_t y2,
                uint16_t width,
                uint16_t color,
                uint16_t fill_color,
                char *str, uint16_t str_color);
void draw_start_page();
void draw_connect_server_page();
void draw_wifi_login_key_page1(uint8_t curr_row,
                               uint8_t curr_column,
                               int8_t *value,
                               uint8_t page_status);
void draw_wifi_login_key_page2(uint8_t curr_row,
                               uint8_t curr_column,
                               int8_t *value,
                               uint8_t page_status);
void draw_wifi_login_page(uint8_t *wifi_connected,
                          uint8_t *wifi_name,
                          uint8_t page_num);
void draw_pic_download_page();
void draw_open_pic_page();
void draw_error_page(char *error_info);
#endif