#include "tick_profiler.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "portmacro.h"   /* portYIELD_FROM_ISR() */
#include <string.h>      /* memset */



/* ---------------- Private Data ---------------- */
static TickProfilerTaskInfo_t g_taskTable[TICK_PROFILER_MAX_TASKS];

#if (TICK_PROFILER_EXPIRED_QUEUE_ENABLED == 1U)
static QueueHandle_t g_expiredQueue = NULL;
#endif

static TaskHandle_t g_schedulerTaskHandle = NULL;

/* ---------------- Private Helpers ---------------- */

/* Find task index, returns -1 if not found */
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

/* Find empty slot, returns -1 if none */
static int32_t findEmptySlot(void)
{
    for (uint32_t i = 0U; i < TICK_PROFILER_MAX_TASKS; ++i) {
        if (g_taskTable[i].task == NULL) {
            return (int32_t)i;
        }
    }
    return -1;
}

/* ---------------- Public API ---------------- */

bool tickProfilerInit(void)
{
    /* Usually called before scheduler starts, so critical section is optional.
       Keeping it safe anyway. */
    taskENTER_CRITICAL();
    {
        memset(g_taskTable, 0, sizeof(g_taskTable));
        g_schedulerTaskHandle = NULL;
    }
    taskEXIT_CRITICAL();

#if (TICK_PROFILER_EXPIRED_QUEUE_ENABLED == 1U)
    /* Create queue (task context). */
    g_expiredQueue = xQueueCreate((UBaseType_t)TICK_PROFILER_EXPIRED_QUEUE_LENGTH,
                                  (UBaseType_t)sizeof(TaskHandle_t));
    if (g_expiredQueue == NULL) {
        return false;
    }
#endif

    return true;
}

bool setupTaskStats(TaskHandle_t task)
{
    if (task == NULL) {
        return false;
    }

    taskENTER_CRITICAL();
    {
        /* Already registered? */
        if (findTaskIndex(task) >= 0) {
            taskEXIT_CRITICAL();
            return false;
        }

        int32_t slot = findEmptySlot();
        if (slot < 0) {
            taskEXIT_CRITICAL();
            return false; /* table full */
        }

        g_taskTable[slot].task = task;
        g_taskTable[slot].run_ticks = 0U;
        g_taskTable[slot].quantum_ticks = 0U; /* scheduler will set */
    }
    taskEXIT_CRITICAL();

    return true;
}

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

void tickProfilerSetSchedulerTaskHandle(TaskHandle_t schedulerHandle)
{
    taskENTER_CRITICAL();
    {
        g_schedulerTaskHandle = schedulerHandle;
    }
    taskEXIT_CRITICAL();
}

QueueHandle_t tickProfilerGetExpiredQueue(void)
{
#if (TICK_PROFILER_EXPIRED_QUEUE_ENABLED == 1U)
    return g_expiredQueue;
#else
    return NULL;
#endif
}

/* ---------------- FreeRTOS Tick Hook ----------------
 * Called from SysTick ISR when configUSE_TICK_HOOK == 1
 *
 * - Increment runtime of currently running task (if registered)
 * - If quantum is set and expired:
 *   - push TaskHandle_t into expired queue (FromISR) if enabled
 *   - notify scheduler task (FromISR)
 * - Yield from ISR if needed
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

            /* Increment run_ticks (Cortex-M4: aligned 32-bit access is atomic) */
            g_taskTable[i].run_ticks++;

            /* Check expiry only if quantum configured */
            if ((g_taskTable[i].quantum_ticks != 0U) &&
                (g_taskTable[i].run_ticks >= g_taskTable[i].quantum_ticks))
            {
#if (TICK_PROFILER_EXPIRED_QUEUE_ENABLED == 1U)
                if (g_expiredQueue != NULL) {
                    (void)xQueueSendFromISR(g_expiredQueue, &current, &xHigherPriorityTaskWoken);
                }
#endif

                if (g_schedulerTaskHandle != NULL) {
                    (void)vTaskNotifyGiveFromISR(g_schedulerTaskHandle, &xHigherPriorityTaskWoken);
                }
            }

            break; /* found task */
        }
    }

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
