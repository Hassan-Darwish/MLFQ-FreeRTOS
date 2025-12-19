#include "scheduler.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>


static MLFQ_Task_Profiler_t g_taskTable[TICK_PROFILER_MAX_TASKS]

void initScheduler(void)
{
    tickProfilerInit();

    for(uint8_t table_index = 0; table_index < TICK_PROFILER_MAX_TASKS; table_index++)
    {
        g_taskTable[table_index].task_info.task = NULL;
        g_taskTable[table_index].task_level = MLFQ_QUEUE_HIGH;

    }
}

void schedulerTask(void* pvParam)
{

}

void registerTask(TaskHandle_t taskHandle)
{
    for(uint8_t table_index = 0; table_index < TICK_PROFILER_MAX_TASKS; table_index++)
    {
        if(g_taskTable[table_index].task_info.task == NULL)
        {
            if(!setupTaskStats(taskHandle))
            {
                break;
            }
            else
            {
                g_taskTable[table_index].task_info.task = taskHandle;

                g_taskTable[table_index].task_level = MLFQ_QUEUE_HIGH;

                vTaskPrioritySet(taskHandle, MLFQ_TO_RTOS_LEVEL_SETTER(MLFQ_QUEUE_HIGH));

                break;
            }
        }
    }
}

void updateTaskPriority(TaskHandle_t task, MLFQ_QueueLevel_t newLevel)
{
    for(uint8_t table_index = 0; table_index < TICK_PROFILER_MAX_TASKS; table_index++)
    {
        if(g_taskTable[table_index].task_info.task == task)
        {
            g_taskTable[table_index].task_level = newLevel;
            vTaskPrioritySet(task, MLFQ_TO_RTOS_LEVEL_SETTER(newLevel));
        }
    }
}

void checkForDemotion(MLFQ_Task_Profiler_t status)
{
    for(uint8_t table_index = 0; table_index < TICK_PROFILER_MAX_TASKS; table_index++)
    {
        if(g_taskTable[table_index].task_info.task == status.task_info.task)
        {
            if(g_taskTable[table_index].task_level != MLFQ_QUEUE_LOW)
            {
                updateTaskPriority(g_taskTable[table_index].task_info.task,
                                   g_taskTable[table_index].task_level + 1)
            }
        }
    }
}

void performGlobalBoost(void)
{
    for(uint8_t table_index = 0; table_index < TICK_PROFILER_MAX_TASKS; table_index++)
    {
        if(g_taskTable[table_index].task_info.task != NULL)
        {
            updateTaskPriority(g_taskTable[table_index].task_info.task, MLFQ_QUEUE_HIGH);
        }
    }
}

void promoteInteractiveTask(TaskHandle_t task)
{
    for(uint8_t table_index = 0; table_index < TICK_PROFILER_MAX_TASKS; table_index++)
    {
        if(g_taskTable[table_index].task_info.task == task)
        {
            if(g_taskTable[table_index].task_level != MLFQ_QUEUE_HIGH)
            {
                updateTaskPriority(g_taskTable[table_index].task_info.task,
                                   g_taskTable[table_index].task_level - 1);
            }
        }
    }
}
