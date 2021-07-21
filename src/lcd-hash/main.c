#include <stdio.h>
#include "bsp.h"
#include "ft6236u.h"
#include "gpiohs.h"
#include "keypad.h"
#include "lcd.h"
#include "pin_config.h"
#include "sleep.h"
#include "st7789.h"
#include "string.h"
#include "sysctl.h"
#include "uarths.h"
#include "ui.h"
#include "w25qxx.h"
#include "wifi.h"

corelock_t lock;

uint32_t key_count = 0;

// #define BUF_LENGTH (40 * 1024 + 5)
// #define DATA_ADDRESS 0xB00000

// uint8_t write_buf[BUF_LENGTH];
// uint8_t read_buf[BUF_LENGTH];

int flash_init(void)
{
    uint8_t manuf_id, device_id;
    uint8_t spi_index = 3, spi_ss = 0;
    printf("flash init \n");

    w25qxx_init(spi_index, spi_ss, 60000000);
    /* 读取flash的ID */
    w25qxx_read_id(&manuf_id, &device_id);
    printf("manuf_id:0x%02x, device_id:0x%02x\n", manuf_id, device_id);
    if((manuf_id != 0xEF && manuf_id != 0xC8) || (device_id != 0x17 && device_id != 0x16))
    {
        /* flash初始化失败 */
        printf("w25qxx_read_id error\n");
        printf("manuf_id:0x%02x, device_id:0x%02x\n", manuf_id, device_id);
        return 0;
    } else
    {
        return 1;
    }
}

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

    fpioa_set_function(PIN_KEYPAD_LEFT, FUNC_KEYPAD_LEFT);
    fpioa_set_function(PIN_KEYPAD_MIDDLE, FUNC_KEYPAD_MIDDLE);
    fpioa_set_function(PIN_KEYPAD_RIGHT, FUNC_KEYPAD_RIGHT);

    fpioa_set_function(PIN_KEY, FUNC_KEY);
}

int key_irq_cb(void *ctx)
{
    uint32_t *tmp = (uint32_t *)(ctx);
    *tmp = 1;

    return 0;
}

void init_key(void)
{
    /* 设置按键的GPIO模式为上拉输入 */
    gpiohs_set_drive_mode(KEY_GPIONUM, GPIO_DM_INPUT_PULL_UP);
    /* 设置按键的GPIO电平触发模式为上升沿和下降沿 */
    gpiohs_set_pin_edge(KEY_GPIONUM, GPIO_PE_FALLING);
    /* 设置按键GPIO口的中断回调 */
    gpiohs_irq_register(KEY_GPIONUM, 1, key_irq_cb, &key_count);
}

uint8_t start_page_operation(uint8_t *pic_download)
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
        lcd_draw_picture_half(0, 0, 320, 240, picture_data);

        // if(pic_download != PIC_DONWLOAD_OK)
        // {
        //     draw_error_page("No picture downloaded");
        //     return ERROR_PAGE;
        // }
        // draw_open_pic_page();
        return OPEN_PICTURE_PAGE;
    }

    return START_PAGE;
}

uint8_t connect_server_operation(uint8_t *pic_download)
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

void key_left(void *arg)
{
    keypad_flag = 1;
    uint8_t row_nums[] = {12, 12, 11, 10, 1};
    curr_column--;
    for(int j = 0; j < 5; j++)
    {
        if(curr_row == j)
        {
            if(curr_column < 0)
            {
                curr_column = row_nums[j > 0 ? j - 1 : 4];
                curr_row--;
            }
        }
    }
    if(curr_row < 0)
    {
        curr_row = 4;
    }
}

void key_right(void *arg)
{
    keypad_flag = 1;
    uint8_t row_nums[] = {12, 12, 11, 10, 1};
    curr_column++;
    for(int j = 0; j < 5; j++)
    {
        if(curr_row == j)
        {
            if(curr_column > row_nums[j])
            {
                curr_column = 0;
                curr_row++;
            }
        }
    }
    if(curr_row > 4)
    {
        curr_row = 0;
    }
}

void key_middle(void *arg)
{
    keypad_flag = 1;
    uint8_t row_nums[] = {12, 12, 11, 10, 1};

    curr_row++;
    if(curr_row > 4)
    {
        curr_row = 0;
    }
    curr_column = curr_column > row_nums[curr_row] ? row_nums[curr_row] : curr_column;
}

uint8_t wifi_login_operation(uint8_t *pic_download)
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
                    server_connect();
                    printf("wifi connect end, connect_server = %d\n", connect_server);
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
        if(connect_server == CONNECT_SERVER_OK)
        {
            draw_connect_server_page();
            return CONNECT_SERVER_PAGE;
        }
    }

    return WIFI_LOGIN_PAGE;
}

uint8_t pic_download_operation(uint8_t *pic_download)
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

uint8_t open_picture_operation(uint8_t *pic_download)
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
    keypad_init();
    init_key();

    lcd_init();
    ft6236_init();

    /* 设置keypad回调 */
    keypad[EN_KEY_ID_LEFT].short_key_down = key_left;
    keypad[EN_KEY_ID_LEFT].long_key_down = key_left;
    keypad[EN_KEY_ID_LEFT].repeat_key_down = key_left;

    keypad[EN_KEY_ID_MIDDLE].short_key_down = key_middle;
    keypad[EN_KEY_ID_MIDDLE].long_key_down = key_middle;
    keypad[EN_KEY_ID_MIDDLE].repeat_key_down = key_middle;

    keypad[EN_KEY_ID_RIGHT].short_key_down = key_right;
    keypad[EN_KEY_ID_RIGHT].long_key_down = key_right;
    keypad[EN_KEY_ID_RIGHT].repeat_key_down = key_right;

    draw_start_page();

    wifi_module_init();
    register_core1(core1_main, NULL);

    // wifi_send_cmd_and_printf_log("AT+CWJAP_DEF=\"jianliang\",\"95898063\"\r\n");

    uint8_t page_state = START_PAGE;

    uint8_t pic_download = 0;

    /* 设置新PLL0频率 */
    sysctl_pll_set_freq(SYSCTL_PLL0, 800000000);
    uarths_init();

    /* 初始化flash */
    uint8_t res = 0;
    res = flash_init();
    if(res == 0)
        return 0;

    while(1)
    {
        if(ft6236.touch_state & TP_COORD_UD || keypad_flag == 1 || key_count == 1)
        {
            ft6236.touch_state &= ~TP_COORD_UD;
            ft6236_scan();
            // printf("X=%d, Y=%d \n ", ft6236.touch_x, ft6236.touch_y);

            switch(page_state)
            {
                case START_PAGE:
                    page_state = start_page_operation(&pic_download);
                    break;
                case CONNECT_SERVER_PAGE:
                    page_state = connect_server_operation(&pic_download);
                    break;
                case PIC_DOWNLOAD_PAGE:
                    page_state = pic_download_operation(&pic_download);
                    break;
                case OPEN_PICTURE_PAGE:
                    page_state = open_picture_operation(&pic_download);
                    break;
                case WIFI_LOGIN_PAGE:
                    page_state = wifi_login_operation(&pic_download);
                    break;
                case ERROR_PAGE:
                    page_state = error_operation();
                    break;

                default:
                    break;
            }
        }
        ft6236.touch_state &= ~TP_COORD_UD;
        keypad_flag = 0;
        key_count = 0;

        msleep(50);
    }
    return 0;
}
