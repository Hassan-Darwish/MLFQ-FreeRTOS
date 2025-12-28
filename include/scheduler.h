/******************************************************************************
 *  MODULE NAME  : MLFQ Scheduler
 *  FILE         : scheduler.h
 *  DESCRIPTION  : Public interface for the Multi-Level Feedback Queue (MLFQ)
 *                 scheduler. Defines queue levels, task control blocks,
 *                 configuration macros, and scheduler APIs.
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

/* FreeRTOS task management */
#include "task.h"

/* Tick-based runtime profiler interface */
#include "tick_profiler.h"

/* Standard types */
#include <stdbool.h>
#include <stdint.h>

/******************************************************************************
 *  TYPE DEFINITIONS
 ******************************************************************************/

/*
 * Description : Enumeration defining the available MLFQ priority levels.
 *               Lower enum value corresponds to higher scheduling priority.
 */
typedef enum
{
    MLFQ_QUEUE_HIGH,
    MLFQ_QUEUE_MEDIUM,
    MLFQ_QUEUE_LOW,
    MLFQ_NUMBER_QUEUES
} MLFQ_QueueLevel_t;

/*
 * Description : Task Control Block used internally by the scheduler.
 *               Stores scheduling metadata for each registered task.
 */
typedef struct
{
    TaskHandle_t      task_handle;   /* FreeRTOS task handle */
    MLFQ_QueueLevel_t task_level;    /* Current MLFQ queue level */
    TickType_t        arrival_tick;  /* Tick count when task was registered */
} MLFQ_TCB_t;

/*
 * Description : Aggregated task profiling structure.
 *               Combines scheduler metadata with runtime profiling data.
 */
typedef struct
{
    TickProfilerTaskInfo_t task_info;    /* Runtime profiler statistics */
    MLFQ_QueueLevel_t      task_level;   /* Current MLFQ level */
    TickType_t             arrival_tick; /* Task arrival timestamp */
} MLFQ_Task_Profiler_t;

/******************************************************************************
 *  MACRO DEFINITIONS AND CONFIGURATIONS
 ******************************************************************************/

/* Highest FreeRTOS priority number used by the scheduler */
#define MLFQ_TOP_PRIORITY_NUMBER                (5U)

/*
 * Description : Converts an MLFQ queue level to a FreeRTOS priority value.
 */
#define MLFQ_TO_RTOS_LEVEL_SETTER(level)        (MLFQ_TOP_PRIORITY_NUMBER - level)

/* Periodic priority boost interval (milliseconds) */
#define MLFQ_BOOST_PERIOD_MS                    3000U

/* Time slice values assigned per queue level (ticks) */
#define MLFQ_TIME_SLICE_HIGH                    20U
#define MLFQ_TIME_SLICE_MEDIUM                  50U
#define MLFQ_TIME_SLICE_LOW                     100U

/* Generic wait duration used by scheduler logic */
#define TICKS_TO_BE_WAITED                      (10U)

/******************************************************************************
 *  FUNCTION PROTOTYPES
 ******************************************************************************/

/*
 * Description : Returns the time quantum assigned to a specific MLFQ level.
 * Note        : Declared here for internal consistency with implementation.
 */
static uint32_t getQuantumForLevel(MLFQ_QueueLevel_t level);


/*
 * Description : Initializes the scheduler and underlying
 *               runtime profiling infrastructure.
 */
void initScheduler(void);

/*
 * Description : Registers a task with the scheduler and assigns
 *               initial priority and quantum values.
 */
void registerTask(TaskHandle_t task);

/*
 * Description : Updates a task’s MLFQ level and synchronizes its
 *               FreeRTOS priority and runtime statistics.
 */
void updateTaskPriority(TaskHandle_t task, MLFQ_QueueLevel_t newLevel);

/*
 * Description : Demotes a task after it exhausts its time quantum.
 */
void checkForDemotion(uint8_t table_index);

/*
 * Description : Performs a global priority boost to prevent starvation.
 */
void performGlobalBoost(void);

/*
 * Description : Promotes an interactive task to a higher priority queue.
 */
void promoteInteractiveTask(TaskHandle_t task);

/*
 * Description : Main scheduler task responsible for handling demotion,
 *               boosting, and reporting logic.
 */
void schedulerTask(void* pvParameters);

/*
 * Description : Retrieves scheduler and profiling information for a task
 *               indexed in the internal task table.
 */
bool schedulerGetTaskStats(uint32_t index, MLFQ_Task_Profiler_t *output);

/******************************************************************************
 *  END OF FILE
 ******************************************************************************/

#endif /* SCHEDULER_H_ */
