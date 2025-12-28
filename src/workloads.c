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

volatile uint32_t g_interactive_work_counter = 0;
volatile uint32_t g_cpu_work_counter = 0;
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
    vTaskDelay(5);
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
    volatile const char *taskName = (char *)pvParameters;

    /* Task execution loop */
    for (;;)
    {
        /* Simulated short computation workload */
        for (uint16_t task_loop = 0; task_loop < INTERACTIVE_TASK_TIME; task_loop++)
        {
            mathCalculation++;
        }
        g_interactive_work_counter++;
        /* Simulate blocking behavior */
        simulateBlocking();
    }
}

/*
 * Description : Represents a CPU-intensive task that performs
 *               long computations before yielding execution,
 *               simulating CPU-bound behavior.
 */
/* workloads.c */

/* workloads.c */

void runCPUHeavyTask(void *pvParameters)
{
    volatile uint8_t mathCalculation = 0;
    const char *taskName = (char*) pvParameters;

    for (;;)
    {
        /* INCREASED LOAD:
         * 100 iterations * ~1ms = ~100ms of CPU time.
         * This ensures it breaks the 10ms limit AND the 20ms limit.
         */
        for (uint16_t i = 0; i < 1000; i++) // <--- CHANGE THIS FROM 20 TO 100
        {
            for (uint16_t task_loop = 0; task_loop < HEAVY_TASK_TIME; task_loop++)
            {
                mathCalculation++;
            }
            g_cpu_work_counter ++;
        }

        /* Now yield */
        simulateBlocking();
    }
}

/******************************************************************************
 *  END OF FILE
 ******************************************************************************/
