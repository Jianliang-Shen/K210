#include <stdio.h>
#include "bsp.h"
#include "ft6236u.h"
#include "gpiohs.h"
#include "keypad.h"
#include "lcd.h"
#include "operation.h"
#include "pin_config.h"
#include "save_data.h"
#include "sleep.h"
#include "st7789.h"
#include "string.h"
#include "sysctl.h"
#include "uarths.h"
#include "ui.h"
#include "w25qxx.h"
#include "wifi.h"

uint32_t key_count = 0;
uint8_t tmp[153608] = {'\0'};
uint8_t pic_download = 1;

uint8_t start_page_operation()
{
    if(BUTTON_1_X1 < ft6236.touch_x && ft6236.touch_x < BUTTON_1_X2 &&
       BUTTON_1_Y1 < ft6236.touch_y && ft6236.touch_y < BUTTON_1_Y2)
    {
        draw_button(BUTTON_1_X1, BUTTON_1_Y1, BUTTON_1_X2, BUTTON_1_Y2,
                    BUTTON_BOUNDARY_WIDTH, BUTTON_BOUNDARY_COLOR, BUTTON_TRIGGER_COLOR,
                    "1.Connect Server", BUTTON_CHAR_COLOR);
        msleep(50);

        draw_connect_server_page();
        return CONNECT_SERVER_PAGE;
    } else if(BUTTON_2_X1 < ft6236.touch_x && ft6236.touch_x < BUTTON_2_X2 &&
              BUTTON_2_Y1 < ft6236.touch_y && ft6236.touch_y < BUTTON_2_Y2)
    {
        draw_button(BUTTON_2_X1, BUTTON_2_Y1, BUTTON_2_X2, BUTTON_2_Y2,
                    BUTTON_BOUNDARY_WIDTH, BUTTON_BOUNDARY_COLOR, BUTTON_TRIGGER_COLOR,
                    "2.Download picture", BUTTON_CHAR_COLOR);
        msleep(50);

        if(connect_server != CONNECT_SERVER_OK)
        {
            draw_error_page("No server connect");
            return ERROR_PAGE;
        }
        draw_pic_download_page();
        return PIC_DOWNLOAD_PAGE;
    } else if(BUTTON_3_X1 < ft6236.touch_x && ft6236.touch_x < BUTTON_3_X2 &&
              BUTTON_3_Y1 < ft6236.touch_y && ft6236.touch_y < BUTTON_3_Y2)
    {
        draw_button(BUTTON_3_X1, BUTTON_3_Y1, BUTTON_3_X2, BUTTON_3_Y2,
                    BUTTON_BOUNDARY_WIDTH, BUTTON_BOUNDARY_COLOR, BUTTON_TRIGGER_COLOR,
                    "3.Open picture", BUTTON_CHAR_COLOR);
        msleep(50);

        if(pic_download != PIC_DONWLOAD_OK)
        {
            draw_error_page("No picture downloaded");
            return ERROR_PAGE;
        }
        draw_open_pic_page();
        return OPEN_PICTURE_PAGE;
    }

    return START_PAGE;
}

