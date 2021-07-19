
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "pin_config.h"

#include "mbed_platform.h"

void hardware_init(void)
{
    // fpioa映射
    fpioa_set_function(PIN_UART_USB_RX, FUNC_UART_USB_RX);
    fpioa_set_function(PIN_UART_USB_TX, FUNC_UART_USB_TX);
}
//************************* Mbedtls code start *****************************



//************************* Mbedtls code end *******************************

int main(void)
{
    hardware_init();
    // 初始化串口3，设置波特率为115200
    uart_init(UART_USB_NUM);
    uart_configure(UART_USB_NUM, 115200, UART_BITWIDTH_8BIT, UART_STOP_1, UART_PARITY_NONE);

    /* 开机发送hello yahboom! */
    char *log = {"Run mbedtls on K210\n"};
    uart_send_data(UART_USB_NUM, log, strlen(log));

    //******************************* mbedtls code start *******************



    //******************************* mbedtls code end **********************

    while(1)
    {
    }
    return 0;
}
