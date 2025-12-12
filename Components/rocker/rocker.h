#ifndef __ROCKER_H__
#define __ROCKER_H__

#include "mydefine.h"

// =============================================================================
// 摇杆组件 - A2级别（完整功能）
// 功能：校准、死区、滤波、范围映射、方向检测
// =============================================================================

// -----------------------------------------------------------------------------
// 1. 宏定义
// -----------------------------------------------------------------------------

/** 默认死区半径（原始ADC值，0-4095）- 增大防止ADC噪声误触发 */
#define ROCKER_DEFAULT_DEADZONE 500

/** 默认滤波采样数（1 = 不滤波） */
#define ROCKER_DEFAULT_FILTER_SIZE 4

/** 默认输出范围 */
#define ROCKER_DEFAULT_OUTPUT_MIN (-100)
#define ROCKER_DEFAULT_OUTPUT_MAX (100)

/** ADC参考值 */
#define ROCKER_ADC_MAX 4095
#define ROCKER_ADC_CENTER 2048

// -----------------------------------------------------------------------------
// 1.1 事件队列对接定义
// -----------------------------------------------------------------------------

/** 摇杆事件源ID（避免与按键ID冲突，使用0x0100起始） */
#define ROCKER_SOURCE_ID 0x0100

/** 摇杆事件类型 */
typedef enum
{
    ROCKER_EVT_NONE = 0,  /*!< 无事件 */
    ROCKER_EVT_DIR_ENTER, /*!< 进入某个方向（从中心或其他方向切换） */
    ROCKER_EVT_DIR_LEAVE, /*!< 离开某个方向（回到中心） */
    ROCKER_EVT_DIR_HOLD,  /*!< 持续保持某个方向（可选，用于连续触发） */
} rocker_event_type_t;

/**
 * @brief 摇杆事件数据打包格式（存入app_event_t.data）
 * 低8位: 方向 (rocker_direction_t)
 * 次8位: 幅度 (0-100)
 * 高16位: 保留
 */
#define ROCKER_EVT_PACK_DATA(dir, mag) ((uint32_t)(dir) | ((uint32_t)(mag) << 8))
#define ROCKER_EVT_UNPACK_DIR(data) ((rocker_direction_t)((data) & 0xFF))
#define ROCKER_EVT_UNPACK_MAG(data) ((uint8_t)(((data) >> 8) & 0xFF))

// -----------------------------------------------------------------------------
// 2. 类型定义
// -----------------------------------------------------------------------------

/**
 * @brief 摇杆方向枚举（8方向 + 中心）
 */
typedef enum
{
    ROCKER_DIR_CENTER = 0, /*!< 中心位置（在死区内） */
    ROCKER_DIR_UP,         /*!< 上（0度） */
    ROCKER_DIR_UP_RIGHT,   /*!< 右上（45度） */
    ROCKER_DIR_RIGHT,      /*!< 右（90度） */
    ROCKER_DIR_DOWN_RIGHT, /*!< 右下（135度） */
    ROCKER_DIR_DOWN,       /*!< 下（180度） */
    ROCKER_DIR_DOWN_LEFT,  /*!< 左下（225度） */
    ROCKER_DIR_LEFT,       /*!< 左（270度） */
    ROCKER_DIR_UP_LEFT     /*!< 左上（315度） */
} rocker_direction_t;

/**
 * @brief 摇杆配置结构体
 */
typedef struct
{
    uint16_t deadzone;   /*!< 死区半径，原始ADC值（0-4095） */
    uint8_t filter_size; /*!< 滑动平均滤波采样数（1-16） */
    int16_t output_min;  /*!< 输出最小值（如 -100） */
    int16_t output_max;  /*!< 输出最大值（如 +100） */
} rocker_config_t;

/**
 * @brief 摇杆校准数据
 */
