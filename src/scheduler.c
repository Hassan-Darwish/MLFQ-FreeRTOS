/******************************************************************************
 *  MODULE NAME  : MLFQ Scheduler
 *  FILE         : scheduler.c
 *  DESCRIPTION  : Implements a Multi-Level Feedback Queue (MLFQ) scheduler
 *                 on top of FreeRTOS, handling task registration, promotion,
 *                 demotion, and periodic priority boosting.
 *  AUTHOR       : Hassan Darwish
 ******************************************************************************/

/******************************************************************************
 *  INCLUDES
 ******************************************************************************/
#include "scheduler.h"
#include "metrics_logger.h"
#include "drivers.h"
#include <stdlib.h>

/******************************************************************************
 *  STATIC (PRIVATE) VARIABLES
 ******************************************************************************/
/* Task table holding MLFQ-related metadata for all registered tasks */
static MLFQ_TCB_t g_taskTable[TICK_PROFILER_MAX_TASKS];

/******************************************************************************
 *  STATIC (PRIVATE) FUNCTION DEFINITIONS
 *****************************************************************************-0---*/

/*
 * Description : Returns the time quantum assigned to a specific MLFQ level.
 *               Higher priority queues receive shorter response-focused
 *               time slices, while lower levels get longer CPU bursts.
 */
static uint32_t getQuantumForLevel(MLFQ_QueueLevel_t level)
{
    switch(level) {
        case MLFQ_QUEUE_HIGH:   return MLFQ_TIME_SLICE_HIGH;
        case MLFQ_QUEUE_MEDIUM: return MLFQ_TIME_SLICE_MEDIUM;
        case MLFQ_QUEUE_LOW:    return MLFQ_TIME_SLICE_LOW;
        default:                return MLFQ_TIME_SLICE_LOW;
    }
}

/******************************************************************************
 *  FUNCTION DEFINITIONS
 ******************************************************************************/

/*
 * Description : Updates the scheduling level of a task.
 *               This includes updating the internal MLFQ table,
 *               adjusting the FreeRTOS priority, resetting runtime
 *               statistics, assigning a new time quantum, and
 *               reflecting the change using system LEDs.
 */
void updateTaskPriority(TaskHandle_t task, MLFQ_QueueLevel_t newLevel)
{
    for (uint8_t table_index = 0; table_index < TICK_PROFILER_MAX_TASKS; table_index++)
    {
        if (g_taskTable[table_index].task_handle == task)
        {
            g_taskTable[table_index].task_level = newLevel;

            /* Update RTOS priority according to MLFQ level */
            vTaskPrioritySet(task, MLFQ_TO_RTOS_LEVEL_SETTER(newLevel));

            /* Reset runtime statistics and apply new quantum */
            setTaskQuantum(task, getQuantumForLevel(newLevel));
            resetTaskRuntime(task);

            /* Visual indication of task level */
            setLEDColor(newLevel);
            return;
        }
    }
}

/*
 * Description : Initializes the scheduler subsystem.
 *               Sets up the tick profiler and clears the
 *               internal task table entries.
 */
void initScheduler(void)
{
    /* Initialize runtime profiling system */
    tickProfilerInit();

    /* Clear task table */
    for (uint8_t table_index = 0; table_index < TICK_PROFILER_MAX_TASKS; table_index++)
    {
        g_taskTable[table_index].task_handle = NULL;
        g_taskTable[table_index].task_level = MLFQ_QUEUE_HIGH;
        g_taskTable[table_index].arrival_tick = 0;
    }
}

/*
 * Description : Registers a new task with the scheduler.
 *               Initializes profiling data, assigns the
 *               highest priority queue, and sets the initial
 *               quantum and arrival timestamp.
 */
void registerTask(TaskHandle_t taskHandle)
{
    for (uint8_t table_index = 0; table_index < TICK_PROFILER_MAX_TASKS; table_index++)
    {
        if (g_taskTable[table_index].task_handle == NULL)
        {
            /* Initialize profiler statistics for the task */
            if (!setupTaskStats(taskHandle))
            {
                break;
            }
            else
            {
                g_taskTable[table_index].task_handle = taskHandle;
                g_taskTable[table_index].task_level = MLFQ_QUEUE_HIGH;
                g_taskTable[table_index].arrival_tick = xTaskGetTickCount();

                /* Assign highest RTOS priority */
                vTaskPrioritySet(taskHandle,
                                 MLFQ_TO_RTOS_LEVEL_SETTER(MLFQ_QUEUE_HIGH));

                /* Assign initial quantum */
                setTaskQuantum(taskHandle, MLFQ_TIME_SLICE_HIGH);

                break;
            }
        }
    }
}

