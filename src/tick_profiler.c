/******************************************************************************
 *  MODULE NAME  : Tick Profiler
 *  FILE         : tick_profiler.c
 *  DESCRIPTION  : Implements a lightweight runtime profiling mechanism for
 *                 FreeRTOS tasks. Tracks execution ticks, enforces time
 *                 quanta, and notifies the scheduler upon quantum expiration.
 *  AUTHOR       : Elham Karam
 *  Date         : December 2025
 ******************************************************************************/

/******************************************************************************
 *  INCLUDES
 ******************************************************************************/
#include "tick_profiler.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "portmacro.h"
#include <string.h>

/******************************************************************************
 *  STATIC (PRIVATE) VARIABLES
 ******************************************************************************/
/* Table holding runtime statistics for all tracked tasks */
static TickProfilerTaskInfo_t g_taskTable[TICK_PROFILER_MAX_TASKS];

/* Queue used to notify scheduler of expired task quanta (optional) */
#if (TICK_PROFILER_EXPIRED_QUEUE_ENABLED == 1U)
static QueueHandle_t g_expiredQueue = NULL;
#endif

/* Handle of the scheduler task to be notified from ISR */
static TaskHandle_t g_schedulerTaskHandle = NULL;

/******************************************************************************
 *  STATIC (PRIVATE) FUNCTION DEFINITIONS
 ******************************************************************************/

/*
 * Description : Finds the index of a task inside the profiler table.
 *               Returns -1 if the task is not found or invalid.
 */
static int32_t findTaskIndex(TaskHandle_t task)
{
    if (task == NULL) {
        return -1;
    }

    for (uint32_t i = 0U; i < TICK_PROFILER_MAX_TASKS; ++i) {
        if (g_taskTable[i].task == task) {
            return (int32_t)i;
        }
    }
    return -1;
}

/*
 * Description : Finds the first available empty slot in the task table.
 *               Returns -1 if the table is full.
 */
static int32_t findEmptySlot(void)
{
    for (uint32_t i = 0U; i < TICK_PROFILER_MAX_TASKS; ++i) {
        if (g_taskTable[i].task == NULL) {
            return (int32_t)i;
        }
    }
    return -1;
}

/******************************************************************************
 *  FUNCTION DEFINITIONS
 ******************************************************************************/

/*
 * Description : Initializes the tick profiler subsystem.
 *               Clears the task table, resets scheduler linkage,
 *               and creates the expired-quantum queue if enabled.
 */
bool tickProfilerInit(void)
{
    /* Protect shared data during initialization */
    taskENTER_CRITICAL();
    {
        memset(g_taskTable, 0, sizeof(g_taskTable));
        g_schedulerTaskHandle = NULL;
    }
    taskEXIT_CRITICAL();

#if (TICK_PROFILER_EXPIRED_QUEUE_ENABLED == 1U)
    /* Create queue for expired task notifications */
    g_expiredQueue = xQueueCreate(
        (UBaseType_t)TICK_PROFILER_EXPIRED_QUEUE_LENGTH,
        (UBaseType_t)sizeof(TaskHandle_t));

    if (g_expiredQueue == NULL) {
        return false;
    }
#endif

    return true;
}

/*
 * Description : Registers a task with the profiler.
 *               Allocates a table entry and initializes
 *               runtime and quantum counters.
 */
bool setupTaskStats(TaskHandle_t task)
{
    if (task == NULL) {
        return false;
    }

    taskENTER_CRITICAL();
    {
        /* Prevent duplicate registration */
        if (findTaskIndex(task) >= 0) {
            taskEXIT_CRITICAL();
            return false;
        }

        /* Find empty slot */
        int32_t slot = findEmptySlot();
        if (slot < 0) {
            taskEXIT_CRITICAL();
            return false;
        }

        /* Initialize task statistics */
        g_taskTable[slot].task = task;
        g_taskTable[slot].run_ticks = 0U;
        g_taskTable[slot].quantum_ticks = 0U;
    }
    taskEXIT_CRITICAL();

    return true;
}

