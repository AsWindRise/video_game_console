#include "scheduler.h" 

// 定义调度器支持的最大任务数量
#define MAX_TASKS 20 

// 任务结构体定义
typedef struct {
    void (*task_func)(void); // 任务函数指针
    uint32_t rate_ms;        // 任务的执行周期（毫秒）
    uint32_t last_run;       // 任务上次运行时的系统时间（毫秒）
} task_t;


// 静态任务数组，存放所有注册的任务
static task_t scheduler_task[MAX_TASKS];
// 当前已注册的任务数量
static uint8_t task_num = 0;


/**
 * @brief 调度器初始化函数。
 * 准备调度器运行环境。
 */
void scheduler_init(void)
{
    // 将任务计数器清零，为后续任务注册做准备
    task_num = 0;
}

/**
 * @brief 向调度器添加一个任务。
 * 该函数由应用层调用，用于注册所有需要周执行的任务。
 * @param task_func: 任务函数指针。
 * @param rate_ms: 任务执行周期（毫秒）。
 * @return bool: 成功返回 true，失败返回 false。
 */
bool scheduler_add_task(void (*task_func)(void), uint32_t rate_ms)
{
    // 检查是否有足够的空间、函数指针是否有效以及周期是否大于零
    if (task_num >= MAX_TASKS || task_func == NULL || rate_ms == 0)
    {
        return false;
    }

    // 填充任务信息
    scheduler_task[task_num].task_func = task_func;
    scheduler_task[task_num].rate_ms = rate_ms;
    // 使用当前系统时间初始化上次运行时间
    scheduler_task[task_num].last_run = HAL_GetTick(); 
    task_num++;

    return true;
}


/**
 * @brief 调度器运行函数。
 * 遍历任务数组，检查任务是否达到执行周期并运行。
 */
void scheduler_run(void)
{
    // 遍历所有已注册的任务
    for (uint8_t i = 0; i < task_num; i++)
    {
        uint32_t now_time = HAL_GetTick();

        // 使用 "上次运行时间 + 周期" 与 "当前时间" 进行比较，判断是否应该运行
        if (now_time >= scheduler_task[i].rate_ms + scheduler_task[i].last_run)
        {
            // 更新上次运行时间为当前时间
            scheduler_task[i].last_run = now_time;

            // 执行任务函数
            scheduler_task[i].task_func();
        }
    }
}
