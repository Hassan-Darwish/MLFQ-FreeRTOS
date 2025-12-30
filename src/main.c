/******************************************************************************
 * MODULE NAME  : Main Application
 * FILE         : main.c
 * DESCRIPTION  : Entry point for the MLFQ Scheduler project. Initializes
 * drivers, creates workload tasks, and starts the scheduler.
 * AUTHOR       : Amr Olama
 * DATE         : December 2025
 ******************************************************************************/

/******************************************************************************
 * INCLUDES
 ******************************************************************************/
/* FreeRTOS Core */
#include "FreeRTOS.h"
#include "task.h"

/* Project Modules */
#include "drivers.h"        /* UART and GPIO/LEDs */
#include "scheduler.h"      /* MLFQ Logic */
#include "workloads.h"      /* Simulation Tasks (Heavy/Interactive) */
#include "metrics_logger.h" /* Logging Utilities */

/******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/
/* Task Handles (Required to register them with the MLFQ Scheduler) */
TaskHandle_t hTask1_Interactive = NULL;
TaskHandle_t hTask2_Heavy       = NULL;
TaskHandle_t hTask3_Heavy       = NULL;
TaskHandle_t hTask4_Interactive = NULL;

/* Scheduler Management Task Handle */
TaskHandle_t hSchedulerTask     = NULL;

/******************************************************************************
 * FUNCTION DEFINITIONS
 ******************************************************************************/

int main(void)
{

    /* Initialize UART for logging (Baud: 115200) */
    initUART();

    /* Initialize RGB LEDs for visual feedback (Red/Green/Blue) */
    initGPIO();

    /* Send boot banner */
    sendLog("\n\n");
    sendLog("************************************************\r\n");
    sendLog("* MLFQ SCHEDULER PROJECT START          *\r\n");
    sendLog("* Target: Tiva-C (TM4C123G)             *\r\n");
    sendLog("************************************************\r\n");

    /* Initialize internal tables and Tick Profiler */
    initScheduler();

    /* TASK 1: Interactive (Should stay High Priority / Green LED) */
    xTaskCreate(runInteractiveTask,         /* Function */
                "Interact_1",               /* Name */
                256,                        /* Stack Size */
                (void*)"Interact_1",        /* Parameter */
                MLFQ_TOP_PRIORITY_NUMBER,   /* Initial Priority (Must match MLFQ High) */
                &hTask1_Interactive);       /* Handle Storage */

    /* TASK 2: CPU Heavy (Should drop to Low Priority / Red LED) */
    xTaskCreate(runCPUHeavyTask,
                "Heavy_2",
                256,
                (void*)"Heavy_2",
                MLFQ_TOP_PRIORITY_NUMBER,
                &hTask2_Heavy);

    /* TASK 3: CPU Heavy (Should drop to Low Priority / Red LED) */
    xTaskCreate(runCPUHeavyTask,
                "Heavy_3",
                256,
                (void*)"Heavy_3",
                MLFQ_TOP_PRIORITY_NUMBER,
                &hTask3_Heavy);

    /* TASK 4: Interactive (Should stay High Priority / Green LED) */
    xTaskCreate(runInteractiveTask,
                "Interact_4",
                256,
                (void*)"Interact_4",
                MLFQ_TOP_PRIORITY_NUMBER,
                &hTask4_Interactive);

    /* This tells the scheduler to start tracking these tasks' runtimes */
    registerTask(hTask1_Interactive);
    registerTask(hTask2_Heavy);
    registerTask(hTask3_Heavy);
    registerTask(hTask4_Interactive);

    sendLog("[System] Workload tasks created and registered.\r\n");

    /* * Scheduler Task: Manages Demotion and Global Boosts.
     * PRIORITY: Must be higher than the highest MLFQ queue so it can interrupt!
     * STACK: Large (512 or 1024) because it calls printQueueReport() -> snprintf().
     */
    xTaskCreate(schedulerTask,
                "Scheduler",
                512,                       /* Stack size increased for safe printing */
                NULL,
                MLFQ_TOP_PRIORITY_NUMBER + 1, /* Highest priority in system */
                &hSchedulerTask);

    /* ---------------------------------------------------------------------
     * 6. Start the Kernel
     * --------------------------------------------------------------------- */
    sendLog("[System] Starting FreeRTOS Scheduler...\r\n");

    /* Set LED to White (All On) briefly to indicate start */
    // Note: You can add a specific LED pattern here if desired

    vTaskStartScheduler();

    /* Code should never reach here unless memory allocation failed */
    while (1)
    {
    }
}


