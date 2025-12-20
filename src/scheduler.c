/******************************************************************************
 *  MODULE NAME  : MLFQ Scheduler
 *  FILE         : scheduler.c
 *  DESCRIPTION  : Implements a Multi-Level Feedback Queue (MLFQ) scheduler
 *                 on top of FreeRTOS, handling task registration, promotion,
 *                 demotion, and periodic priority boosting.
 *  AUTHOR       : Hassan Darwish
 *  Date         : December 2025
 ******************************************************************************/

/******************************************************************************
 *  INCLUDES
 ******************************************************************************/
#include "scheduler.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

/******************************************************************************
 *  STATIC (PRIVATE) VARIABLES
 ******************************************************************************/
/* Table holding task profiling information and current MLFQ level */
static MLFQ_Task_Profiler_t g_taskTable[TICK_PROFILER_MAX_TASKS];

/******************************************************************************
 *  FUNCTION DEFINITIONS
 ******************************************************************************/

/*
 * Description : Updates the priority of a given task based on the new
 *               MLFQ queue level and applies the corresponding RTOS priority.
 */
void updateTaskPriority(TaskHandle_t task, MLFQ_QueueLevel_t newLevel)
{
    /* Iterate through task table to find the matching task */
    for (uint8_t table_index = 0; table_index < TICK_PROFILER_MAX_TASKS; table_index++)
    {
        if (g_taskTable[table_index].task_info.task == task)
        {
            /* Update internal MLFQ level */
            g_taskTable[table_index].task_level = newLevel;

            /* Update RTOS task priority */
            vTaskPrioritySet(task, MLFQ_TO_RTOS_LEVEL_SETTER(newLevel));
        }
    }
}

/*
 * Description : Initializes the scheduler subsystem by initializing
 *               the tick profiler and clearing the task table.
 */
void initScheduler(void)
{
    /* Initialize tick profiling mechanism */
    tickProfilerInit();

    /* Reset all task table entries */
    for (uint8_t table_index = 0; table_index < TICK_PROFILER_MAX_TASKS; table_index++)
    {
        g_taskTable[table_index].task_info.task = NULL;
        g_taskTable[table_index].task_level = MLFQ_QUEUE_HIGH;
    }
}

/*
 * Description : Registers a new task in the scheduler task table
 *               and assigns it the highest priority queue.
 */
void registerTask(TaskHandle_t taskHandle)
{
    /* Search for an empty slot in the task table */
    for (uint8_t table_index = 0; table_index < TICK_PROFILER_MAX_TASKS; table_index++)
    {
        if (g_taskTable[table_index].task_info.task == NULL)
        {
            /* Initialize profiling statistics for the task */
            if (!setupTaskStats(taskHandle))
            {
                break;
            }
            else
            {
                /* Store task handle */
                g_taskTable[table_index].task_info.task = taskHandle;

                /* Assign highest MLFQ level */
                g_taskTable[table_index].task_level = MLFQ_QUEUE_HIGH;

                /* Apply RTOS priority */
                vTaskPrioritySet(taskHandle,
                                 MLFQ_TO_RTOS_LEVEL_SETTER(MLFQ_QUEUE_HIGH));

                break;
            }
        }
    }
}

/*
 * Description : Checks whether a task should be demoted to a lower
 *               priority queue based on its execution behavior.
 */
void checkForDemotion(MLFQ_Task_Profiler_t status)
{
    /* Locate the task in the task table */
    for (uint8_t table_index = 0; table_index < TICK_PROFILER_MAX_TASKS; table_index++)
    {
        if (g_taskTable[table_index].task_info.task == status.task_info.task)
        {
            /* Demote task if it is not already at the lowest level */
            if (g_taskTable[table_index].task_level != MLFQ_QUEUE_LOW)
            {
                updateTaskPriority(
                    g_taskTable[table_index].task_info.task,
                    (MLFQ_QueueLevel_t)(g_taskTable[table_index].task_level + 1));
            }
        }
    }
}

/*
 * Description : Performs a global priority boost by moving all
 *               registered tasks back to the highest queue.
 */
void performGlobalBoost(void)
{
    /* Iterate through all registered tasks */
    for (uint8_t table_index = 0; table_index < TICK_PROFILER_MAX_TASKS; table_index++)
    {
        if (g_taskTable[table_index].task_info.task != NULL)
        {
            updateTaskPriority(g_taskTable[table_index].task_info.task,
                               MLFQ_QUEUE_HIGH);
        }
    }
}

/*
 * Description : Promotes an interactive task to a higher priority
 *               queue if it is not already at the top level.
 */
void promoteInteractiveTask(TaskHandle_t task)
{
    /* Search for the task in the table */
    for (uint8_t table_index = 0; table_index < TICK_PROFILER_MAX_TASKS; table_index++)
    {
        if (g_taskTable[table_index].task_info.task == task)
        {
            /* Promote task if possible */
            if (g_taskTable[table_index].task_level != MLFQ_QUEUE_HIGH)
            {
                updateTaskPriority(
                    g_taskTable[table_index].task_info.task,
                    (MLFQ_QueueLevel_t)(g_taskTable[table_index].task_level - 1));
            }
        }
    }
}

/*
 * Description : Scheduler management task responsible for handling
 *               task demotions based on expired ticks and performing
 *               periodic global priority boosts.
 */
void schedulerTask(void *pvParameters)
{
    /* Register scheduler task with tick profiler */
    tickProfilerSetSchedulerTaskHandle(xTaskGetCurrentTaskHandle());

    /* Get queue of expired tasks */
    QueueHandle_t expiredQueue = tickProfilerGetExpiredQueue();

    /* Handle for expired task */
    TaskHandle_t xExpiredHandle = NULL;

    /* Track last global boost time */
    TickType_t xLastBoostTime = xTaskGetTickCount();

    /* Global boost interval */
    const TickType_t xBoostPeriod =
        pdMS_TO_TICKS(MLFQ_BOOST_PERIOD_MS);

    /* Scheduler loop */
    for (;;)
    {
        /* Check for expired task notifications */
        if (xQueueReceive(expiredQueue, &xExpiredHandle, 10) == pdTRUE)
        {
            for (uint8_t table_index = 0; table_index < TICK_PROFILER_MAX_TASKS; table_index++)
            {
                if (g_taskTable[table_index].task_info.task == xExpiredHandle)
                {
                    /* Apply demotion logic */
                    checkForDemotion(g_taskTable[table_index]);

                    break;
                }
            }
        }

        /* Check if it is time for a global boost */
        TickType_t xNow = xTaskGetTickCount();

        if ((xNow - xLastBoostTime) >= xBoostPeriod)
        {
            performGlobalBoost();
            xLastBoostTime = xNow;
        }
    }
}

/******************************************************************************
 *  END OF FILE
 ******************************************************************************/
