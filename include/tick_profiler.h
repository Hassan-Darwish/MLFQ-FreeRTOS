/******************************************************************************
 *  MODULE NAME  : Tick Profiler
 *  FILE         : tick_profiler.h
 *  DESCRIPTION  : Public API for tracking task runtime using FreeRTOS tick hook.
 *  AUTHOR       : Elham Karam
 *  DATE CREATED : December 2025
 ******************************************************************************/

#ifndef TICK_PROFILER_H
#define TICK_PROFILER_H

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 *  INCLUDES
 ******************************************************************************/

#include "FreeRTOS.h"   /* FreeRTOS base definitions */
#include "task.h"       /* Task services */
#include "queue.h"      /* Queue services */
#include <stdint.h>     /* Fixed-width integer types */
#include <stdbool.h>    /* Boolean type */

/******************************************************************************
 *  MACRO DEFINITIONS AND CONFIGURATIONS
 ******************************************************************************/

/* Maximum number of tasks that can be profiled */
#ifndef TICK_PROFILER_MAX_TASKS
#define TICK_PROFILER_MAX_TASKS    16U
#endif

/* Enables expired quantum queue support */
#ifndef TICK_PROFILER_EXPIRED_QUEUE_ENABLED
#define TICK_PROFILER_EXPIRED_QUEUE_ENABLED  1U
#endif

/* Length of the expired quantum queue */
#ifndef TICK_PROFILER_EXPIRED_QUEUE_LENGTH
#define TICK_PROFILER_EXPIRED_QUEUE_LENGTH   (TICK_PROFILER_MAX_TASKS * 2U)
#endif

/******************************************************************************
 *  TYPE DEFINITIONS
 ******************************************************************************/

/* Structure holding per-task runtime statistics */
typedef struct
{
    TaskHandle_t task;        /* Associated FreeRTOS task */
    uint32_t     run_ticks;   /* Total execution time in ticks */
    uint32_t     quantum_ticks; /* Assigned execution quantum */
} TickProfilerTaskInfo_t;

/******************************************************************************
 *  FUNCTION PROTOTYPES
 ******************************************************************************/

/* Initializes the tick profiler module */
bool tickProfilerInit(void);

/* Registers a task for runtime profiling */
bool setupTaskStats(TaskHandle_t task);

/* Assigns a time quantum to a task */
bool setTaskQuantum(TaskHandle_t task, uint32_t quantumTicks);

/* Retrieves the runtime of a task */
uint32_t getTaskRuntime(TaskHandle_t task);

/* Resets the runtime counter of a task */
bool resetTaskRuntime(TaskHandle_t task);

/* Sets the scheduler task handle for ISR notification */
void tickProfilerSetSchedulerTaskHandle(TaskHandle_t schedulerHandle);

/* Returns the queue containing expired tasks */
QueueHandle_t tickProfilerGetExpiredQueue(void);

/* FreeRTOS tick hook implementation */
void vApplicationTickHook(void);

#ifdef __cplusplus
}
#endif

/******************************************************************************
 *  END OF FILE
 ******************************************************************************/

#endif /* TICK_PROFILER_H */