/*
 * Description : Demotes a task to a lower priority queue
 *               when it exhausts its assigned time quantum.
 *               Tasks at the lowest level remain there.
 */
void checkForDemotion(uint8_t table_index)
{
    TaskHandle_t task = g_taskTable[table_index].task_handle;
    MLFQ_QueueLevel_t currentLevel = g_taskTable[table_index].task_level;

    if(currentLevel < MLFQ_QUEUE_LOW)
    {
        updateTaskPriority(task,
            (MLFQ_QueueLevel_t)(MLFQ_TO_RTOS_LEVEL_SETTER(currentLevel + 1)));
    }
    else
    {
        updateTaskPriority(task,
            (MLFQ_QueueLevel_t)(MLFQ_QUEUE_LOW));
    }
}

/*
 * Description : Performs a global priority boost.
 *               Periodically elevates all tasks back to
 *               the highest priority queue to prevent starvation.
 */
void performGlobalBoost(void)
{
    for (uint8_t table_index = 0; table_index < TICK_PROFILER_MAX_TASKS; table_index++)
    {
        if (g_taskTable[table_index].task_handle != NULL)
        {
            updateTaskPriority(g_taskTable[table_index].task_handle,
                               MLFQ_QUEUE_HIGH);
        }
    }
}

/*
 * Description : Promotes an interactive task to a higher
 *               priority queue to improve responsiveness.
 */
void promoteInteractiveTask(TaskHandle_t task)
{
    for (uint8_t table_index = 0; table_index < TICK_PROFILER_MAX_TASKS; table_index++)
    {
        if (g_taskTable[table_index].task_handle == task)
        {
            if (g_taskTable[table_index].task_level != MLFQ_QUEUE_HIGH)
            {
                updateTaskPriority(
                    g_taskTable[table_index].task_handle,
                    (MLFQ_QueueLevel_t)
                    (g_taskTable[table_index].task_level - 1));
            }
        }
    }
}

/*
 * Description : Dedicated scheduler task.
 *               Handles task demotion events, periodic global
 *               priority boosts, and system monitoring reports.
 */
void schedulerTask(void *pvParameters)
{
    /* Register scheduler task with profiler */
    tickProfilerSetSchedulerTaskHandle(xTaskGetCurrentTaskHandle());

    /* Retrieve expired-quantum notification queue */
    QueueHandle_t expiredQueue = tickProfilerGetExpiredQueue();
    TaskHandle_t xExpiredHandle = NULL;

    /* Global boost timing control */
    TickType_t xLastBoostTime = xTaskGetTickCount();
    const TickType_t xBoostPeriod = pdMS_TO_TICKS(MLFQ_BOOST_PERIOD_MS);

    for (;;)
    {
        /* 1. Handle task demotions */
        while (xQueueReceive(expiredQueue, &xExpiredHandle, 0) == pdTRUE)
        {
            for (uint8_t i = 0; i < TICK_PROFILER_MAX_TASKS; i++)
            {
                if (g_taskTable[i].task_handle == xExpiredHandle)
                {
                    checkForDemotion(i);
                    break;
                }
            }
        }

        /* 2. Periodic global boost and reporting */
        TickType_t xNow = xTaskGetTickCount();
        if ((xNow - xLastBoostTime) >= xBoostPeriod)
        {
            performGlobalBoost();
            printQueueReport();
            xLastBoostTime = xNow;
        }

        /* 3. Scheduler idle delay */
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

/*
 * Description : Retrieves MLFQ and runtime profiling information
 *               for a task indexed in the scheduler table.
 */
bool schedulerGetTaskStats(uint32_t index, MLFQ_Task_Profiler_t *output)
{
    if (index >= TICK_PROFILER_MAX_TASKS ||
        g_taskTable[index].task_handle == NULL)
    {
        return false;
    }

    /* Copy scheduler metadata */
    output->task_info.task  = g_taskTable[index].task_handle;
    output->task_level      = g_taskTable[index].task_level;
    output->arrival_tick    = g_taskTable[index].arrival_tick;

    /* Copy live profiler statistics */
    output->task_info.run_ticks     =
        getTaskRuntime(output->task_info.task);
    output->task_info.quantum_ticks =
        getQuantumForLevel(output->task_level);

    return true;
}

/******************************************************************************
 *  END OF FILE
 ******************************************************************************/
