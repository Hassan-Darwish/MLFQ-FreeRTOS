#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#include "FreeRTOS.h"

typedef enum
{
    MLFQ_QUEUE_HIGH,
    MLFQ_QUEUE_MEDIUM,
    MLFQ_QUEUE_LOW,
    MLFQ_NUMBER_QUEUES
}MLFQ_QueueLevel_t;

#define MLFQ_TOP_PRIORITY_NUMBER                5

#define MLFQ_TO_RTOS_LEVEL_SETTER(level)        (MLFQ_TOP_PRIORITY_NUMBER - level)

void initScheduler(void);
void updateTaskPriority(TaskHandle_t task, MLFQ_QueueLevel_t level);
void checkForDemotion(TaskStats status);
void performGlobalBoost(void);
void promoteInteractiveTask(TaskHandle_t task);

#endif /* SCHEDULER_H_ */
