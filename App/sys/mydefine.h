#ifndef __MYDEFINE_H__
#define __MYDEFINE_H__

/* ========== HAL 库头文件 ========== */
#include "main.h"
#include "usart.h"
#include "gpio.h"

/* ========== C 标准头文件 ========== */
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>
#include <stdint.h>
#include <stdbool.h>

/* ========== 核心调度器头文件 ========== */
#include "scheduler.h"

/* ========== 驱动层头文件 ========== */
#include "key_driver.h"
#include 	"ebtn_driver.h"//easy-button驱动层头文件
#include	"uart_driver.h"//串口驱动层头文件

/* ========== 组件层头文件 ========== */
#include "ebtn.h"//easy-button组件库头文件
#include "ringbuffer.h"//环形缓存区组件库头文件
#include "event_queue.h"//消息队列组件库头文件

/* ========== 应用层头文件 ========== */
#include "system_assembly.h"//应用的入口装配文件，包含系统初始化和所有应用任务的注册。

/* ========== 其他 ========== */
#include "sys_key_monitor.h"//ebtn库移植测试文件

/* ========== 全局用户宏 ========== */

#endif

