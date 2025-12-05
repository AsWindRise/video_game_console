#include "rocker.h"
#include "event_queue.h" // 事件队列组件
#include <string.h>

// =============================================================================
// 摇杆组件 - A2级别实现
// =============================================================================

// -----------------------------------------------------------------------------
// 1. 私有宏定义
// -----------------------------------------------------------------------------

/** 滤波缓冲区最大大小 */
#define FILTER_BUFFER_MAX 16

// -----------------------------------------------------------------------------
// 2. 私有数据
// -----------------------------------------------------------------------------

/** 当前配置 */
static rocker_config_t s_config;

/** 校准数据 */
static rocker_calibration_t s_calibration;

/** 当前状态 */
static rocker_state_t s_state;

/** 滤波缓冲区 */
static uint16_t s_filter_buf_x[FILTER_BUFFER_MAX];
static uint16_t s_filter_buf_y[FILTER_BUFFER_MAX];
static uint8_t s_filter_index;
static bool s_filter_filled;

/** 范围校准临时数据 */
static uint16_t s_cal_temp_min_x;
static uint16_t s_cal_temp_max_x;
static uint16_t s_cal_temp_min_y;
static uint16_t s_cal_temp_max_y;
static bool s_cal_range_active;

/** 事件队列对接相关 */
static bool s_event_enabled;                // 事件推送是否启用
static bool s_event_hold_enabled;           // HOLD事件是否启用
static uint32_t s_event_hold_interval;      // HOLD事件间隔（毫秒）
static uint32_t s_event_hold_last_tick;     // 上次HOLD事件时间戳
static rocker_direction_t s_prev_direction; // 上一次的方向（用于检测变化）

// -----------------------------------------------------------------------------
// 3. 私有函数声明
// -----------------------------------------------------------------------------

static uint16_t filter_apply(uint16_t new_x, uint16_t new_y, uint16_t *out_x, uint16_t *out_y);
static int16_t map_value(int32_t value, int32_t in_min, int32_t in_max, int32_t out_min, int32_t out_max);
static void detect_direction(int16_t x, int16_t y, bool in_deadzone);
static uint32_t fast_sqrt(uint32_t n);

// -----------------------------------------------------------------------------
// 4. API实现
// -----------------------------------------------------------------------------

/**
 * @brief 初始化摇杆组件
 */
void rocker_init(const rocker_config_t *config)
{
    // 应用配置或使用默认值
    if (config != NULL)
    {
        s_config = *config;
    }
    else
    {
        s_config.deadzone = ROCKER_DEFAULT_DEADZONE;
        s_config.filter_size = ROCKER_DEFAULT_FILTER_SIZE;
        s_config.output_min = ROCKER_DEFAULT_OUTPUT_MIN;
        s_config.output_max = ROCKER_DEFAULT_OUTPUT_MAX;
    }

    // 限制滤波大小
    if (s_config.filter_size > FILTER_BUFFER_MAX)
    {
        s_config.filter_size = FILTER_BUFFER_MAX;
    }
    if (s_config.filter_size < 1)
    {
        s_config.filter_size = 1;
    }

    // 初始化校准数据（使用默认中心值）
    s_calibration.center_x = ROCKER_ADC_CENTER;
    s_calibration.center_y = ROCKER_ADC_CENTER;
    s_calibration.min_x = 0;
    s_calibration.max_x = ROCKER_ADC_MAX;
    s_calibration.min_y = 0;
    s_calibration.max_y = ROCKER_ADC_MAX;
    s_calibration.is_calibrated = false;

    // 清空状态
    memset(&s_state, 0, sizeof(s_state));

    // 清空滤波缓冲区
    memset(s_filter_buf_x, 0, sizeof(s_filter_buf_x));
    memset(s_filter_buf_y, 0, sizeof(s_filter_buf_y));
    s_filter_index = 0;
    s_filter_filled = false;

    // 清空范围校准临时数据
    s_cal_range_active = false;

    // 初始化事件相关变量
    s_event_enabled = false;
    s_event_hold_enabled = false;
    s_event_hold_interval = 0;
    s_event_hold_last_tick = 0;
    s_prev_direction = ROCKER_DIR_CENTER;
}

/**
 * @brief 校准摇杆中心点
 */
void rocker_calibrate_center(uint16_t raw_x, uint16_t raw_y)
{
    s_calibration.center_x = raw_x;
    s_calibration.center_y = raw_y;
}

/**
 * @brief 更新范围校准数据
 */
