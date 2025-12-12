#ifndef __UART_DRIVER_H__
#define __UART_DRIVER_H__

#include "mydefine.h"

extern UART_HandleTypeDef huart1;
int my_printf(UART_HandleTypeDef *huart, const char *format, ...);//串口广播函数
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size);//串口接收事件回调函数

#endif
