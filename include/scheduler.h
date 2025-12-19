#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#include "FreeRTOS.h"
#include "tick_profiler.h"
#include "task.h"


typedef enum
{
    MLFQ_QUEUE_HIGH,
    MLFQ_QUEUE_MEDIUM,
    MLFQ_QUEUE_LOW,
    MLFQ_NUMBER_QUEUES
}MLFQ_QueueLevel_t;

typedef struct
{
    TickProfilerTaskInfo_t task_info;
    MLFQ_QueueLevel_t task_level;
}MLFQ_Task_Profiler_t;

#define MLFQ_TOP_PRIORITY_NUMBER                (5U)

#define MLFQ_TO_RTOS_LEVEL_SETTER(level)        (MLFQ_TOP_PRIORITY_NUMBER - level)

void initScheduler(void);
void schedulerTask(void* pvParam);
void registerTask(TaskHandle_t task);

void updateTaskPriority(TaskHandle_t task, MLFQ_QueueLevel_t newLevel);
void checkForDemotion(MLFQ_Task_Profiler_t status);
void performGlobalBoost(void);
void promoteInteractiveTask(TaskHandle_t task);

#endif /* SCHEDULER_H_ */