uint8_t connect_server_operation()
{
    static int8_t cur_idx = 0; //index 从0开始计数，最大值为 MAX_WIFI_NUM - 1

    if(connect_server != CONNECT_SERVER_OK)
    {
        //如果按下搜寻wifi的按钮
        if(8 < ft6236.touch_x && ft6236.touch_x < 156 &&
           164 < ft6236.touch_y && ft6236.touch_y < 194)
        {
            //闪烁
            draw_button(8, 164, 156, 194,
                        2, BUTTON_BOUNDARY_COLOR, BUTTON_TRIGGER_COLOR,
                        "Search Wifi", BUTTON_CHAR_COLOR);

            //搜寻wifi
            wifi_send_cmd("AT+CWLAP\r\n");

            //等待处理
            sleep(2);

            //重置静态变量现场
            cur_idx = 0;

            //打印wifi列表
            draw_wifi_list(1, wifi_log, cur_idx, 0);
            printf("current index = %d, wifi_num =%d wifi name = %s\n", cur_idx, wifi_num, wifi_name);

            wifi_log_clear_flag = 1; //清楚wifi消息

            //搜寻完毕后，按钮变正常
            draw_button(8, 164, 156, 194,
                        2, BUTTON_BOUNDARY_COLOR, BUTTON_NORMAL_COLOR, "Search Wifi", BUTTON_CHAR_COLOR);
            draw_button(164, 164, 312, 194,
                        2, BUTTON_BOUNDARY_COLOR, BUTTON_NORMAL_COLOR, "Connect", BUTTON_CHAR_COLOR);
        }

        //如果按下连接按钮
        if(164 < ft6236.touch_x && ft6236.touch_x < 312 &&
           164 < ft6236.touch_y && ft6236.touch_y < 194 && wifi_searched)
        {
            draw_button(164, 164, 312, 194,
                        2, BUTTON_BOUNDARY_COLOR, BUTTON_TRIGGER_COLOR,
                        "Connect", BUTTON_CHAR_COLOR);
            msleep(50);
            draw_wifi_login_page();

            //重置静态变量现场
            cur_idx = 0;
            return WIFI_LOGIN_PAGE;

            //跳转到连接函数，绘制连接动画
            //如果连接失败
            // draw_error_page("connect failed");
            // return ERROR_PAGE;
            //如果连接成功
            //打印连接成功的logo，在wifi后加上对勾
        }

        if(242 < ft6236.touch_x && ft6236.touch_x < 312 &&
           24 < ft6236.touch_y && ft6236.touch_y < 86 && wifi_searched)
        {
            draw_button(242, 24, 312, 86,
                        2, BUTTON_BOUNDARY_COLOR, BUTTON_TRIGGER_COLOR,
                        "up", BUTTON_CHAR_COLOR);
            cur_idx--;
            if(cur_idx < 0)
            {
                cur_idx = wifi_num;
            }

            draw_wifi_list(0, wifi_log, cur_idx, 0);
            printf("current index = %d, wifi_num =%d wifi name = %s\n", cur_idx, wifi_num, wifi_name);

            draw_button(242, 24, 312, 86,
                        2, BUTTON_BOUNDARY_COLOR, BUTTON_NORMAL_COLOR,
                        "up", BUTTON_CHAR_COLOR);
        }

        if(242 < ft6236.touch_x && ft6236.touch_x < 312 &&
           94 < ft6236.touch_y && ft6236.touch_y < 156 && wifi_searched)
        {
            draw_button(242, 94, 312, 156,
                        2, BUTTON_BOUNDARY_COLOR, BUTTON_TRIGGER_COLOR,
                        "down", BUTTON_CHAR_COLOR);
            cur_idx++;
            if(cur_idx > wifi_num)
            {
                cur_idx = 0;
            }

            draw_wifi_list(0, wifi_log, cur_idx, 0);
            printf("current index = %d, wifi_num =%d wifi name = %s\n", cur_idx, wifi_num, wifi_name);

            draw_button(242, 94, 312, 156,
                        2, BUTTON_BOUNDARY_COLOR, BUTTON_NORMAL_COLOR,
                        "down", BUTTON_CHAR_COLOR);
        }
        if(BACK_BUTTON_X1 < ft6236.touch_x && ft6236.touch_x < BACK_BUTTON_X2 &&
           BACK_BUTTON_Y1 < ft6236.touch_y && ft6236.touch_y < BACK_BUTTON_Y2)
        {
            draw_button(BACK_BUTTON_X1, BACK_BUTTON_Y1, BACK_BUTTON_X2, BACK_BUTTON_Y2,
                        2, BUTTON_BOUNDARY_COLOR, BUTTON_TRIGGER_COLOR, "BACK", BUTTON_CHAR_COLOR);
            msleep(50);

            //重置静态变量现场
            cur_idx = 0;

            draw_start_page();

            return START_PAGE;
        }
    } else
    {
        if(BACK_BUTTON_X1 < ft6236.touch_x && ft6236.touch_x < BACK_BUTTON_X2 &&
           164 < ft6236.touch_y && ft6236.touch_y < 194)
        {
            draw_button(BACK_BUTTON_X1, 164, BACK_BUTTON_X2, 194,
                        2, BUTTON_BOUNDARY_COLOR, BUTTON_TRIGGER_COLOR, "Disconnect", BUTTON_CHAR_COLOR);
            msleep(50);

            //重置静态变量现场
            cur_idx = 0;

            server_disconnect(connect_server);
            draw_connect_server_page();

            printf("wifi disconnect end, connect_server = %d\n", connect_server);

            return CONNECT_SERVER_PAGE;
        }

        if(BACK_BUTTON_X1 < ft6236.touch_x && ft6236.touch_x < BACK_BUTTON_X2 &&
           BACK_BUTTON_Y1 < ft6236.touch_y && ft6236.touch_y < BACK_BUTTON_Y2)
        {
            draw_button(BACK_BUTTON_X1, BACK_BUTTON_Y1, BACK_BUTTON_X2, BACK_BUTTON_Y2,
                        2, BUTTON_BOUNDARY_COLOR, BUTTON_TRIGGER_COLOR, "BACK", BUTTON_CHAR_COLOR);
            msleep(50);

            //重置静态变量现场
            cur_idx = 0;

            draw_start_page();

            return START_PAGE;
        }
    }

    return CONNECT_SERVER_PAGE;
}

