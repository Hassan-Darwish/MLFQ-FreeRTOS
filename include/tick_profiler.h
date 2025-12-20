#ifndef TICK_PROFILER_H
#define TICK_PROFILER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include <stdint.h>
#include <stdbool.h>

/* ---------------- Configuration Defaults ----------------
 * You can override these macros in your build system or before including this header.
 */
#ifndef TICK_PROFILER_MAX_TASKS
#define TICK_PROFILER_MAX_TASKS    16U
#endif

#ifndef TICK_PROFILER_EXPIRED_QUEUE_ENABLED
#define TICK_PROFILER_EXPIRED_QUEUE_ENABLED  1U
#endif

#ifndef TICK_PROFILER_EXPIRED_QUEUE_LENGTH
#define TICK_PROFILER_EXPIRED_QUEUE_LENGTH   (TICK_PROFILER_MAX_TASKS * 2U)
#endif

/* Per-task accounting structure */
typedef struct
{
    TaskHandle_t task;       /* NULL => empty slot */
    uint32_t     run_ticks;  /* Accumulated ticks for current CPU burst */
    uint32_t     quantum_ticks; /* Quantum configured for this task (0 => unset/disabled) */
} TickProfilerTaskInfo_t;

/* ---------------- Public API ---------------- */

/* Initialize internal state and (optionally) create expired-task queue.
 * Call before vTaskStartScheduler().
 * Returns true on success, false on allocation failure (queue creation).
 */
bool tickProfilerInit(void);

/* Register a task so profiler can account it.
 * Call after xTaskCreate().
 * Returns false if already registered or table is full.
 */
bool setupTaskStats(TaskHandle_t task);

/* Set a task quantum in ticks (must be > 0).
 * Returns false if task not registered or invalid input.
 */
bool setTaskQuantum(TaskHandle_t task, uint32_t quantumTicks);

/* Get accumulated run ticks for a task since last reset.
 * Returns 0 if not found.
 */
uint32_t getTaskRuntime(TaskHandle_t task);

/* Reset run_ticks for a task (scheduler typically calls this after handling expiry).
 * Returns false if task not found.
 */
bool resetTaskRuntime(TaskHandle_t task);

/* Provide the scheduler-manager task handle so tick hook can notify it from ISR. */
void tickProfilerSetSchedulerTaskHandle(TaskHandle_t schedulerHandle);

/* If enabled, returns the queue used to send expired TaskHandle_t from ISR to scheduler.
 * Returns NULL if feature is disabled or queue not created.
 */
QueueHandle_t tickProfilerGetExpiredQueue(void);

/* FreeRTOS tick hook (requires configUSE_TICK_HOOK == 1).
 * Do NOT call directly.
 */
void vApplicationTickHook(void);

#ifdef __cplusplus
}
#endif

#endif /* TICK_PROFILER_H */
