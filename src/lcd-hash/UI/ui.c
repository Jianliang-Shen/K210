#include "ft6236u.h"
#include "gpiohs.h"
#include "lcd.h"
#include "pin_config.h"
#include "sleep.h"
#include "st7789.h"
#include "sysctl.h"
#include "ui.h"

void draw_page_title(char *str, uint16_t color)
{
    uint8_t str_len = strlen(str);
    lcd_draw_string(LCD_X_MAX / 2 - str_len * 8 / 2, 6, str, color);
}

int draw_button(uint16_t x1, uint16_t y1,
                uint16_t x2, uint16_t y2,
                uint16_t width,
                uint16_t color,
                uint16_t fill_color,
                char *str, uint16_t str_color)
{
    uint32_t data = ((uint32_t)fill_color << 16) | (uint32_t)fill_color;
    uint16_t str_len = strlen(str);

    lcd_set_area(x1 + width, y1 + width, x2 - width, y2 - width);
    tft_fill_data(&data, (x2 - x1 - 2 * width) * (y2 - y1 - 2 * width));

    lcd_draw_rectangle(x1, y1, x2, y2, width, color);

    lcd_draw_string((x1 + x2) / 2 - 8 * str_len / 2, (y1 + y2) / 2 - 5, str, str_color);
}

void draw_start_page()
{
    lcd_clear(BACKGROUND_COLOR);
    draw_page_title("The IoT secure system based on K210", TITLE_COLOR);

    draw_button(BUTTON_1_X1, BUTTON_1_Y1, BUTTON_1_X2, BUTTON_1_Y2,
                BUTTON_BOUNDARY_WIDTH, BUTTON_BOUNDARY_COLOR, BUTTON_NORMAL_COLOR, "1.Connect Server", BUTTON_CHAR_COLOR);
    draw_button(BUTTON_2_X1, BUTTON_2_Y1, BUTTON_2_X2, BUTTON_2_Y2,
                BUTTON_BOUNDARY_WIDTH, BUTTON_BOUNDARY_COLOR, BUTTON_NORMAL_COLOR, "2.Download picture", BUTTON_CHAR_COLOR);
    draw_button(BUTTON_3_X1, BUTTON_3_Y1, BUTTON_3_X2, BUTTON_3_Y2,
                BUTTON_BOUNDARY_WIDTH, BUTTON_BOUNDARY_COLOR, BUTTON_NORMAL_COLOR, "3.Open picture", BUTTON_CHAR_COLOR);
}

void draw_connect_server_page(uint8_t *connect_server)
{
    lcd_clear(BACKGROUND_COLOR);
    draw_page_title("Connct Server", TITLE_COLOR);

    if(connect_server != CONNECT_SERVER_OK)
    {
        //draw txt list blank
        draw_button(8, 24, 234, 156,
                    2, BLACK, WHITE, "", BUTTON_CHAR_COLOR);

        //draw search wifi button
        draw_button(8, 164, 156, 194,
                    2, BUTTON_BOUNDARY_COLOR, BUTTON_NORMAL_COLOR, "Search", BUTTON_CHAR_COLOR);

        draw_button(164, 164, 312, 194,
                    2, BUTTON_BOUNDARY_COLOR, BUTTON_NORMAL_COLOR, "Connect", BUTTON_CHAR_COLOR);

        draw_button(242, 24, 312, 86,
                    2, BUTTON_BOUNDARY_COLOR, BUTTON_NORMAL_COLOR, "up", BUTTON_CHAR_COLOR);

        draw_button(242, 94, 312, 156,
                    2, BUTTON_BOUNDARY_COLOR, BUTTON_NORMAL_COLOR, "down", BUTTON_CHAR_COLOR);

        //draw back button
        draw_button(BACK_BUTTON_X1, BACK_BUTTON_Y1, BACK_BUTTON_X2, BACK_BUTTON_Y2,
                    2, BUTTON_BOUNDARY_COLOR, BUTTON_NORMAL_COLOR, "BACK", BUTTON_CHAR_COLOR);
    } else
    {
        //draw txt list
        //draw search wifi button
        //draw connect button(default green)
        //draw back button
        draw_button(BACK_BUTTON_X1, BACK_BUTTON_Y1, BACK_BUTTON_X2, BACK_BUTTON_Y2,
                    2, BUTTON_BOUNDARY_COLOR, BUTTON_NORMAL_COLOR, "BACK", BUTTON_CHAR_COLOR);
    }
}

void draw_pic_download_page()
{
    lcd_clear(BACKGROUND_COLOR);
    draw_page_title("Download picture", TITLE_COLOR);
    //draw_title
    //draw start button
    //draw check button
    //draw back button
}

void draw_open_pic_page()
{
    lcd_clear(BACKGROUND_COLOR);
    draw_page_title("Open picture", TITLE_COLOR);

    //draw title
    //draw load pic button
    //draw back button
}

void draw_error_page(char *error_info)
{
    lcd_clear(WHITE);

    //draw error info
    lcd_draw_string(LCD_X_MAX / 2 - strlen(error_info) * 8 / 2, 100, error_info, RED);

    //draw back button
    draw_button(BACK_BUTTON_X1, BACK_BUTTON_Y1, BACK_BUTTON_X2, BACK_BUTTON_Y2,
                2, BUTTON_BOUNDARY_COLOR, BUTTON_NORMAL_COLOR, "BACK", BUTTON_CHAR_COLOR);
}