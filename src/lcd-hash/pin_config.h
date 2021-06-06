#ifndef _PIN_CONFIG_H_
#define _PIN_CONFIG_H_
/*****************************HEAR-FILE************************************/
#include "fpioa.h"
#include "uart.h"

/*****************************HARDWARE-PIN*********************************/
// 硬件IO口，与原理图对应
#define PIN_LCD_CS             (36)
#define PIN_LCD_RST            (37)
#define PIN_LCD_RS             (38)
#define PIN_LCD_WR             (39)

#define PIN_FT_RST             (37)
#define PIN_FT_INT             (12)
#define PIN_FT_SCL             (9)
#define PIN_FT_SDA             (10)

// 硬件IO口，与原理图对应
#define PIN_UART_USB_RX       (4)
#define PIN_UART_USB_TX       (5)

#define PIN_UART_WIFI_RX      (13)
#define PIN_UART_WIFI_TX      (14)

/*****************************SOFTWARE-GPIO********************************/
// 软件GPIO口，与程序对应
#define LCD_RST_GPIONUM        (0)
#define LCD_RS_GPIONUM         (1)

#define FT_INT_GPIONUM         (2)
#define FT_RST_GPIONUM         (3)

#define UART_USB_NUM           UART_DEVICE_3
#define UART_WIFI_NUM          UART_DEVICE_1

/*****************************FUNC-GPIO************************************/
// GPIO口的功能，绑定到硬件IO口
#define FUNC_LCD_CS             (FUNC_SPI0_SS3)
#define FUNC_LCD_RST            (FUNC_GPIOHS0 + LCD_RST_GPIONUM)
#define FUNC_LCD_RS             (FUNC_GPIOHS0 + LCD_RS_GPIONUM)
#define FUNC_LCD_WR             (FUNC_SPI0_SCLK)

#define FUNC_FT_RST             (FUNC_GPIOHS0 + FT_RST_GPIONUM)
#define FUNC_FT_INT             (FUNC_GPIOHS0 + FT_INT_GPIONUM)
#define FUNC_FT_SCL             (FUNC_I2C0_SCLK)
#define FUNC_FT_SDA             (FUNC_I2C0_SDA)

#define FUNC_UART_USB_RX       (FUNC_UART1_RX + UART_USB_NUM * 2)
#define FUNC_UART_USB_TX       (FUNC_UART1_TX + UART_USB_NUM * 2)

#define FUNC_UART_WIFI_RX      (FUNC_UART1_RX + UART_WIFI_NUM * 2)
#define FUNC_UART_WIFI_TX      (FUNC_UART1_TX + UART_WIFI_NUM * 2)

#endif /* _PIN_CONFIG_H_ */