uint8_t wifi_login_operation()
{
    static int8_t key_value = 0;
    static uint8_t shift_on = 0;

    if(shift_on == 0)
    {
        draw_wifi_login_key_page1(curr_row, curr_column, &key_value, 1);
    } else
    {
        draw_wifi_login_key_page2(curr_row, curr_column, &key_value, 1);
    }
    if(key_count == 1)
    {
        switch(key_value)
        {
            case -1:
                if(wifi_login_enter_flag == SET_PASSWD && passwd_idx > 0)
                {
                    passwd_idx--;
                    passwd[passwd_idx] = '\0';
                }

                if(wifi_login_enter_flag == SET_IP && server_ip_idx > 0)
                {
                    server_ip_idx--;
                    server_ip[server_ip_idx] = '\0';
                } else if(wifi_login_enter_flag == SET_IP && server_ip_idx == 0)
                {
                    wifi_login_enter_flag--;
                }

                if(wifi_login_enter_flag == SET_PORT && server_port_idx > 0)
                {
                    server_port_idx--;
                    server_port[server_port_idx] = '\0';
                } else if(wifi_login_enter_flag == SET_PORT && server_port_idx == 0)
                {
                    wifi_login_enter_flag--;
                }
                if(wifi_login_enter_flag == CONNECT_WIFI)
                {
                    wifi_login_enter_flag--;
                }

                break;
            case -2:
                //Enter 按下后光标下移一行
                wifi_login_enter_flag++;
                if(wifi_login_enter_flag == CONNECT_WIFI)
                {
                    wifi_login_enter_flag--;
                    return server_connect();
                    // printf("wifi connect end, connect_server = %d\n", connect_server);
                }
                break;
            case -3:
                if(shift_on == 0)
                {
                    draw_wifi_login_key_page2(curr_row, curr_column, &key_value, 0);
                    shift_on = 1;
                } else
                {
                    draw_wifi_login_key_page1(curr_row, curr_column, &key_value, 0);
                    shift_on = 0;
                }
                break;
            case -4:
                draw_connect_server_page();
                shift_on = 0;
                key_value = 0;
                curr_row = 0;
                curr_column = 0;
                // memset(passwd, '\0', sizeof(passwd));
                // passwd_idx = 0;
                return CONNECT_SERVER_PAGE;
                break;
            default:
                if(wifi_login_enter_flag == SET_PASSWD && passwd_idx < sizeof(passwd))
                {
                    passwd[passwd_idx] = '\0' + key_value;
                    passwd_idx++;
                }
                if(wifi_login_enter_flag == SET_IP && server_ip_idx < sizeof(server_ip))
                {
                    server_ip[server_ip_idx] = '\0' + key_value;
                    server_ip_idx++;
                }
                if(wifi_login_enter_flag == SET_PORT && server_port_idx < sizeof(server_port))
                {
                    server_port[server_port_idx] = '\0' + key_value;
                    server_port_idx++;
                }
                break;
        }
        if(wifi_login_enter_flag == SET_PASSWD)
            draw_button(95, 33, 290, 47, 1, WHITE, WHITE, passwd, BUTTON_CHAR_COLOR);
        if(wifi_login_enter_flag == SET_IP)
            draw_button(95, 48, 290, 72, 1, WHITE, WHITE, server_ip, BUTTON_CHAR_COLOR);
        if(wifi_login_enter_flag == SET_PORT)
            draw_button(95, 73, 290, 97, 1, WHITE, WHITE, server_port, BUTTON_CHAR_COLOR);
        // if(connect_server == CONNECT_SERVER_OK)
        // {
        //     draw_connect_server_page();
        //     return CONNECT_SERVER_PAGE;
        // }
    }

    return WIFI_LOGIN_PAGE;
}