void rocker_calibrate_range(uint16_t raw_x, uint16_t raw_y)
{
    if (!s_cal_range_active)
    {
        // 首次调用，初始化临时数据
        s_cal_temp_min_x = raw_x;
        s_cal_temp_max_x = raw_x;
        s_cal_temp_min_y = raw_y;
        s_cal_temp_max_y = raw_y;
        s_cal_range_active = true;
    }
    else
    {
        // 更新最小/最大值
        if (raw_x < s_cal_temp_min_x)
            s_cal_temp_min_x = raw_x;
        if (raw_x > s_cal_temp_max_x)
            s_cal_temp_max_x = raw_x;
        if (raw_y < s_cal_temp_min_y)
            s_cal_temp_min_y = raw_y;
        if (raw_y > s_cal_temp_max_y)
            s_cal_temp_max_y = raw_y;
    }
}

/**
 * @brief 完成范围校准
 */
void rocker_calibrate_finish(void)
{
    if (s_cal_range_active)
    {
        s_calibration.min_x = s_cal_temp_min_x;
        s_calibration.max_x = s_cal_temp_max_x;
        s_calibration.min_y = s_cal_temp_min_y;
        s_calibration.max_y = s_cal_temp_max_y;
        s_calibration.is_calibrated = true;
        s_cal_range_active = false;
    }
}

/**
 * @brief 处理原始摇杆输入并更新内部状态
 */
void rocker_update(uint16_t raw_x, uint16_t raw_y)
{
    uint16_t filtered_x, filtered_y;
    int32_t centered_x, centered_y;
    int32_t mapped_x, mapped_y;
    uint32_t distance_sq;
    uint32_t deadzone_sq;
    bool in_deadzone;

    // 保存原始值
    s_state.raw_x = raw_x;
    s_state.raw_y = raw_y;

    // 1. 滤波处理
    filter_apply(raw_x, raw_y, &filtered_x, &filtered_y);

    // 2. 中心点偏移（相对于校准中心）
    centered_x = (int32_t)filtered_x - (int32_t)s_calibration.center_x;
    centered_y = (int32_t)filtered_y - (int32_t)s_calibration.center_y;

    // 3. 死区检测（使用平方避免开方运算）
    distance_sq = (uint32_t)(centered_x * centered_x + centered_y * centered_y);
    deadzone_sq = (uint32_t)s_config.deadzone * s_config.deadzone;
    in_deadzone = (distance_sq <= deadzone_sq);
    s_state.in_deadzone = in_deadzone;

    if (in_deadzone)
    {
        // 在死区内，输出为0
        s_state.x = 0;
        s_state.y = 0;
        s_state.magnitude = 0;
    }
    else
    {
        // 4. 范围映射
        // X轴映射
        if (centered_x < 0)
        {
            // 负方向：从 min_x 到 center_x 映射到 output_min 到 0
            int32_t range_neg = (int32_t)s_calibration.center_x - (int32_t)s_calibration.min_x;
            if (range_neg > 0)
            {
                mapped_x = map_value(centered_x, -range_neg, 0, s_config.output_min, 0);
            }
            else
            {
                mapped_x = 0;
            }
        }
        else
        {
            // 正方向：从 center_x 到 max_x 映射到 0 到 output_max
            int32_t range_pos = (int32_t)s_calibration.max_x - (int32_t)s_calibration.center_x;
            if (range_pos > 0)
            {
                mapped_x = map_value(centered_x, 0, range_pos, 0, s_config.output_max);
            }
            else
            {
                mapped_x = 0;
            }
        }

        // Y轴映射（同上）
        if (centered_y < 0)
        {
            int32_t range_neg = (int32_t)s_calibration.center_y - (int32_t)s_calibration.min_y;
            if (range_neg > 0)
            {
                mapped_y = map_value(centered_y, -range_neg, 0, s_config.output_min, 0);
            }
            else
            {
                mapped_y = 0;
            }
        }
        else
        {
            int32_t range_pos = (int32_t)s_calibration.max_y - (int32_t)s_calibration.center_y;
            if (range_pos > 0)
            {
                mapped_y = map_value(centered_y, 0, range_pos, 0, s_config.output_max);
            }
            else
            {
                mapped_y = 0;
            }
        }

        // 限幅
        if (mapped_x < s_config.output_min)
            mapped_x = s_config.output_min;
        if (mapped_x > s_config.output_max)
            mapped_x = s_config.output_max;
        if (mapped_y < s_config.output_min)
            mapped_y = s_config.output_min;
        if (mapped_y > s_config.output_max)
            mapped_y = s_config.output_max;

        s_state.x = (int16_t)mapped_x;
        s_state.y = (int16_t)mapped_y;

        // 5. 计算幅度百分比
        // magnitude = sqrt(x^2 + y^2) / max_output * 100
        int32_t out_range = s_config.output_max; // 假设对称范围
        uint32_t mag_sq = (uint32_t)(mapped_x * mapped_x + mapped_y * mapped_y);
        uint32_t mag = fast_sqrt(mag_sq);
        uint32_t mag_percent = (mag * 100) / (uint32_t)out_range;
        if (mag_percent > 100)
            mag_percent = 100;
        s_state.magnitude = (uint8_t)mag_percent;
    }

    // 6. 方向检测
    detect_direction(s_state.x, s_state.y, in_deadzone);

    // 7. 自动事件推送（如果启用）
    if (s_event_enabled)
    {
        rocker_event_process();
    }
}

