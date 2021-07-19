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
#include "wifi.h"

corelock_t lock;
uint8_t wifi_log[MAX_WIFI_LOG_SIZE];
uint8_t wifi_log_clear_flag = 0;

void hardware_init(void)
{
    //init lcd pins
    fpioa_set_function(PIN_LCD_CS, FUNC_LCD_CS);
    fpioa_set_function(PIN_LCD_RST, FUNC_LCD_RST);
    fpioa_set_function(PIN_LCD_RS, FUNC_LCD_RS);
    fpioa_set_function(PIN_LCD_WR, FUNC_LCD_WR);

    /* 使能SPI0和DVP数据 */
    sysctl_set_spi0_dvp_data(1);

    /* I2C FT6236 */
    // fpioa_set_function(PIN_FT_RST, FUNC_FT_RST);
    fpioa_set_function(PIN_FT_INT, FUNC_FT_INT);
    fpioa_set_function(PIN_FT_SCL, FUNC_FT_SCL);
    fpioa_set_function(PIN_FT_SDA, FUNC_FT_SDA);
}

uint8_t start_page_operation(uint8_t *connect_server, uint8_t *pic_download)
{
    if(BUTTON_1_X1 < ft6236.touch_x && ft6236.touch_x < BUTTON_1_X2 &&
       BUTTON_1_Y1 < ft6236.touch_y && ft6236.touch_y < BUTTON_1_Y2)
    {
        draw_button(BUTTON_1_X1, BUTTON_1_Y1, BUTTON_1_X2, BUTTON_1_Y2,
                    BUTTON_BOUNDARY_WIDTH, BUTTON_BOUNDARY_COLOR, BUTTON_TRIGGER_COLOR,
                    "1.Connect Server", BUTTON_CHAR_COLOR);
        msleep(50);

        draw_connect_server_page(connect_server);
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

uint8_t connect_server_operation(uint8_t *connect_server, uint8_t *pic_download)
{
    static int8_t cur_idx = 0; //index 从0开始计数，最大值为 MAX_WIFI_NUM - 1
    static uint8_t cur_wifi_name[16] = {'\0'};
    static uint8_t wifi_searched = 0;
    static uint8_t wifi_num = 0;

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

        //打印wifi列表
        draw_wifi_list(&wifi_searched, 1, wifi_log, cur_idx, &wifi_num, cur_wifi_name);
        printf("current index = %d, wifi_num =%d wifi name = %s\n", cur_idx, wifi_num, cur_wifi_name);

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

        draw_wifi_list(&wifi_searched, 0, wifi_log, cur_idx, &wifi_num, cur_wifi_name);
        printf("current index = %d, wifi_num =%d wifi name = %s\n", cur_idx, wifi_num, cur_wifi_name);

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

        draw_wifi_list(&wifi_searched, 0, wifi_log, cur_idx, &wifi_num, cur_wifi_name);
        printf("current index = %d, wifi_num =%d wifi name = %s\n", cur_idx, wifi_num, cur_wifi_name);

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
        cur_idx = 0;
        wifi_searched = 0;

        draw_start_page();

        return START_PAGE;
    }
    return CONNECT_SERVER_PAGE;
}

uint8_t pic_download_operation(uint8_t *connect_server, uint8_t *pic_download)
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
    // return PIC_DOWNLOAD_PAGE;
}

uint8_t open_picture_operation(uint8_t *connect_server, uint8_t *pic_download)
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

int core1_main(void *ctx)
{
    char recv = 0, send = 0;
    int idx = 0;
    //关闭透传模式
    wifi_send_cmd("+++");

    //设置wsp8266为wifi连接模式
    wifi_send_cmd("AT+CWMODE_DEF=1\r\n");

    while(1)
    {
        //判断是否清空缓冲区数据
        if(wifi_log_clear_flag == 1)
        {
            printf("wifi returns: %s\n", wifi_log);
            memset(wifi_log, '\0', 500);
            idx = 0;
            wifi_log_clear_flag = 0;
        }
        /* 接收WIFI模块的信息 */
        if(uart_receive_data(UART_WIFI_NUM, &recv, 1))
        {
            corelock_lock(&lock);
            {
                wifi_log[idx++] = recv;
            }
            corelock_unlock(&lock);
        }
    }
}

int main(void)
{
    hardware_init();
    io_set_power();
    plic_init();
    sysctl_enable_irq();
    lcd_init();
    ft6236_init();
    draw_start_page();

    wifi_module_init();
    register_core1(core1_main, NULL);

    // wifi_send_cmd_and_printf_log("AT+CWJAP_DEF=\"jianliang\",\"95898063\"\r\n");

    uint8_t page_state = START_PAGE;

    uint8_t connect_server = 0;
    uint8_t pic_download = 0;

    while(1)
    {
        if(ft6236.touch_state & TP_COORD_UD)
        {
            ft6236.touch_state &= ~TP_COORD_UD;
            ft6236_scan();
            // printf("X=%d, Y=%d \n ", ft6236.touch_x, ft6236.touch_y);

            switch(page_state)
            {
                case START_PAGE:

                    page_state = start_page_operation(&connect_server, &pic_download);
                    break;

                case CONNECT_SERVER_PAGE:
                    page_state = connect_server_operation(&connect_server, &pic_download);
                    break;
                case PIC_DOWNLOAD_PAGE:
                    page_state = pic_download_operation(&connect_server, &pic_download);

                    break;
                case OPEN_PICTURE_PAGE:
                    page_state = open_picture_operation(&connect_server, &pic_download);

                    break;
                case ERROR_PAGE:
                    page_state = error_operation();
                    break;

                default:
                    break;
            }
        }
        ft6236.touch_state &= ~TP_COORD_UD;

        msleep(50);
    }
    return 0;
}