uint8_t pic_download_operation()
{
    // if () //如果按下下载按钮
    // {
    //     //如果已经下载，弹窗提示是否重新下载
    //     //跳转到下载函数(图片数据和散列值存到sd card中)，绘制下载动画
    // }
    // if () //如果按下校验按钮
    // {
    //     //计算散列值并校验
    //     //成功后打印log
    //     //失败时draw error pag
    //     // draw_error_page("hash failed");
    //     // return ERROR_PAGE;
    // }
    // if () //如果按下返回按钮
    // {
    //     draw_start_page();
    //     return START_PAGE;
    // }

    if(BACK_BUTTON_X1 < ft6236.touch_x && ft6236.touch_x < BACK_BUTTON_X2 &&
       BACK_BUTTON_Y1 < ft6236.touch_y && ft6236.touch_y < BACK_BUTTON_Y2)
    {
        draw_button(BACK_BUTTON_X1, BACK_BUTTON_Y1, BACK_BUTTON_X2, BACK_BUTTON_Y2,
                    2, BUTTON_BOUNDARY_COLOR, BUTTON_TRIGGER_COLOR, "BACK", BUTTON_CHAR_COLOR);
        msleep(50);

        draw_start_page();

        return START_PAGE;
    }
    return PIC_DOWNLOAD_PAGE;
}

uint8_t open_picture_operation()
{
    // if () //如果按下打开按钮
    // {
    //     //load图像数据和散列值
    //     //校验成功后绘制图片，保持五秒后推出
    //     //校验失败后，弹出错误信息并返回
    //     // draw_error_page("hash failed");
    //     // return ERROR_PAGE;
    // }
    // if () //如果按下返回按钮
    // {
    //     draw_start_page();
    //     return START_PAGE;
    // }
    // return OPEN_PICTURE_PAGE;
    if(0 < ft6236.touch_x && ft6236.touch_x < 320 &&
       1 < ft6236.touch_y && ft6236.touch_y < 240)
    {
        draw_start_page();

        return START_PAGE;
    }
}
uint8_t error_operation()
{
    if(BACK_BUTTON_X1 < ft6236.touch_x && ft6236.touch_x < BACK_BUTTON_X2 &&
       BACK_BUTTON_Y1 < ft6236.touch_y && ft6236.touch_y < BACK_BUTTON_Y2)
    {
        draw_button(BACK_BUTTON_X1, BACK_BUTTON_Y1, BACK_BUTTON_X2, BACK_BUTTON_Y2,
                    2, BUTTON_BOUNDARY_COLOR, BUTTON_TRIGGER_COLOR, "BACK", BUTTON_CHAR_COLOR);
        msleep(50);

        draw_start_page();

        return START_PAGE;
    }
    return ERROR_PAGE;
}
