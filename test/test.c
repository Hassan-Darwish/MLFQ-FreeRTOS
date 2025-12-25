/******************************************************************************
 * FILE         : main.c
 * DESCRIPTION  : Test Runner for MLFQ vs Standard Scheduler A/B Testing.
 * Outputs throughput data via UART in CSV format.
 ******************************************************************************/

#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* FreeRTOS Includes */
#include "FreeRTOS.h"
#include "task.h"

/* Project Includes */
#include "scheduler.h"    // MLFQ Logic
#include "workloads.h"    // CPU Heavy & Interactive Tasks
#include "test_config.h"  // Switches between Test Modes (0 or 1)
#include "drivers.h"      // Tiva-C UART & GPIO Drivers

/* Task Handles */
TaskHandle_t xHeavyHandle = NULL;
TaskHandle_t xInteractHandle = NULL;
TaskHandle_t hSchedulerTask     = NULL;


/* * MONITOR TASK
 * Description : Runs every 1 second. Calculates the "Loop Count" (Throughput)
 * of the other tasks and sends a CSV line over UART.
 * * CSV Format  : Time(ms), TestMode, CpuHeavy_Ops/Sec, Interactive_Ops/Sec
 */
void vMonitorTask(void *pvParameters)
{
    /* Access global counters from workloads.c */
    extern volatile uint32_t g_cpu_work_counter;
    extern volatile uint32_t g_interactive_work_counter;

    static uint32_t last_cpu_count = 0;
    static uint32_t last_inter_count = 0;
    char buffer[128];

    /* Send CSV Header for Excel/Python */
    sendLog("\r\n--- TEST STARTED ---\r\n");
    sendLog("Time_MS, Mode, Heavy_Ops, Inter_Ops\r\n");

    for(;;)
    {
        /* Wait 1 second */
        vTaskDelay(pdMS_TO_TICKS(1000));

        /* Snapshot current counters */
        uint32_t current_cpu = g_cpu_work_counter;
        uint32_t current_inter = g_interactive_work_counter;

        /* Calculate delta (Operations per Second) */
        uint32_t cpu_speed = (current_cpu - last_cpu_count);
        uint32_t inter_speed = (current_inter - last_inter_count);

        TickType_t now = xTaskGetTickCount();

        /* Format Data: Time, Mode, CPU_Speed, User_Speed */
        /* Note: TEST_MODE comes from test_config.h */
        snprintf(buffer, sizeof(buffer), "%lu, %d, %lu, %lu\r\n",
                 now, TEST_MODE, cpu_speed, inter_speed);

        /* Send to PC via UART */
        sendLog(buffer);

        #if (TEST_MODE == 1)
             /* Optional: If in MLFQ mode, you can also print the queue report
                to see tasks moving between queues. */
             // printQueueReport();
        #endif

        /* Update history */
        last_cpu_count = current_cpu;
        last_inter_count = current_inter;
    }
}

/*
 * MAIN FUNCTION
 * Entry point for the Test Runner.
 */
int main(void)
{
    /* 1. Initialize Tiva-C Hardware */
    initUART();
    initGPIO();

    /* 2. Initialize Scheduler Internal Structures */
    initScheduler();

    /* 3. Create the Monitor Task (The Observer) */
    /* Priority 5 ensures it always runs to print stats, regardless of CPU load */
    xTaskCreate(vMonitorTask, "Monitor", 1024, NULL, 5, NULL);

    /* 4. Configure the Scheduler based on Test Mode */
    #if (TEST_MODE == 1)
        /* ---------------------------------------------------------
         * MODE: MLFQ SCHEDULER (Experimental)
         * --------------------------------------------------------- */
        sendLog("[INFO] System Mode: MLFQ (Dynamic Priority)\r\n");

        /* Create the Supervisor Task (The MLFQ Manager) */
        xTaskCreate(schedulerTask,
                    "Scheduler",
                    1024,                       /* Stack size increased for safe printing */
                    NULL,
                    MLFQ_TOP_PRIORITY_NUMBER + 1, /* Highest priority in system */
                    &hSchedulerTask);
        /* Create Workloads */
        /* 256 is plenty for these simple tasks */
        xTaskCreate(runCPUHeavyTask, "Hog", 256, "Hog", 4, &xHeavyHandle);
        xTaskCreate(runInteractiveTask, "User", 256, "User", 4, &xInteractHandle);

        if (xHeavyHandle != NULL) {
            registerTask(xHeavyHandle);
            registerTask(xInteractHandle);
        }


    #else
        /* ---------------------------------------------------------
         * MODE: STANDARD FREE RTOS (Control Group)
         * --------------------------------------------------------- */
        sendLog("[INFO] System Mode: STANDARD (Round Robin)\r\n");

        /* Create Workloads at EQUAL Priority (4) to simulate contention */
        xTaskCreate(runCPUHeavyTask, "Hog", 1024, "Hog", 4, &xHeavyHandle);
        xTaskCreate(runInteractiveTask, "User", 1024, "User", 4, &xInteractHandle);

        /* DO NOT Register them. Standard FreeRTOS handles them naturally. */
    #endif

    /* 5. Start the Kernel */
    sendLog("[INFO] Starting Scheduler...\r\n");
    vTaskStartScheduler();

    /* Should never reach here */
    while(1);
}
