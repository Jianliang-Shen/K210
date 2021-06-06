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

void wifi_module_init()
{
    /* USB串口 */
    fpioa_set_function(PIN_UART_USB_RX, FUNC_UART_USB_RX);
    fpioa_set_function(PIN_UART_USB_TX, FUNC_UART_USB_TX);

    /* WIFI模块串口 */
    fpioa_set_function(PIN_UART_WIFI_RX, FUNC_UART_WIFI_RX);
    fpioa_set_function(PIN_UART_WIFI_TX, FUNC_UART_WIFI_TX);

    // 初始化USB串口，设置波特率为115200
    uart_init(UART_USB_NUM);
    uart_configure(UART_USB_NUM, 115200, UART_BITWIDTH_8BIT, UART_STOP_1, UART_PARITY_NONE);

    /* 初始化WiFi模块的串口 */
    uart_init(UART_WIFI_NUM);
    uart_configure(UART_WIFI_NUM, 115200, UART_BITWIDTH_8BIT, UART_STOP_1, UART_PARITY_NONE);
}

void wifi_send_cmd_and_printf_log(char *cmd)
{
    char recv = 0, send = 0;

    char recv_buff[100] = {'\0'};
    uint8_t idx = 0;
    uart_send_data(UART_WIFI_NUM, cmd, strlen(cmd));
    sleep(2);

    // uart_receive_data(UART_WIFI_NUM, recv_buff, 100);
    // printf("wifi returns: %s\n",recv_buff);
    // printf("already send: %s\n",cmd);
    // while(uart_receive_data(UART_WIFI_NUM, &recv, 1) ||
    //       (uart_send_data(UART_WIFI_NUM, cmd[idx++], 1) > 0 && idx < strlen(cmd)))
    // {
    //     /* 发送WiFi的数据到USB串口显示 */
    //     uart_send_data(UART_USB_NUM, &recv, 1);
    //     // recv_buff[idx++] = recv;
    // }
    
    // while (1)
    // {
    //     /* 接收WIFI模块的信息 */
    //     if(uart_receive_data(UART_WIFI_NUM, &recv, 1))
    //     {
    //         /* 发送WiFi的数据到USB串口显示 */
    //         uart_send_data(UART_USB_NUM, &recv, 1);
    //     }

    //     /* 接收串口的信息，并发送给WiFi模块 */
    //     if(uart_receive_data(UART_USB_NUM, &send, 1))
    //     {
    //         uart_send_data(UART_WIFI_NUM, &send, 1);
    //     }
    // }
}