#include "scheduler.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

static MLFQ_TCB_t g_taskTable[TICK_PROFILER_MAX_TASKS];

static uint32_t getQuantumForLevel(MLFQ_QueueLevel_t level)
{
    switch(level) {
        case MLFQ_QUEUE_HIGH:   return MLFQ_TIME_SLICE_HIGH;
        case MLFQ_QUEUE_MEDIUM: return MLFQ_TIME_SLICE_MEDIUM;
        case MLFQ_QUEUE_LOW:    return MLFQ_TIME_SLICE_LOW;
        default:                return MLFQ_TIME_SLICE_LOW;
    }
}

void updateTaskPriority(TaskHandle_t task, MLFQ_QueueLevel_t newLevel)
{
    for (uint8_t table_index = 0; table_index < TICK_PROFILER_MAX_TASKS; table_index++)
    {
        if (g_taskTable[table_index].task_handle == task)
        {
            g_taskTable[table_index].task_level = newLevel;

            vTaskPrioritySet(task, MLFQ_TO_RTOS_LEVEL_SETTER(newLevel));

            setTaskQuantum(task, getQuantumForLevel(newLevel));
            resetTaskRuntime(task);
        }
    }
}


void initScheduler(void)
{
    tickProfilerInit();

    for (uint8_t table_index = 0; table_index < TICK_PROFILER_MAX_TASKS; table_index++)
    {
        g_taskTable[table_index].task_handle = NULL;
        g_taskTable[table_index].task_level = MLFQ_QUEUE_HIGH;
        g_taskTable[table_index].arrival_tick = 0;
    }
}


void registerTask(TaskHandle_t taskHandle)
{
    for (uint8_t table_index = 0; table_index < TICK_PROFILER_MAX_TASKS; table_index++)
    {
        if (g_taskTable[table_index].task_handle == NULL)
        {
            if (!setupTaskStats(taskHandle))
            {
                break;
            }
            else
            {
                g_taskTable[table_index].task_handle = taskHandle;
                g_taskTable[table_index].task_level = MLFQ_QUEUE_HIGH;
                g_taskTable[table_index].arrival_tick = xTaskGetTickCount();

                vTaskPrioritySet(taskHandle,
                                 MLFQ_TO_RTOS_LEVEL_SETTER(MLFQ_QUEUE_HIGH));
                setTaskQuantum(taskHandle, MLFQ_TIME_SLICE_HIGH);

                break;
            }
        }
    }
}


void checkForDemotion(uint8_t table_index)
{
    TaskHandle_t task = g_taskTable[table_index].task_handle;
    MLFQ_QueueLevel_t currentLevel = g_taskTable[table_index].task_level;

    if(currentLevel < MLFQ_QUEUE_LOW)
    {
        updateTaskPriority(task, (MLFQ_QueueLevel_t)(MLFQ_TO_RTOS_LEVEL_SETTER(currentLevel+1)));
    }
    else
    {
        updateTaskPriority(task,(MLFQ_QueueLevel_t)(MLFQ_QUEUE_LOW));
    }
}


void performGlobalBoost(void)
{
    for (uint8_t table_index = 0; table_index < TICK_PROFILER_MAX_TASKS; table_index++)
    {
        if (g_taskTable[table_index].task_info.task != NULL)
        {
            updateTaskPriority(g_taskTable[table_index].task_handle,
                               MLFQ_QUEUE_HIGH);
        }
    }
}


void promoteInteractiveTask(TaskHandle_t task)
{
    for (uint8_t table_index = 0; table_index < TICK_PROFILER_MAX_TASKS; table_index++)
    {
        if (g_taskTable[table_index].task_info.task == task)
        {
            if (g_taskTable[table_index].task_level != MLFQ_QUEUE_HIGH)
            {
                updateTaskPriority(
                    g_taskTable[table_index].task_handle,
                    (MLFQ_QueueLevel_t)(g_taskTable[table_index].task_level - 1));
            }
        }
    }
}


void schedulerTask(void *pvParameters)
{
    tickProfilerSetSchedulerTaskHandle(xTaskGetCurrentTaskHandle());
    QueueHandle_t expiredQueue = tickProfilerGetExpiredQueue();
    TaskHandle_t xExpiredHandle = NULL;
    TickType_t xLastBoostTime = xTaskGetTickCount();
    const TickType_t xBoostPeriod = pdMS_TO_TICKS(MLFQ_BOOST_PERIOD_MS);

    for (;;)
    {
        /* Wait for Notification from Profiler (Hook) */
        ulTaskNotifyTake(pdTRUE, 1); // Optional: Check notification

        /* Check Expired Queue */
        while (xQueueReceive(expiredQueue, &xExpiredHandle, 0) == pdTRUE)
        {
            for (uint8_t i = 0; i < TICK_PROFILER_MAX_TASKS; i++)
            {
                if (g_taskTable[i].taskh_handle == xExpiredHandle)
                {
                    checkForDemotion(i); /* Pass index, not copy */
                    break;
                }
            }
        }

        /* Check Global Boost */
        TickType_t xNow = xTaskGetTickCount();
        if ((xNow - xLastBoostTime) >= xBoostPeriod)
        {
            performGlobalBoost();
            xLastBoostTime = xNow;
        }

        /* Yield to let others run if nothing happened */
        vTaskDelay(1);
}


    bool schedulerGetTaskStats(uint32_t index, MLFQ_Task_Profiler_t *output)
    {
        if (index >= TICK_PROFILER_MAX_TASKS || g_taskTable[index].task_handle == NULL)
        {
            return false;
        }

        output->task_info.task  = g_taskTable[index].task_handle;
        output->task_level      = g_taskTable[index].task_level;
        output->arrival_tick    = g_taskTable[index].arrival_tick;

        /* 2. Copy info from Profiler (Live Stats) */
        /* Note: We call the Profiler directly here so the Logger doesn't have to */
        output->task_info.run_ticks     = getTaskRuntime(output->task_info.task);
        output->task_info.quantum_ticks = getQuantumForLevel(output->task_level);

        return true;
    }

