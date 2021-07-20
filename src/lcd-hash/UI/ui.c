#include "ft6236u.h"
#include "gpiohs.h"
#include "lcd.h"
#include "pin_config.h"
#include "sleep.h"
#include "st7789.h"
#include "sysctl.h"
#include "ui.h"
#include "wifi.h"

void draw_wifi_list(uint8_t *wifi_searched,
                    uint8_t wifi_updated,
                    uint8_t *info,
                    uint8_t choose_idx,
                    uint8_t *wifi_num,
                    uint8_t *wifi_name)
{
    static uint8_t str[8][20];

    uint16_t y = 28, x = 14;

    //column 从0开始计算
    uint16_t column = -1, idx = 0;
    uint16_t length = strlen(info);

    static uint8_t now_grey_idx;
    static uint8_t drawed_flag = 0;

    if(wifi_updated)
    {
        //清空区域
        draw_button(8, 24, 234, 156,
                    2, BLACK, WHITE, "", BUTTON_CHAR_COLOR);
        memset(str, '\0', sizeof(str));
        drawed_flag = 0;

        for(uint16_t i = 0; i < length; i++)
        {
            if(info[i] == '(')
            {
                column++;
                if(column > MAX_WIFI_NUM - 1)
                {
                    column = MAX_WIFI_NUM - 1;
                    break;
                }
                for(uint16_t j = i + 4; j < length; j++)
                {
                    if(info[j] == '"')
                    {
                        break;
                    } else
                    {
                        str[column][j - i - 4] = info[j];
                    }
                }
            }
        }
        if(column >= 0)
        {
            *wifi_num = column;
            *wifi_searched = 1;
        } else
        {
            printf("No wifi searched!\n");
        }
    }

    // 如果搜索到wifi
    if(*wifi_searched == 1)
    {
        uint8_t tmp_idx = now_grey_idx;
        for(uint8_t i = 0; i <= *wifi_num; i++)
        {
            if(drawed_flag)
            {
                if(i == choose_idx)
                {
                    uint32_t data = ((uint32_t)LIGHTGREY << 16) | (uint32_t)GREEN;
                    lcd_set_area(10, y, 232, y + 14);
                    tft_fill_data(&data, 222 * 14);
                    lcd_draw_string(x, y, str[i], BLACK);

                    now_grey_idx = i;
                } else if(i == tmp_idx)
                {
                    uint32_t data = ((uint32_t)WHITE << 16) | (uint32_t)WHITE;
                    lcd_set_area(10, y, 232, y + 14);
                    tft_fill_data(&data, 222 * 14);
                    lcd_draw_string(x, y, str[i], BLACK);
                }
            } else
            {
                if(i == choose_idx)
                {
                    uint32_t data = ((uint32_t)LIGHTGREY << 16) | (uint32_t)GREEN;
                    lcd_set_area(10, y, 232, y + 14);
                    tft_fill_data(&data, 222 * 14);

                    now_grey_idx = i;
                }
                lcd_draw_string(x, y, str[i], BLACK);
            }

            y += 16;
        }
        drawed_flag = 1;
    }
    memcpy(wifi_name, str[choose_idx], 16); //获取wifi name
}

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

    lcd_draw_string((x1 + x2) / 2 - 8 * str_len / 2, (y1 + y2) / 2 - 7, str, str_color);
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
                    2, BUTTON_BOUNDARY_COLOR, BUTTON_NORMAL_COLOR, "Search Wifi", BUTTON_CHAR_COLOR);

        //绘制连接按钮，默认无效
        draw_button(164, 164, 312, 194,
                    2, LIGHTGREY, DARKGREY, "Connect", BUTTON_CHAR_COLOR);

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

