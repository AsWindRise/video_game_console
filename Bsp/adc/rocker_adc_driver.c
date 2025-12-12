#include "rocker_adc_driver.h"
// 引入必要的 HAL 句柄声明
#include "tim.h"
#include "adc.h"

// -----------------------------------------------------------------------------
// 1. 外部硬件句柄声明
// -----------------------------------------------------------------------------
extern TIM_HandleTypeDef htim3;
extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc2;

// -----------------------------------------------------------------------------
// 2. 静态数据存储定义
// -----------------------------------------------------------------------------
// 必须与 rocker_adc_driver.h 中的宏定义一致
#define ROCKER_DMA_BUFFER_SIZE 128

// DMA 缓冲区：存储 X/Y 轴原始数据
static uint32_t s_rocker_dma_buffer[ROCKER_DMA_BUFFER_SIZE];

// 最新数据存储：使用 rocker_data_t (已在头文件中定义)
static rocker_data_t s_latest_rocker_data;

// -----------------------------------------------------------------------------
// 3. 驱动 API 实现
// -----------------------------------------------------------------------------

/**
 * @brief 摇杆 ADC 驱动初始化函数。
 */
void rocker_adc_driver_init(void)
{
    // 步骤 1: 启动触发源 (TIM3)
    HAL_TIM_Base_Start(&htim3);

    // 步骤 2: 先启动 Slave ADC (ADC2)
    // 在双ADC同步模式下，必须先启动从ADC，否则从ADC不会进行转换！
    HAL_ADC_Start(&hadc2);

    // 步骤 3: 启动 Master ADC (ADC1) 的多模式DMA传输
    // 使用 HAL_ADCEx_MultiModeStart_DMA 从CDR寄存器读取组合数据（高16位=ADC2，低16位=ADC1）
    HAL_ADCEx_MultiModeStart_DMA(&hadc1, s_rocker_dma_buffer, ROCKER_DMA_BUFFER_SIZE);
}

/**
 * @brief 获取最新的摇杆原始 ADC 值。
 */
rocker_data_t rocker_adc_get_raw_value(void)
{
    // 1. 获取 DMA 剩余传输次数 (Remaining Transfers)
    // 修正：使用正确的 DMA 宏替换未声明的函数 HAL_DMA_GetCounter
    uint16_t remaining_transfers = (uint16_t)__HAL_DMA_GET_COUNTER(hadc1.DMA_Handle);

    // 2. 计算 DMA 下一个要写入的数组索引 (Next Write Index)
    uint16_t next_write_index = ROCKER_DMA_BUFFER_SIZE - remaining_transfers;

    // 3. 计算最新完成数据的索引 (Read Index)
    uint16_t read_index;

    // 处理数组头部的回绕情况 (wrap-around)
    if (next_write_index >= 1)
    {
        // 正常情况：读取最新的一个数据
        read_index = next_write_index - 1;
    }
    else
    {
        // 回绕情况：最新数据在缓冲区的末尾
        read_index = ROCKER_DMA_BUFFER_SIZE - 1;
    }

    // 4. 从32位组合数据中提取 X/Y 轴数据
    // 在双ADC模式(ADC_DMAACCESSMODE_2)下，DMA从CDR寄存器读取32位数据：
    // 低16位 (bits 0-15)  = ADC1数据 (X轴)
    // 高16位 (bits 16-31) = ADC2数据 (Y轴)
    uint32_t combined_data = s_rocker_dma_buffer[read_index];
    s_latest_rocker_data.x_raw_value = combined_data & 0xFFFF;         // 提取低16位 (X轴)
    s_latest_rocker_data.y_raw_value = (combined_data >> 16) & 0xFFFF; // 提取高16位 (Y轴)

    // 5. 返回最新数据
    return s_latest_rocker_data;
}