/**
 * @brief 获取当前处理后的摇杆状态
 */
rocker_state_t rocker_get_state(void)
{
    return s_state;
}

/**
 * @brief 获取当前校准数据
 */
rocker_calibration_t rocker_get_calibration(void)
{
    return s_calibration;
}

/**
 * @brief 手动设置校准数据
 */
void rocker_set_calibration(const rocker_calibration_t *cal)
{
    if (cal != NULL)
    {
        s_calibration = *cal;
    }
}

/**
 * @brief 运行时更新配置
 */
void rocker_set_config(const rocker_config_t *config)
{
    if (config != NULL)
    {
        s_config = *config;

        // 限制滤波大小
        if (s_config.filter_size > FILTER_BUFFER_MAX)
        {
            s_config.filter_size = FILTER_BUFFER_MAX;
        }
        if (s_config.filter_size < 1)
        {
            s_config.filter_size = 1;
        }
    }
}

/**
 * @brief 获取方向名称字符串
 */
const char *rocker_get_direction_name(rocker_direction_t dir)
{
    static const char *names[] = {
        "CENTER",
        "UP",
        "UP_RIGHT",
        "RIGHT",
        "DOWN_RIGHT",
        "DOWN",
        "DOWN_LEFT",
        "LEFT",
        "UP_LEFT"};

    if (dir <= ROCKER_DIR_UP_LEFT)
    {
        return names[dir];
    }
    return "UNKNOWN";
}

// -----------------------------------------------------------------------------
// 5. 私有函数实现
// -----------------------------------------------------------------------------

/**
 * @brief 滑动平均滤波
 */
static uint16_t filter_apply(uint16_t new_x, uint16_t new_y, uint16_t *out_x, uint16_t *out_y)
{
    uint32_t sum_x = 0, sum_y = 0;
    uint8_t count;

    // 存入缓冲区
    s_filter_buf_x[s_filter_index] = new_x;
    s_filter_buf_y[s_filter_index] = new_y;
    s_filter_index++;

    if (s_filter_index >= s_config.filter_size)
    {
        s_filter_index = 0;
        s_filter_filled = true;
    }

    // 计算有效采样数
    count = s_filter_filled ? s_config.filter_size : s_filter_index;
    if (count == 0)
        count = 1;

    // 计算平均值
    for (uint8_t i = 0; i < count; i++)
    {
        sum_x += s_filter_buf_x[i];
        sum_y += s_filter_buf_y[i];
    }

    *out_x = (uint16_t)(sum_x / count);
    *out_y = (uint16_t)(sum_y / count);

    return 0;
}

/**
 * @brief 数值映射（类似Arduino的map函数）
 */
