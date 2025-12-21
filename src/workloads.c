/******************************************************************************
 *  MODULE NAME  : Workload Tasks
 *  FILE         : workloads.c
 *  DESCRIPTION  : Implements simulated workloads used to test scheduler
 *                 behavior, including interactive and CPU-heavy tasks.
 *  AUTHOR       : Ahmed Alaa
 *  Date         : December 2025
 ******************************************************************************/

/******************************************************************************
 *  INCLUDES
 ******************************************************************************/
/* Workload task declarations */
#include "workloads.h"

/* FreeRTOS task management */
#include "FreeRTOS.h"
#include "task.h"


/* Standard integer types */
#include <stdint.h>

/******************************************************************************
 *  FUNCTION DEFINITIONS
 ******************************************************************************/

/*
 * Description : Simulates a blocking operation by delaying the
 *               currently running task for a short duration.
 */
void simulateBlocking(void)
{
    /* Yield CPU execution for one tick */
    vTaskDelay(1);
}

/*
 * Description : Represents an interactive task that performs a
 *               short computation repeatedly and frequently blocks,
 *               simulating user-driven or I/O-bound behavior.
 */
void runInteractiveTask(void *pvParameters)
{
    /* Variable used to simulate computation */
    volatile uint8_t mathCalculation = 0;

    /* Task name passed as parameter (unused except for identification) */
    const char *taskName = (char *)pvParameters;

    /* Task execution loop */
    for (;;)
    {
        /* Simulated short computation workload */
        for (uint8_t task_loop = 0; task_loop < INTERACTIVE_TASK_TIME; task_loop++)
        {
            mathCalculation++;
        }

        /* Simulate blocking behavior */
        simulateBlocking();
    }
}

/*
 * Description : Represents a CPU-intensive task that performs
 *               long computations before yielding execution,
 *               simulating CPU-bound behavior.
 */
void runCPUHeavyTask(void *pvParameters)
{
    /* Variable used to simulate heavy computation */
    volatile uint16_t mathCalculation = 0;

    /* Task name passed as parameter (unused except for identification) */
    const char *taskname = (char*) pvParameters;

    /* Task execution loop */
    for (;;)
    {
        /* Simulated heavy computation workload */
        for (uint16_t task_loop = 0; task_loop < HEAVY_TASK_TIME; task_loop++)
        {
            mathCalculation++;
        }

        /* Simulate blocking behavior */
        simulateBlocking();
    }
}

/******************************************************************************
 *  END OF FILE
 ******************************************************************************/
