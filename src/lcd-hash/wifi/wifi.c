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
uint8_t server_ip_idx = 13;

uint8_t server_port[6] = "43200";
uint8_t server_port_idx = 5;

uint8_t wifi_login_enter_flag = SET_PORT;

uint8_t wifi_log[MAX_WIFI_LOG_SIZE];
uint8_t wifi_log_clear_flag = 0;

int8_t *GetNext(char *match)
{
    int *pNext = NULL;
    pNext = (int *)malloc(sizeof(int) * strlen(match)); //手动申请空间
    pNext[0] = 0;                                       //子串的第一个字符的Next一定为0
    int i = 1;
    int j = i - 1;
    while(i < strlen(match))
    {
        if(match[i] == match[pNext[j]])
        { //如果与前面的字符相等
            pNext[i] = pNext[j] + 1;
            i++;
            j = i - 1;
        } else
        { //不相等
            if(pNext[j] == 0)
            { //前一个next值是0
                pNext[i] = 0;
                i++;
                j = i - 1;
            } else
            { //前一个next值非0
                j = pNext[j] - 1;
            }
        }
    }
    return pNext; //返回Next数组
}

int8_t KMP(char *src, char *match)
{
    if(src == NULL || match == NULL)
        return -1;
    //获得next数组
    int *pNext = NULL;
    pNext = GetNext(match); //匹配
    int i, j;
    i = 0;
    j = 0;
    while(i < strlen(src) && j < strlen(match))
    { //判断主串或子串没到最后位置
        if(src[i] == match[j])
        { //如果主串与子串所对应位置相等
            i++;
            j++;
        } else
        { //不相等则匹配（子）串跳转
            if(j == 0)
            {
                i++;
            } else
            {
                j = pNext[j - 1];
            }
        }
    }
    //检测
    if(j == strlen(match))
    {                 //如果子串匹配到了最后
        return i - j; //返回子串在主串中首次出现的位置
    } else
    { //如果主串到了最后匹配失败
        return -1;
    }
}

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

uint8_t server_connect()
{
    uint8_t wifi_msg[70] = {'\0'};
    //TODO:这里设置为一个函数，函数返回成功后，connect_server置位，并跳转到连接成功画面，并且wifi密码加密后保存到flash中，读取时解密

    //连接wifi
    sprintf(wifi_msg, "AT+CWJAP_DEF=\"%s\",\"%s\"\r\n", wifi_name, passwd);
    wifi_send_cmd(wifi_msg);
    draw_process_bar("Login",8);

    if(KMP(wifi_log, "OK") < 0)
    {
        wifi_log_clear_flag = 1;
        draw_error_page("Wifi connect Error!");
        return ERROR_PAGE;
    } else
    {
        wifi_log_clear_flag = 1;

        //设置透传模式
        memset(wifi_msg, '\0', sizeof(wifi_msg));
        sprintf(wifi_msg, "AT+SAVETRANSLINK=1,\"%s\",%s,\"TCP\"\r\n", server_ip, server_port);
        wifi_send_cmd(wifi_msg);
            draw_process_bar("Connect server",4);


        if(KMP(wifi_log, "OK") < 0)
        {
            wifi_log_clear_flag = 1;
            draw_error_page("Server connect Error.");
            return ERROR_PAGE;
        } else
        {
            wifi_log_clear_flag = 1;
            //重启wifi模块
            wifi_send_cmd("AT+RST\r\n");
            draw_process_bar("Restart Wifi",4);
            wifi_log_clear_flag = 1;

            connect_server = CONNECT_SERVER_OK;
            draw_connect_server_page();
            return CONNECT_SERVER_PAGE;
        }
    }
    return CONNECT_SERVER_PAGE;
}

void server_disconnect()
{
    //关闭透传模式
    wifi_send_cmd("+++");

    //设置wsp8266为wifi连接模式
    wifi_send_cmd("AT+CWMODE_DEF=1\r\n");
    wifi_log_clear_flag = 1;
    connect_server = 0;
}