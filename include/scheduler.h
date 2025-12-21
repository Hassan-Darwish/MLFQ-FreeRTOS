
#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#include "FreeRTOS.h"

#include "tick_profiler.h"

#include "task.h"
#include <stdbool.h>
#include <stdint.h>

typedef enum
{
    MLFQ_QUEUE_HIGH,
    MLFQ_QUEUE_MEDIUM,
    MLFQ_QUEUE_LOW,
    MLFQ_NUMBER_QUEUES
} MLFQ_QueueLevel_t;

typedef struct
{
    TaskHandle_t      task_handle;
    MLFQ_QueueLevel_t task_level;
    TickType_t        arrival_tick;
} MLFQ_TCB_t;


typedef struct
{
    TickProfilerTaskInfo_t task_info;
    MLFQ_QueueLevel_t      task_level;
    TickType_t             arrival_tick;
} MLFQ_Task_Profiler_t;


#define MLFQ_TOP_PRIORITY_NUMBER                (5U)


#define MLFQ_TO_RTOS_LEVEL_SETTER(level)        (MLFQ_TOP_PRIORITY_NUMBER - level)

#define MLFQ_BOOST_PERIOD_MS                    500

#define MLFQ_TIME_SLICE_HIGH                    10U
#define MLFQ_TIME_SLICE_MEDIUM                  20U
#define MLFQ_TIME_SLICE_LOW                     50U

#define TICKS_TO_BE_WAITED                      (10U)

static uint32_t getQuantumForLevel(MLFQ_QueueLevel_t level);

void updateTaskPriority(TaskHandle_t task, MLFQ_QueueLevel_t newLevel);


void initScheduler(void);


void registerTask(TaskHandle_t task);


void checkForDemotion(MLFQ_Task_Profiler_t status);


void performGlobalBoost(void);


void promoteInteractiveTask(TaskHandle_t task);

void schedulerTask(void* pvParameters);


bool schedulerGetTaskStats(uint32_t index, MLFQ_Task_Profiler_t *output);

#endif /* SCHEDULER_H_ */

