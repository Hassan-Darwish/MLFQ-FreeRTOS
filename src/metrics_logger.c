/******************************************************************************
 * MODULE NAME  : Metrics Logger
 * FILE         : metrics_logger.c
 * DESCRIPTION  : Implementation of logging and reporting logic for the MLFQ
 * scheduler project.
 * AUTHOR       : Yousef Tantawy
 * DATE         : December 2025
 ******************************************************************************/

#include "metrics_logger.h"
#include "drivers.h"        // For sendLog()
#include "scheduler.h"      // For MLFQ definitions and getter
#include "tick_profiler.h"  // For getTaskRuntime()

#include <stdio.h>
#include <string.h>

/* Helper buffer size for log messages */
static char g_logBuffer[LOG_BUFFER_SIZE];

/*
 * Description : Computes latency for a given task.
 * Formula     : Start Time - Arrival Time
 */
float calculateLatency(uint32_t arrivalTick, uint32_t startTick)
{
    if (startTick < arrivalTick) 
    {
        return 0.0f; // Handle potential overflow or invalid input
    }
    return (float)(startTick - arrivalTick);
}

/*
 * Description : Produces a formatted string for UART or report.
 * Format: [Name] | Runtime: [X] ticks | Level: [Y]
 */
char *formatStatsLog(MLFQ_Task_Profiler_t stats)
{
    if (g_logBuffer == NULL) return NULL;
    uint32_t currentTick = xTaskGetTickCount();
    uint32_t totalTimeAlive = currentTick - stats.arrival_tick;
    uint32_t waitingTime = 0;

    if(totalTimeAlive > stats.task_info.run_ticks)
    {
        waitingTime = totalTimeAlive - stats.task_info.run_ticks;
    }
        
    snprintf(g_logBuffer, LOG_BUFFER_SIZE, 
                "%-10s | Lvl: %d | Run: %4lu | Qtm: %2lu | Arr: %5lu | Wait: %4lu\n", 
                pcTaskGetName(stats.task_info.task), /* Use FreeRTOS helper for name string */
                stats.task_level,
                stats.task_info.run_ticks, 
                stats.task_info.quantum_ticks,
                stats.arrival_tick,
                waitingTime);
    // Despite the misleading name, snprintf does not print but rather it saves the output in the mentioned buffer
    
    return g_logBuffer;
}

/*
 * Description : Prints current queue levels and stats for all tasks.
 * It relies on schedulerGetTaskStats (Helper) to bridge
 * the private data in scheduler.c.
 */
void printQueueReport(void)
{
    MLFQ_Task_Profiler_t currentStats;
    
    // 1. Print Header
    sendLog("\n================ MLFQ QUEUE REPORT ================\n");
    sendLog("Name       | Lvl | Run  | Qtm | Arr   | Wait\n");
    sendLog("---------------------------------------------------\n");

    // 2. Iterate through all potential slots
    // We use TICK_PROFILER_MAX_TASKS as defined in tick_profiler.h
    for (uint32_t i = 0; i < TICK_PROFILER_MAX_TASKS; i++)
    {
        // helper function to fetch stats from scheduler.c
        // Returns true if a valid task exists at this index
        if (schedulerGetTaskStats(i, &currentStats)) 
        {
            // Format and Send
            sendLog(formatStatsLog(currentStats));
        }
    }
    
    sendLog("===================================================\n");
}