/*
 * Description : Assigns a time quantum (in ticks) to a task.
 */
bool setTaskQuantum(TaskHandle_t task, uint32_t quantumTicks)
{
    if (task == NULL || quantumTicks == 0U) {
        return false;
    }

    taskENTER_CRITICAL();
    {
        int32_t idx = findTaskIndex(task);
        if (idx < 0) {
            taskEXIT_CRITICAL();
            return false;
        }

        g_taskTable[idx].quantum_ticks = quantumTicks;
    }
    taskEXIT_CRITICAL();

    return true;
}

/*
 * Description : Returns the accumulated runtime (in ticks)
 *               of the specified task.
 */
uint32_t getTaskRuntime(TaskHandle_t task)
{
    if (task == NULL) {
        return 0U;
    }

    uint32_t runtime = 0U;

    taskENTER_CRITICAL();
    {
        int32_t idx = findTaskIndex(task);
        if (idx >= 0) {
            runtime = g_taskTable[idx].run_ticks;
        }
    }
    taskEXIT_CRITICAL();

    return runtime;
}

/*
 * Description : Resets the runtime counter of a task.
 */
bool resetTaskRuntime(TaskHandle_t task)
{
    if (task == NULL) {
        return false;
    }

    taskENTER_CRITICAL();
    {
        int32_t idx = findTaskIndex(task);
        if (idx < 0) {
            taskEXIT_CRITICAL();
            return false;
        }

        g_taskTable[idx].run_ticks = 0U;
    }
    taskEXIT_CRITICAL();

    return true;
}

/*
 * Description : Registers the scheduler task handle.
 *               Used to notify the scheduler from ISR
 *               when task quanta expire.
 */
void tickProfilerSetSchedulerTaskHandle(TaskHandle_t schedulerHandle)
{
    taskENTER_CRITICAL();
    {
        g_schedulerTaskHandle = schedulerHandle;
    }
    taskEXIT_CRITICAL();
}

/*
 * Description : Returns the queue used to report expired
 *               task quanta to the scheduler.
 */
QueueHandle_t tickProfilerGetExpiredQueue(void)
{
#if (TICK_PROFILER_EXPIRED_QUEUE_ENABLED == 1U)
    return g_expiredQueue;
#else
    return NULL;
#endif
}

/*
 * Description : FreeRTOS tick hook.
 *               Executes on every system tick interrupt.
 *               Updates runtime counters, checks for quantum
 *               expiration, and notifies the scheduler.
 */
void vApplicationTickHook(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    TaskHandle_t current = xTaskGetCurrentTaskHandle();

    if (current == NULL) {
        return;
    }

    for (uint32_t i = 0U; i < TICK_PROFILER_MAX_TASKS; ++i) {
        if (g_taskTable[i].task == current) {

            /* Increment runtime counter */
            g_taskTable[i].run_ticks++;

            /* Check for quantum expiration */
            if ((g_taskTable[i].quantum_ticks != 0U) &&
                (g_taskTable[i].run_ticks >= g_taskTable[i].quantum_ticks))
            {
#if (TICK_PROFILER_EXPIRED_QUEUE_ENABLED == 1U)
                /* Notify scheduler via queue */
                if (g_expiredQueue != NULL) {
                    (void)xQueueSendFromISR(
                        g_expiredQueue,
                        &current,
                        &xHigherPriorityTaskWoken);
                }
#endif

                /* Direct scheduler notification */
                if (g_schedulerTaskHandle != NULL) {
                    (void)vTaskNotifyGiveFromISR(
                        g_schedulerTaskHandle,
                        &xHigherPriorityTaskWoken);
                }
            }

            break;
        }
    }

    /* Perform context switch if required */
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/******************************************************************************
 *  END OF FILE
 ******************************************************************************/
