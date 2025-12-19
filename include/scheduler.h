/******************************************************************************
 *  MODULE NAME  : MLFQ Scheduler
 *  FILE         : scheduler.h
 *  DESCRIPTION  : Declares interfaces, data types, and configuration macros
 *                 for the Multi-Level Feedback Queue (MLFQ) scheduler.
 *  AUTHOR       : Hassan Darwish
 *  Date         : December 2025
 ******************************************************************************/

#ifndef SCHEDULER_H_
#define SCHEDULER_H_

/******************************************************************************
 *  INCLUDES
 ******************************************************************************/
/* FreeRTOS core definitions */
#include "FreeRTOS.h"

/* Tick profiler interface used for task execution tracking */
#include "tick_profiler.h"

/* FreeRTOS task management interface */
#include "task.h"

/******************************************************************************
 *  TYPE DEFINITIONS
 ******************************************************************************/
/*
 * Description : Enumeration defining the available MLFQ priority levels.
 *               Tasks move between these queues based on their behavior.
 */
typedef enum
{
    MLFQ_QUEUE_HIGH,     /* Highest priority queue */
    MLFQ_QUEUE_MEDIUM,   /* Medium priority queue */
    MLFQ_QUEUE_LOW,      /* Lowest priority queue */
    MLFQ_NUMBER_QUEUES   /* Total number of MLFQ levels */
} MLFQ_QueueLevel_t;

/*
 * Description : Structure holding task profiling information along with
 *               its current MLFQ priority level.
 */
typedef struct
{
    TickProfilerTaskInfo_t task_info; /* Execution statistics of the task */
    MLFQ_QueueLevel_t task_level;     /* Current MLFQ queue level */
} MLFQ_Task_Profiler_t;

/******************************************************************************
 *  MACRO DEFINITIONS AND CONFIGURATIONS
 ******************************************************************************/
/* Highest RTOS priority number used by the MLFQ scheduler */
#define MLFQ_TOP_PRIORITY_NUMBER                (5U)

/*
 * Description : Maps an MLFQ queue level to the corresponding
 *               FreeRTOS priority value.
 */
#define MLFQ_TO_RTOS_LEVEL_SETTER(level)        (MLFQ_TOP_PRIORITY_NUMBER - level)

/* Period (in milliseconds) after which a global priority boost is applied */
#define MLFQ_BOOST_PERIOD_MS   500

/******************************************************************************
 *  FUNCTION PROTOTYPES
 ******************************************************************************/
/*
 * Description : Updates the priority of a task according to the specified
 *               MLFQ queue level.
 */
void updateTaskPriority(TaskHandle_t task, MLFQ_QueueLevel_t newLevel);

/*
 * Description : Initializes the MLFQ scheduler and internal task table.
 */
void initScheduler(void);

/*
 * Description : Registers a task with the scheduler and assigns it
 *               the highest priority queue.
 */
void registerTask(TaskHandle_t task);

/*
 * Description : Evaluates a task’s execution behavior and demotes it
 *               to a lower priority queue if required.
 */
void checkForDemotion(MLFQ_Task_Profiler_t status);

/*
 * Description : Boosts all registered tasks back to the highest
 *               priority queue.
 */
void performGlobalBoost(void);

/*
 * Description : Promotes an interactive task to a higher priority
 *               queue if possible.
 */
void promoteInteractiveTask(TaskHandle_t task);

/*
 * Description : Scheduler management task responsible for handling
 *               demotion events and periodic global boosts.
 */
void schedulerTask(void* pvParam);

#endif /* SCHEDULER_H_ */

/******************************************************************************
 *  END OF FILE
 ******************************************************************************/