void draw_wifi_login_key_page1(uint8_t curr_row, uint8_t curr_column, int8_t *value, uint8_t page_status)
{
    /*
        =========================
              Enter passwd
                 xxxx
        =========================
        1 2 3 4 5 6 7 8 9 0 - = <==
        q w e r t y u i o p [ ] \
          a s d f g h j k l ; ' Enter
           z x c v b n m , . / Shift
                blank    back
        =========================
    */
    static uint8_t old_row = 0;
    static uint8_t old_column = 0;
    uint8_t line[5][15][5] = {{"1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "-", "=", "<=="},
                              {"q", "w", "e", "r", "t", "y", "u", "i", "o", "p", "[", "]", "\\"},
                              {"a", "s", "d", "f", "g", "h", "j", "k", "l", ";", "\'", "Enter"},
                              {"z", "x", "c", "v", "b", "n", "m", ",", ".", "/", "Shift"},
                              {" ", "<<"}};
    int8_t value_array[5][15] = {{'1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', -1},
                                 {'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\\'},
                                 {'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', -2},
                                 {'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', -3},
                                 {' ', -4}};
    *value = value_array[curr_row][curr_column];

    if(page_status == 0)
    {
        for(int j = 0; j < 5; j++)
        {
            if(j == 0)
            {
                for(int i = 0; i < 12; i++)
                {
                    draw_button(5 + i * 20, 102, 5 + i * 20 + 18, 127, 1,
                                BUTTON_BOUNDARY_COLOR, ((j == curr_row) && (i == curr_column)) ? GREEN : WHITE,
                                line[j][i], BUTTON_CHAR_COLOR);
                }
                draw_button(245, 102, 315, 127, 1,
                            BUTTON_BOUNDARY_COLOR, ((j == curr_row) && (12 == curr_column)) ? GREEN : WHITE,
                            line[j][12], BUTTON_CHAR_COLOR);
            }
            if(j == 1)
            {
                for(int i = 0; i < 12; i++)
                {
                    draw_button(15 + i * 20, 129, 15 + i * 20 + 18, 154, 1,
                                BUTTON_BOUNDARY_COLOR, ((j == curr_row) && (i == curr_column)) ? GREEN : WHITE,
                                line[j][i], BUTTON_CHAR_COLOR);
                }
                draw_button(255, 129, 305, 154, 1,
                            BUTTON_BOUNDARY_COLOR, ((j == curr_row) && (12 == curr_column)) ? GREEN : WHITE,
                            line[j][12], BUTTON_CHAR_COLOR);
            }
            if(j == 2)
            {
                for(int i = 0; i < 11; i++)
                {
                    draw_button(25 + i * 20, 156, 25 + i * 20 + 18, 181, 1,
                                BUTTON_BOUNDARY_COLOR, ((j == curr_row) && (i == curr_column)) ? GREEN : WHITE,
                                line[j][i], BUTTON_CHAR_COLOR);
                }
                draw_button(245, 156, 295, 181, 1,
                            BUTTON_BOUNDARY_COLOR, ((j == curr_row) && (11 == curr_column)) ? GREEN : WHITE,
                            line[j][11], BUTTON_CHAR_COLOR);
            }
            if(j == 3)
            {
                for(int i = 0; i < 10; i++)
                {
                    draw_button(35 + i * 20, 183, 35 + i * 20 + 18, 208, 1,
                                BUTTON_BOUNDARY_COLOR, ((j == curr_row) && (i == curr_column)) ? GREEN : WHITE,
                                line[j][i], BUTTON_CHAR_COLOR);
                }
                draw_button(235, 183, 285, 208, 1,
                            BUTTON_BOUNDARY_COLOR, ((j == curr_row) && (10 == curr_column)) ? GREEN : WHITE,
                            line[j][10], BUTTON_CHAR_COLOR);
            }
            if(j == 4)
            {
                draw_button(45, 210, 213, 235, 1,
                            BUTTON_BOUNDARY_COLOR, ((j == curr_row) && (0 == curr_column)) ? GREEN : WHITE,
                            line[j][0], BUTTON_CHAR_COLOR);

                draw_button(215, 210, 275, 235, 1,
                            BUTTON_BOUNDARY_COLOR, ((j == curr_row) && (1 == curr_column)) ? GREEN : WHITE,
                            line[j][1], BUTTON_CHAR_COLOR);
            }
        }
    }

    if(page_status == 1)
    {
        for(int j = 0; j < 5; j++)
        {
            if(j == 0)
            {
                for(int i = 0; i < 12; i++)
                {
                    if(j == old_row && i == old_column)
                    {
                        draw_button(5 + i * 20, 102, 5 + i * 20 + 18, 127, 1, BUTTON_BOUNDARY_COLOR, WHITE, line[j][i], BUTTON_CHAR_COLOR);
                    }
                    if(j == curr_row && i == curr_column)
                    {
                        draw_button(5 + i * 20, 102, 5 + i * 20 + 18, 127, 1, BUTTON_BOUNDARY_COLOR, GREEN, line[j][i], BUTTON_CHAR_COLOR);
                    }
                }
                if(j == old_row && 12 == old_column)
                {
                    draw_button(245, 102, 315, 127, 1, BUTTON_BOUNDARY_COLOR, WHITE, line[j][12], BUTTON_CHAR_COLOR);
                }
                if(j == curr_row && 12 == curr_column)
                {
                    draw_button(245, 102, 315, 127, 1, BUTTON_BOUNDARY_COLOR, GREEN, line[j][12], BUTTON_CHAR_COLOR);
                }
            }
            if(j == 1)
            {
                for(int i = 0; i < 12; i++)
                {

                    if(j == old_row && i == old_column)
                    {
                        draw_button(15 + i * 20, 129, 15 + i * 20 + 18, 154, 1, BUTTON_BOUNDARY_COLOR, WHITE, line[j][i], BUTTON_CHAR_COLOR);
                    }
                    if(j == curr_row && i == curr_column)
                    {
                        draw_button(15 + i * 20, 129, 15 + i * 20 + 18, 154, 1, BUTTON_BOUNDARY_COLOR, GREEN, line[j][i], BUTTON_CHAR_COLOR);
                    }
                }

                if(j == old_row && 12 == old_column)
                {
                    draw_button(255, 129, 305, 154, 1, BUTTON_BOUNDARY_COLOR, WHITE, line[j][12], BUTTON_CHAR_COLOR);
                }
                if(j == curr_row && 12 == curr_column)
                {
                    draw_button(255, 129, 305, 154, 1, BUTTON_BOUNDARY_COLOR, GREEN, line[j][12], BUTTON_CHAR_COLOR);
                }
            }

            if(j == 2)
            {
                for(int i = 0; i < 11; i++)
                {

                    if(j == old_row && i == old_column)
                    {
                        draw_button(25 + i * 20, 156, 25 + i * 20 + 18, 181, 1, BUTTON_BOUNDARY_COLOR, WHITE, line[j][i], BUTTON_CHAR_COLOR);
                    }
                    if(j == curr_row && i == curr_column)
                    {
                        draw_button(25 + i * 20, 156, 25 + i * 20 + 18, 181, 1, BUTTON_BOUNDARY_COLOR, GREEN, line[j][i], BUTTON_CHAR_COLOR);
                    }
                }

                if(j == old_row && 11 == old_column)
                {
                    draw_button(245, 156, 295, 181, 1, BUTTON_BOUNDARY_COLOR, WHITE, line[j][11], BUTTON_CHAR_COLOR);
                }
                if(j == curr_row && 11 == curr_column)
                {
                    draw_button(245, 156, 295, 181, 1, BUTTON_BOUNDARY_COLOR, GREEN, line[j][11], BUTTON_CHAR_COLOR);
                }
            }
            if(j == 3)
            {
                for(int i = 0; i < 10; i++)
                {

                    if(j == old_row && i == old_column)
                    {
                        draw_button(35 + i * 20, 183, 35 + i * 20 + 18, 208, 1, BUTTON_BOUNDARY_COLOR, WHITE, line[j][i], BUTTON_CHAR_COLOR);
                    }
                    if(j == curr_row && i == curr_column)
                    {
                        draw_button(35 + i * 20, 183, 35 + i * 20 + 18, 208, 1, BUTTON_BOUNDARY_COLOR, GREEN, line[j][i], BUTTON_CHAR_COLOR);
                    }
                }

                if(j == old_row && 10 == old_column)
                {
                    draw_button(235, 183, 285, 208, 1, BUTTON_BOUNDARY_COLOR, WHITE, line[j][10], BUTTON_CHAR_COLOR);
                }
                if(j == curr_row && 10 == curr_column)
                {
                    draw_button(235, 183, 285, 208, 1, BUTTON_BOUNDARY_COLOR, GREEN, line[j][10], BUTTON_CHAR_COLOR);
                }
            }
            if(j == 4)
            {

                if(j == old_row && 0 == old_column)
                {
                    draw_button(45, 210, 213, 235, 1, BUTTON_BOUNDARY_COLOR, WHITE, line[j][0], BUTTON_CHAR_COLOR);
                }
                if(j == curr_row && 0 == curr_column)
                {
                    draw_button(45, 210, 213, 235, 1, BUTTON_BOUNDARY_COLOR, GREEN, line[j][0], BUTTON_CHAR_COLOR);
                }
                if(j == old_row && 1 == old_column)
                {
                    draw_button(215, 210, 275, 235, 1, BUTTON_BOUNDARY_COLOR, WHITE, line[j][1], BUTTON_CHAR_COLOR);
                }
                if(j == curr_row && 1 == curr_column)
                {
                    draw_button(215, 210, 275, 235, 1, BUTTON_BOUNDARY_COLOR, GREEN, line[j][1], BUTTON_CHAR_COLOR);
                }
            }
        }

        old_row = curr_row;
        old_column = curr_column;
    }
}

void draw_wifi_login_key_page2(uint8_t curr_row, uint8_t curr_column, int8_t *value, uint8_t page_status)
{
    /*
        =========================
              Enter passwd
                 xxxx
        =========================
        ! @ # 4 5 ^ & * ( ) _ + <==
        Q W E R T Y U I O P { } |
          A S D F G H J K L ： “ Enter
           Z X C V B N M < > ？ Shift
                blank    back
        =========================
    */
    static uint8_t old_row = 0;
    static uint8_t old_column = 0;
    uint8_t line[5][15][5] = {{"!", "@", "#", "$", "%", "^", "&", "*", "(", ")", "_", "+", "<=="},
                              {"Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "{", "}", "\\"},
                              {"A", "S", "D", "F", "G", "H", "J", "K", "L", ":", "\"", "Enter"},
                              {"Z", "X", "C", "V", "B", "N", "M", "<", ">", "?", "Shift"},
                              {" ", "<<"}};
    int8_t value_array[5][15] = {{'!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', -1},
                                 {'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\\'},
                                 {'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '\"', -2},
                                 {'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', -3},
                                 {' ', -4}};
    *value = value_array[curr_row][curr_column];


    if(page_status == 0)
    {
        for(int j = 0; j < 5; j++)
        {
            if(j == 0)
            {
                for(int i = 0; i < 12; i++)
                {
                    draw_button(5 + i * 20, 102, 5 + i * 20 + 18, 127, 1,
                                BUTTON_BOUNDARY_COLOR, ((j == curr_row) && (i == curr_column)) ? GREEN : WHITE,
                                line[j][i], BUTTON_CHAR_COLOR);
                }
                draw_button(245, 102, 315, 127, 1,
                            BUTTON_BOUNDARY_COLOR, ((j == curr_row) && (12 == curr_column)) ? GREEN : WHITE,
                            line[j][12], BUTTON_CHAR_COLOR);
            }
            if(j == 1)
            {
                for(int i = 0; i < 12; i++)
                {
                    draw_button(15 + i * 20, 129, 15 + i * 20 + 18, 154, 1,
                                BUTTON_BOUNDARY_COLOR, ((j == curr_row) && (i == curr_column)) ? GREEN : WHITE,
                                line[j][i], BUTTON_CHAR_COLOR);
                }
                draw_button(255, 129, 305, 154, 1,
                            BUTTON_BOUNDARY_COLOR, ((j == curr_row) && (12 == curr_column)) ? GREEN : WHITE,
                            line[j][12], BUTTON_CHAR_COLOR);
            }
            if(j == 2)
            {
                for(int i = 0; i < 11; i++)
                {
                    draw_button(25 + i * 20, 156, 25 + i * 20 + 18, 181, 1,
                                BUTTON_BOUNDARY_COLOR, ((j == curr_row) && (i == curr_column)) ? GREEN : WHITE,
                                line[j][i], BUTTON_CHAR_COLOR);
                }
                draw_button(245, 156, 295, 181, 1,
                            BUTTON_BOUNDARY_COLOR, ((j == curr_row) && (11 == curr_column)) ? GREEN : WHITE,
                            line[j][11], BUTTON_CHAR_COLOR);
            }
            if(j == 3)
            {
                for(int i = 0; i < 10; i++)
                {
                    draw_button(35 + i * 20, 183, 35 + i * 20 + 18, 208, 1,
                                BUTTON_BOUNDARY_COLOR, ((j == curr_row) && (i == curr_column)) ? GREEN : WHITE,
                                line[j][i], BUTTON_CHAR_COLOR);
                }
                draw_button(235, 183, 285, 208, 1,
                            BUTTON_BOUNDARY_COLOR, ((j == curr_row) && (10 == curr_column)) ? GREEN : WHITE,
                            line[j][10], BUTTON_CHAR_COLOR);
            }
            if(j == 4)
            {
                draw_button(45, 210, 213, 235, 1,
                            BUTTON_BOUNDARY_COLOR, ((j == curr_row) && (0 == curr_column)) ? GREEN : WHITE,
                            line[j][0], BUTTON_CHAR_COLOR);

                draw_button(215, 210, 275, 235, 1,
                            BUTTON_BOUNDARY_COLOR, ((j == curr_row) && (1 == curr_column)) ? GREEN : WHITE,
                            line[j][1], BUTTON_CHAR_COLOR);
            }
        }
    }

    if(page_status == 1)
    {
        for(int j = 0; j < 5; j++)
        {
            if(j == 0)
            {
                for(int i = 0; i < 12; i++)
                {
                    if(j == old_row && i == old_column)
                    {
                        draw_button(5 + i * 20, 102, 5 + i * 20 + 18, 127, 1, BUTTON_BOUNDARY_COLOR, WHITE, line[j][i], BUTTON_CHAR_COLOR);
                    }
                    if(j == curr_row && i == curr_column)
                    {
                        draw_button(5 + i * 20, 102, 5 + i * 20 + 18, 127, 1, BUTTON_BOUNDARY_COLOR, GREEN, line[j][i], BUTTON_CHAR_COLOR);
                    }
                }
                if(j == old_row && 12 == old_column)
                {
                    draw_button(245, 102, 315, 127, 1, BUTTON_BOUNDARY_COLOR, WHITE, line[j][12], BUTTON_CHAR_COLOR);
                }
                if(j == curr_row && 12 == curr_column)
                {
                    draw_button(245, 102, 315, 127, 1, BUTTON_BOUNDARY_COLOR, GREEN, line[j][12], BUTTON_CHAR_COLOR);
                }
            }
            if(j == 1)
            {
                for(int i = 0; i < 12; i++)
                {

                    if(j == old_row && i == old_column)
                    {
                        draw_button(15 + i * 20, 129, 15 + i * 20 + 18, 154, 1, BUTTON_BOUNDARY_COLOR, WHITE, line[j][i], BUTTON_CHAR_COLOR);
                    }
                    if(j == curr_row && i == curr_column)
                    {
                        draw_button(15 + i * 20, 129, 15 + i * 20 + 18, 154, 1, BUTTON_BOUNDARY_COLOR, GREEN, line[j][i], BUTTON_CHAR_COLOR);
                    }
                }

                if(j == old_row && 12 == old_column)
                {
                    draw_button(255, 129, 305, 154, 1, BUTTON_BOUNDARY_COLOR, WHITE, line[j][12], BUTTON_CHAR_COLOR);
                }
                if(j == curr_row && 12 == curr_column)
                {
                    draw_button(255, 129, 305, 154, 1, BUTTON_BOUNDARY_COLOR, GREEN, line[j][12], BUTTON_CHAR_COLOR);
                }
            }

            if(j == 2)
            {
                for(int i = 0; i < 11; i++)
                {

                    if(j == old_row && i == old_column)
                    {
                        draw_button(25 + i * 20, 156, 25 + i * 20 + 18, 181, 1, BUTTON_BOUNDARY_COLOR, WHITE, line[j][i], BUTTON_CHAR_COLOR);
                    }
                    if(j == curr_row && i == curr_column)
                    {
                        draw_button(25 + i * 20, 156, 25 + i * 20 + 18, 181, 1, BUTTON_BOUNDARY_COLOR, GREEN, line[j][i], BUTTON_CHAR_COLOR);
                    }
                }

                if(j == old_row && 11 == old_column)
                {
                    draw_button(245, 156, 295, 181, 1, BUTTON_BOUNDARY_COLOR, WHITE, line[j][11], BUTTON_CHAR_COLOR);
                }
                if(j == curr_row && 11 == curr_column)
                {
                    draw_button(245, 156, 295, 181, 1, BUTTON_BOUNDARY_COLOR, GREEN, line[j][11], BUTTON_CHAR_COLOR);
                }
            }
            if(j == 3)
            {
                for(int i = 0; i < 10; i++)
                {

                    if(j == old_row && i == old_column)
                    {
                        draw_button(35 + i * 20, 183, 35 + i * 20 + 18, 208, 1, BUTTON_BOUNDARY_COLOR, WHITE, line[j][i], BUTTON_CHAR_COLOR);
                    }
                    if(j == curr_row && i == curr_column)
                    {
                        draw_button(35 + i * 20, 183, 35 + i * 20 + 18, 208, 1, BUTTON_BOUNDARY_COLOR, GREEN, line[j][i], BUTTON_CHAR_COLOR);
                    }
                }

                if(j == old_row && 10 == old_column)
                {
                    draw_button(235, 183, 285, 208, 1, BUTTON_BOUNDARY_COLOR, WHITE, line[j][10], BUTTON_CHAR_COLOR);
                }
                if(j == curr_row && 10 == curr_column)
                {
                    draw_button(235, 183, 285, 208, 1, BUTTON_BOUNDARY_COLOR, GREEN, line[j][10], BUTTON_CHAR_COLOR);
                }
            }
            if(j == 4)
            {

                if(j == old_row && 0 == old_column)
                {
                    draw_button(45, 210, 213, 235, 1, BUTTON_BOUNDARY_COLOR, WHITE, line[j][0], BUTTON_CHAR_COLOR);
                }
                if(j == curr_row && 0 == curr_column)
                {
                    draw_button(45, 210, 213, 235, 1, BUTTON_BOUNDARY_COLOR, GREEN, line[j][0], BUTTON_CHAR_COLOR);
                }
                if(j == old_row && 1 == old_column)
                {
                    draw_button(215, 210, 275, 235, 1, BUTTON_BOUNDARY_COLOR, WHITE, line[j][1], BUTTON_CHAR_COLOR);
                }
                if(j == curr_row && 1 == curr_column)
                {
                    draw_button(215, 210, 275, 235, 1, BUTTON_BOUNDARY_COLOR, GREEN, line[j][1], BUTTON_CHAR_COLOR);
                }
            }
        }

        old_row = curr_row;
        old_column = curr_column;
    }
}

void draw_wifi_login_page(uint8_t *wifi_connected, uint8_t *wifi_name, uint8_t page_num)
{

    uint8_t title[35] = {'\0'};
    tfp_sprintf(title, "Enter %s\'s passwd", wifi_name);
    uint8_t passwd[15] = {'\0'};

    lcd_clear(BACKGROUND_COLOR);
    draw_page_title(title, TITLE_COLOR);
    draw_button(60, 45, 260, 75, 2, BUTTON_BOUNDARY_COLOR, WHITE, passwd, BUTTON_CHAR_COLOR);
    draw_wifi_login_key_page1(0, 0, NULL, 0);
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