#include <stdio.h>
#include "bsp.h"
#include "dvp.h"
#include "ft6236u.h"
#include "gpiohs.h"
#include "incbin.h"
#include "keypad.h"
#include "kpu.h"
#include "lcd.h"
#include "operation.h"
#include "pin_config.h"
#include "region_layer.h"
#include "save_data.h"
#include "sleep.h"
#include "st7789.h"
#include "stdio.h"
#include "string.h"
#include "sysctl.h"
#include "uarths.h"
#include "ui.h"
#include "w25qxx.h"
#include "wifi.h"

uint8_t r_buf[76800];

corelock_t lock;

#define INCBIN_STYLE INCBIN_STYLE_SNAKE
#define INCBIN_PREFIX

volatile uint32_t g_ai_done_flag;
kpu_model_context_t face_detect_task;
static region_layer_t face_detect_rl;
static obj_info_t face_detect_info;
#define ANCHOR_NUM 5

float g_anchor[ANCHOR_NUM * 2] = {0.57273, 0.677385, 1.87446, 2.06253, 3.33843, 5.47434, 7.88282, 3.52778, 9.77052, 9.16828};

#define LOAD_KMODEL_FROM_FLASH 0

#if LOAD_KMODEL_FROM_FLASH
#define KMODEL_SIZE (380 * 1024)
uint8_t *model_data;
#else
INCBIN(model, "detect.kmodel");
#endif

static int ai_done(void *ctx)
{
    g_ai_done_flag = 1;
    return 0;
}

//画人脸识别框
static void draw_edge(uint32_t *gram, obj_info_t *obj_info, uint32_t index, uint16_t color)
{
    uint32_t data = ((uint32_t)color << 16) | (uint32_t)color;
    uint32_t *addr1, *addr2, *addr3, *addr4, x1, y1, x2, y2;

    x1 = obj_info->obj[index].x1;
    y1 = obj_info->obj[index].y1;
    x2 = obj_info->obj[index].x2;
    y2 = obj_info->obj[index].y2;

    if(x1 <= 0)
        x1 = 1;
    if(x2 >= 319)
        x2 = 318;
    if(y1 <= 0)
        y1 = 1;
    if(y2 >= 239)
        y2 = 238;

    addr1 = gram + (320 * y1 + x1) / 2;
    addr2 = gram + (320 * y1 + x2 - 8) / 2;
    addr3 = gram + (320 * (y2 - 1) + x1) / 2;
    addr4 = gram + (320 * (y2 - 1) + x2 - 8) / 2;
    for(uint32_t i = 0; i < 4; i++)
    {
        *addr1 = data;
        *(addr1 + 160) = data;
        *addr2 = data;
        *(addr2 + 160) = data;
        *addr3 = data;
        *(addr3 + 160) = data;
        *addr4 = data;
        *(addr4 + 160) = data;
        addr1++;
        addr2++;
        addr3++;
        addr4++;
    }
    addr1 = gram + (320 * y1 + x1) / 2;
    addr2 = gram + (320 * y1 + x2 - 2) / 2;
    addr3 = gram + (320 * (y2 - 8) + x1) / 2;
    addr4 = gram + (320 * (y2 - 8) + x2 - 2) / 2;
    for(uint32_t i = 0; i < 8; i++)
    {
        *addr1 = data;
        *addr2 = data;
        *addr3 = data;
        *addr4 = data;
        addr1 += 160;
        addr2 += 160;
        addr3 += 160;
        addr4 += 160;
    }
}

int flash_init(void)
{
    uint8_t manuf_id, device_id;
    uint8_t spi_index = 3, spi_ss = 0;
    printf("flash init \n");

    w25qxx_init(spi_index, spi_ss);
    /* 读取flash的ID */
    w25qxx_read_id(&manuf_id, &device_id);
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
    // sysctl_enable_irq();
    keypad_init();
    init_key();

    sysctl_pll_set_freq(SYSCTL_PLL0, 800000000);
    sysctl_pll_set_freq(SYSCTL_PLL1, 400000000UL);
    sysctl_clock_enable(SYSCTL_CLOCK_AI);

    uarths_init();

    /* 初始化flash */
    uint8_t res = 0;
    res = flash_init();
    if(res == 0)
        return 0;
    w25qxx_enable_quad_mode(); /* flash 四倍模式开启*/

    for(int i = 0; i < 76800; i++)
    {
        r_buf[i] = gImage_picture_data_24[8 + 3 * i];
    }

    // w25qxx_write_data(PIC_ADDRESS, gImage_picture_data, sizeof(gImage_picture_data));
    // printf("picture writed to flash\n");

    /*kmodel加载方式： 1：分开烧录模式  2：直接与代码合并编译*/
#if LOAD_KMODEL_FROM_FLASH
    model_data = (uint8_t *)malloc(KMODEL_SIZE + 255);
    uint8_t *model_data_align = (uint8_t *)(((uintptr_t)model_data + 255) & (~255));
    w25qxx_read_data(0xA00000, model_data_align, KMODEL_SIZE, W25QXX_QUAD_FAST);
#else
    uint8_t *model_data_align = model_data;
#endif

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

    if(kpu_load_kmodel(&face_detect_task, model_data_align) != 0)
    {
        printf("\nmodel init error\n");
        while(1)
            ;
    }

    //人脸层配置参数
    face_detect_rl.anchor_number = ANCHOR_NUM;
    face_detect_rl.anchor = g_anchor;
    face_detect_rl.threshold = 0.7;
    face_detect_rl.nms_value = 0.3;
    region_layer_init(&face_detect_rl, 20, 15, 30, 320, 240);

    sysctl_enable_irq();

    g_ai_done_flag = 0;
    int8_t result = kpu_run_kmodel(&face_detect_task, r_buf, DMAC_CHANNEL5, ai_done, NULL);

    while(!g_ai_done_flag)
        ; //等待KPU处理完成

    float *output;
    size_t output_size;
    // 获取 KPU 最终处理的结果
    result = kpu_get_output(&face_detect_task, 0, (uint8_t **)&output, &output_size);
    printf("kpu_get_output result = %d\n", result);

    /*算法检测人脸*/
    face_detect_rl.input = output;
    region_layer_run(&face_detect_rl, &face_detect_info);
    printf("face_detect_info.obj_number = %d\n", face_detect_info.obj_number);

    /*根据返回值进行人脸圈住 */
    for(uint32_t face_cnt = 0; face_cnt < face_detect_info.obj_number; face_cnt++)
    {
        draw_edge((uint32_t *)gImage_picture_data, &face_detect_info, face_cnt, RED);
    }
    /* display result */
    lcd_draw_picture_half(0, 0, 320, 240, gImage_picture_data);
    kpu_model_free(&face_detect_task);

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
