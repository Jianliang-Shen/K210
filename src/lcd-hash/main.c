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

corelock_t lock;


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
    uint8_t page_state = START_PAGE;

    hardware_init();
    io_set_power();
    plic_init();
    sysctl_enable_irq();
    keypad_init();
    init_key();

    /* 设置新PLL0频率 */
    sysctl_pll_set_freq(SYSCTL_PLL0, 800000000);
    uarths_init();

    /* 初始化flash */
    uint8_t res = 0;
    res = flash_init();
    if(res == 0)
        return 0;

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

    w25qxx_write_data(PIC_ADDRESS, gImage_picture_data, sizeof(gImage_picture_data));
    printf("picture writed to flash\n");

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
                    page_state = start_page_operation();
                    break;
                case CONNECT_SERVER_PAGE:
                    page_state = connect_server_operation();
                    break;
                case PIC_DOWNLOAD_PAGE:
                    page_state = pic_download_operation();
                    break;
                case OPEN_PICTURE_PAGE:
                    page_state = open_picture_operation();
                    break;
                case WIFI_LOGIN_PAGE:
                    page_state = wifi_login_operation();
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