typedef struct
{
    uint16_t center_x;  /*!< X轴中心值 */
    uint16_t center_y;  /*!< Y轴中心值 */
    uint16_t min_x;     /*!< X轴最小值 */
    uint16_t max_x;     /*!< X轴最大值 */
    uint16_t min_y;     /*!< Y轴最小值 */
    uint16_t max_y;     /*!< Y轴最大值 */
    bool is_calibrated; /*!< 校准状态标志 */
} rocker_calibration_t;

/**
 * @brief 处理后的摇杆状态
 */
typedef struct
{
    int16_t x;                    /*!< 处理后的X值（output_min ~ output_max） */
    int16_t y;                    /*!< 处理后的Y值（output_min ~ output_max） */
    uint16_t raw_x;               /*!< 原始X轴ADC值（0-4095） */
    uint16_t raw_y;               /*!< 原始Y轴ADC值（0-4095） */
    rocker_direction_t direction; /*!< 8方向 */
    uint8_t magnitude;            /*!< 偏移幅度百分比（0-100） */
    bool in_deadzone;             /*!< 摇杆是否在死区内 */
} rocker_state_t;

// -----------------------------------------------------------------------------
// 3. API声明
// -----------------------------------------------------------------------------

/**
 * @brief 初始化摇杆组件
 * @param config: 配置结构体指针，传NULL使用默认配置
 */
void rocker_init(const rocker_config_t *config);

/**
 * @brief 校准摇杆中心点
 * 在摇杆处于静止位置时调用
 * @param raw_x: 当前原始X值
 * @param raw_y: 当前原始Y值
 */
void rocker_calibrate_center(uint16_t raw_x, uint16_t raw_y);

/**
 * @brief 更新范围校准数据
 * 用户转动摇杆到各个极限位置时持续调用
 * @param raw_x: 当前原始X值
 * @param raw_y: 当前原始Y值
 */
void rocker_calibrate_range(uint16_t raw_x, uint16_t raw_y);

/**
 * @brief 完成范围校准
 * 用户完成摇杆极限位置转动后调用
 */
void rocker_calibrate_finish(void);

/**
 * @brief 处理原始摇杆输入并更新内部状态
 * 此函数应用校准、死区、滤波和映射
 * @param raw_x: 原始X轴ADC值（0-4095）
 * @param raw_y: 原始Y轴ADC值（0-4095）
 */
void rocker_update(uint16_t raw_x, uint16_t raw_y);

/**
 * @brief 获取当前处理后的摇杆状态
 * @return rocker_state_t: 包含所有处理后数值的状态结构体
 */
rocker_state_t rocker_get_state(void);

/**
 * @brief 获取当前校准数据
 * @return rocker_calibration_t: 当前校准值
 */
rocker_calibration_t rocker_get_calibration(void);

/**
 * @brief 手动设置校准数据
 * @param cal: 校准数据结构体指针
 */
void rocker_set_calibration(const rocker_calibration_t *cal);

/**
 * @brief 运行时更新配置
 * @param config: 新配置结构体指针
 */
void rocker_set_config(const rocker_config_t *config);

/**
 * @brief 获取方向名称字符串（用于调试）
 * @param dir: 方向枚举值
 * @return const char*: 方向名称字符串
 */
const char *rocker_get_direction_name(rocker_direction_t dir);

// -----------------------------------------------------------------------------
// 4. 事件队列对接API
// -----------------------------------------------------------------------------

/**
 * @brief 启用事件队列推送功能
 * 调用后，rocker_update() 会自动检测方向变化并推送事件到队列
 * @param enable: true=启用，false=禁用
 */
void rocker_event_enable(bool enable);

/**
 * @brief 启用方向保持事件（ROCKER_EVT_DIR_HOLD）
 * 启用后，摇杆保持在某个方向时会持续推送HOLD事件
 * @param enable: true=启用，false=禁用
 * @param interval_ms: 两次HOLD事件的最小间隔（毫秒），0=每次update都推送
 */
void rocker_event_hold_enable(bool enable, uint32_t interval_ms);

/**
 * @brief 手动检查并推送事件（如果不想在update中自动推送）
 * @return bool: 如果推送了事件返回true
 */
bool rocker_event_process(void);

#endif // __ROCKER_H__