static int16_t map_value(int32_t value, int32_t in_min, int32_t in_max, int32_t out_min, int32_t out_max)
{
    // 避免除零
    if (in_max == in_min)
    {
        return (int16_t)out_min;
    }

    return (int16_t)((value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min);
}

/**
 * @brief 方向检测（8方向）
 */
static void detect_direction(int16_t x, int16_t y, bool in_deadzone)
{
    if (in_deadzone)
    {
        s_state.direction = ROCKER_DIR_CENTER;
        return;
    }

    // 使用角度判断方向
    // 将坐标系分为8个扇区，每个扇区45度
    // 注意：Y轴正方向为下（ADC值增大），需要取反以匹配常规坐标系

    int16_t abs_x = (x >= 0) ? x : -x;
    int16_t abs_y = (y >= 0) ? y : -y;

    // 判断主方向
    // tan(22.5°) ≈ 0.414, 使用整数近似: abs_y * 1000 / abs_x 与 414 比较
    // tan(67.5°) ≈ 2.414, 使用整数近似: abs_y * 1000 / abs_x 与 2414 比较

    bool mostly_horizontal = (abs_x > abs_y * 2); // 水平为主
    bool mostly_vertical = (abs_y > abs_x * 2);   // 垂直为主

    if (mostly_horizontal)
    {
        // 主要是左右方向（ADC值：大=左，小=右）
        s_state.direction = (x > 0) ? ROCKER_DIR_LEFT : ROCKER_DIR_RIGHT;
    }
    else if (mostly_vertical)
    {
        // 主要是上下方向（ADC值：大=下，小=上）
        s_state.direction = (y > 0) ? ROCKER_DIR_UP : ROCKER_DIR_DOWN;
    }
    else
    {
        // 对角线方向（全部翻转）
        if (x > 0 && y < 0)
        {
            s_state.direction = ROCKER_DIR_DOWN_LEFT;
        }
        else if (x > 0 && y > 0)
        {
            s_state.direction = ROCKER_DIR_UP_LEFT;
        }
        else if (x < 0 && y > 0)
        {
            s_state.direction = ROCKER_DIR_UP_RIGHT;
        }
        else
        {
            s_state.direction = ROCKER_DIR_DOWN_RIGHT;
        }
    }
}

/**
 * @brief 快速整数平方根（牛顿迭代法）
 */
static uint32_t fast_sqrt(uint32_t n)
{
    if (n == 0)
        return 0;

    uint32_t x = n;
    uint32_t y = (x + 1) / 2;

    while (y < x)
    {
        x = y;
        y = (x + n / x) / 2;
    }

    return x;
}

// -----------------------------------------------------------------------------
// 6. 事件队列对接API实现
// -----------------------------------------------------------------------------

/**
 * @brief 推送事件到队列（内部辅助函数）
 */
static void push_rocker_event(rocker_event_type_t evt_type, rocker_direction_t dir, uint8_t mag)
{
    app_event_t evt;
    evt.source_id = ROCKER_SOURCE_ID;
    evt.event_type = (uint8_t)evt_type;
    evt.data = ROCKER_EVT_PACK_DATA(dir, mag);
    event_queue_push(evt);
}

/**
 * @brief 启用事件队列推送功能
 */
void rocker_event_enable(bool enable)
{
    s_event_enabled = enable;
    if (enable)
    {
        // 重置上一次方向状态
        s_prev_direction = s_state.direction;
    }
}

/**
 * @brief 启用方向保持事件
 */
void rocker_event_hold_enable(bool enable, uint32_t interval_ms)
{
    s_event_hold_enabled = enable;
    s_event_hold_interval = interval_ms;
    s_event_hold_last_tick = 0;
}

/**
 * @brief 手动检查并推送事件
 */
bool rocker_event_process(void)
{
    bool event_pushed = false;
    rocker_direction_t curr_dir = s_state.direction;
    uint8_t curr_mag = s_state.magnitude;

    // 检测方向是否发生变化
    if (curr_dir != s_prev_direction)
    {
        // 如果之前不在中心，推送LEAVE事件
        if (s_prev_direction != ROCKER_DIR_CENTER)
        {
            push_rocker_event(ROCKER_EVT_DIR_LEAVE, s_prev_direction, 0);
            event_pushed = true;
        }

        // 如果现在不在中心，推送ENTER事件
        if (curr_dir != ROCKER_DIR_CENTER)
        {
            push_rocker_event(ROCKER_EVT_DIR_ENTER, curr_dir, curr_mag);
            event_pushed = true;
        }

        // 更新上一次方向
        s_prev_direction = curr_dir;

        // 重置HOLD计时器
        s_event_hold_last_tick = HAL_GetTick();
    }
    else if (s_event_hold_enabled && curr_dir != ROCKER_DIR_CENTER)
    {
        // 方向没变化，检查是否需要推送HOLD事件
        uint32_t now = HAL_GetTick();
        if (s_event_hold_interval == 0 ||
            (now - s_event_hold_last_tick) >= s_event_hold_interval)
        {
            push_rocker_event(ROCKER_EVT_DIR_HOLD, curr_dir, curr_mag);
            s_event_hold_last_tick = now;
            event_pushed = true;
        }
    }

    return event_pushed;
}
