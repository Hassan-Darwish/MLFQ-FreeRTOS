/******************************************************************************
 * MODULE NAME  : Main Application (Sequenced Start)
 * FILE         : main.c
 ******************************************************************************/

#include "FreeRTOS.h"
#include "MLFQConfig.h"
#include "task.h"
#include "drivers.h"
#include "scheduler.h"
#include "workloads.h"
#include "metrics_logger.h"

/* Global Handles */
TaskHandle_t hTask1_Interactive = NULL;
TaskHandle_t hTask2_Heavy       = NULL;
TaskHandle_t hTask3_Heavy       = NULL;
TaskHandle_t hTask4_Interactive = NULL;
TaskHandle_t hSchedulerTask     = NULL;

/* * SCENARIO LOADER TASK
 * This task acts like a "Director". It wakes up periodically to
 * launch new tasks, simulating users opening apps at different times.
 */
void vScenarioLoader(void *pvParameters)
{
    vTaskDelay(pdMS_TO_TICKS(HEAVY_TASK_ONE_ARRIVAL_TIME));

    xTaskCreate(runCPUHeavyTask, "Heavy_2", 256, (void*)"Heavy_2", MLFQ_TOP_PRIORITY_NUMBER, &hTask2_Heavy);
    registerTask(hTask2_Heavy);

    vTaskDelay(pdMS_TO_TICKS(HEAVY_TASK_TWO_ARRIVAL_TIME));


    xTaskCreate(runCPUHeavyTask, "Heavy_3", 256, (void*)"Heavy_3", MLFQ_TOP_PRIORITY_NUMBER, &hTask3_Heavy);
    registerTask(hTask3_Heavy);


    vTaskDelay(pdMS_TO_TICKS(INTERACTIVE_TASK_ONE_ARRIVAL_TIME));


    xTaskCreate(runInteractiveTask, "Interact_1", 256, (void*)"Interact_1", MLFQ_TOP_PRIORITY_NUMBER, &hTask1_Interactive);
    registerTask(hTask1_Interactive);

    vTaskDelay(pdMS_TO_TICKS(INTERACTIVE_TASK_TWO_ARRIVAL_TIME));


    xTaskCreate(runInteractiveTask, "Interact_4", 256, (void*)"Interact_4", MLFQ_TOP_PRIORITY_NUMBER, &hTask4_Interactive);
    registerTask(hTask4_Interactive);


    vTaskDelete(NULL);
}

int main(void)
{
    initUART();
    initGPIO();
    initScheduler();

    sendLog("\n\n************************************************\r\n");
    sendLog("* MLFQ SEQUENCED START DEMO                    *\r\n");
    sendLog("************************************************\r\n");

    /* 1. Create the MLFQ Scheduler Manager (Priority 6) */
    xTaskCreate(schedulerTask, "Scheduler", 512, NULL, MLFQ_TOP_PRIORITY_NUMBER + 1, &hSchedulerTask);

    /* 2. Create the Scenario Loader (Priority 7 - Highest)
     * We give it the highest priority so it runs immediately to spawn the first task.
     */
    xTaskCreate(vScenarioLoader, "Loader", 512, NULL, MLFQ_TOP_PRIORITY_NUMBER + 2, NULL);

    sendLog("[System] Starting Kernel...\r\n");
    vTaskStartScheduler();

    while (1) {}
}
