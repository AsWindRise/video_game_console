#include "uart_driver.h"

/**
 * @brief 格式化数据并发送到指定串口。
 * 职责：实现类似 printf 的功能，通过 HAL 库的 UART 接口发送数据。
 * @param huart: 指向要发送数据的 UART 句柄。
 * @param format: 格式化字符串。
 * @param ...: 可变参数列表。
 * @return int: 成功发送的字符数。
 */
int my_printf(UART_HandleTypeDef *huart, const char *format, ...)
{
	char buffer[512]; // 临时存储格式化后的字符串
	va_list arg;      // 处理可变参数
	int len;          // 最终字符串长度

	// 1. 启动可变参数处理
	va_start(arg, format);
    
	// 2. 安全地格式化字符串到 buffer (使用 vsnprintf 防止缓冲区溢出)
	len = vsnprintf(buffer, sizeof(buffer), format, arg);
    
	// 3. 结束可变参数处理
	va_end(arg);

	// 4. 通过 HAL 库发送 buffer 中的内容 (使用阻塞式发送，超时时间 0xFF)
	HAL_UART_Transmit(huart, (uint8_t *)buffer, (uint16_t)len, 0xFF);
    
	return len; // 返回实际发送的长度
}
