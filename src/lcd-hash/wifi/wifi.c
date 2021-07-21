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

uint8_t wifi_searched = 0;
uint8_t wifi_num = 0;
uint8_t wifi_name[16] = {'\0'};

uint8_t connect_server = 0;

// uint8_t passwd[20] = {'\0'};
// uint8_t passwd_idx = 0;

// uint8_t server_ip[15] = {'\0'};
// uint8_t server_ip_idx = 0;

// uint8_t server_port[5] = {'\0'};
// uint8_t server_port_idx = 0;

uint8_t passwd[20] = "95898063";
uint8_t passwd_idx = 8;

uint8_t server_ip[16] = "192.168.31.56";
uint8_t server_ip_idx = 113;

uint8_t server_port[6] = "43200";
uint8_t server_port_idx = 5;

uint8_t wifi_login_enter_flag = SET_PORT;

uint8_t wifi_log[MAX_WIFI_LOG_SIZE];
uint8_t wifi_log_clear_flag = 0;

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

void wifi_send_cmd(char *cmd)
{
    char recv = 0, send = 0;

    char recv_buff[100] = {'\0'};
    uint8_t idx = 0;
    uart_send_data(UART_WIFI_NUM, cmd, strlen(cmd));
    sleep(WIFI_TIME_OUT);
}

void server_connect()
{
    uint8_t wifi_msg[70] = {'\0'};
    //TODO:这里设置为一个函数，函数返回成功后，connect_server置位，并跳转到连接成功画面，并且wifi密码加密后保存到flash中，读取时解密
    //连接wifi
    sprintf(wifi_msg, "AT+CWJAP_DEF=\"%s\",\"%s\"\r\n", wifi_name, passwd);
    wifi_send_cmd(wifi_msg);
    sleep(10);
    wifi_log_clear_flag = 1;

    //设置透传模式
    memset(wifi_msg, '\0', sizeof(wifi_msg));
    //TODO:需要增加port和ip的输入控制
    sprintf(wifi_msg, "AT+SAVETRANSLINK=1,\"%s\",%s,\"TCP\"\r\n", server_ip, server_port);
    wifi_send_cmd(wifi_msg);
    sleep(5);
    wifi_log_clear_flag = 1;

    //重启wifi模块
    wifi_send_cmd("AT+RST\r\n");
    sleep(5);
    wifi_log_clear_flag = 1;

    //测试，等待透传的数据并打印
    sleep(10);
    printf("wifi_log = %s\n", wifi_log);
    wifi_log_clear_flag = 1;

    //TODO:如果连接成功
    connect_server = CONNECT_SERVER_OK;

    //FIXME:透传模式下，此时应当禁止WIFI Search和wifi connect，并且显示服务器连接成功
    //FIXME:此时应当再增加一个界面显示连接状态，代替原先的connect server界面，增加断开连接按钮，按钮按下后断开连接
    //并恢复原画面
    //TODO:需要增加服务器断开按钮
}

void server_disconnect()
{
    //TODO: 关闭透传模式，设置wifi连接
    //关闭透传模式
    wifi_send_cmd("+++");

    //设置wsp8266为wifi连接模式
    wifi_send_cmd("AT+CWMODE_DEF=1\r\n");
    connect_server = 0;
}