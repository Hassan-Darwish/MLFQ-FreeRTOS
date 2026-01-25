#ifndef MLFQ_CONFIG_H_
#define MLFQ_CONFIG_H_

/* ==========================================================================
 * SCENARIO SELECTOR
 * 1: "Honest System" - No gaming. Heavy drops to Red. Interact stays Green.
 * 2: "The Exploit"   - Gaming ON. Heavy tasks cheat and stay Green/Blue.
 * 3: "Stress Test"   - Fast dynamics, frequent boosts, gaming OFF.
 * ========================================================================== */
#define TEST_SCENARIO       1

/* ==========================================================================
 * COMMON CALIBRATION
 * ========================================================================== */
#define WORKLOAD_CYCLES_PER_LOOP    12UL
#ifndef configCPU_CLOCK_HZ
    #define configCPU_CLOCK_HZ      16000000UL
#endif
#define MS_TO_LOOPS(ms)   ( ( (ms) * (configCPU_CLOCK_HZ / 1000UL) ) / WORKLOAD_CYCLES_PER_LOOP )

/* ==========================================================================
 * SCENARIO 1: "The Ideal MLFQ" (Honest Behavior)
 * ========================================================================== */
#if (TEST_SCENARIO == 1)

    #define ENABLE_CPU_GAMING_SIMULATION        0

    #define HEAVY_TASK_ONE_ARRIVAL_TIME         0U
    #define HEAVY_TASK_TWO_ARRIVAL_TIME         5000U
    #define INTERACTIVE_TASK_ONE_ARRIVAL_TIME   7000U
    #define INTERACTIVE_TASK_TWO_ARRIVAL_TIME   10000U

    #define INTERACTIVE_TASK_RUNTIME_MS         100UL
    #define HEAVY_TASK_RUNTIME_MS               30000UL

    #define MLFQ_TIME_SLICE_HIGH                5000U    // Very fast drop
    #define MLFQ_TIME_SLICE_MEDIUM              3000U
    #define MLFQ_TIME_SLICE_LOW                 2000U
    #define MLFQ_BOOST_PERIOD_MS                3600000U   // Frequent Boosts
    #define MLFQ_PRINT_FREQUENCY_MS             2000U

/* ==========================================================================
 * SCENARIO 2: "The Hacker" (Gaming Simulation)
 * ========================================================================== */
#elif (TEST_SCENARIO == 2)

    /* CRITICAL: Gaming is ON. Tasks reset their timer if they get preempted.
     * Since multiple tasks are at Level 0, they preempt each other constantly,
     * resetting their timers and staying Green forever! */
    #define ENABLE_CPU_GAMING_SIMULATION        1

    #define HEAVY_TASK_ONE_ARRIVAL_TIME         0U
    #define HEAVY_TASK_TWO_ARRIVAL_TIME         3000U    // Start together to force preemption
    #define INTERACTIVE_TASK_ONE_ARRIVAL_TIME   7000U
    #define INTERACTIVE_TASK_TWO_ARRIVAL_TIME   10000U

    #define INTERACTIVE_TASK_RUNTIME_MS         50U
    #define HEAVY_TASK_RUNTIME_MS               5000U

    /* Standard slices, but the Gaming Logic will bypass them */
    #define MLFQ_TIME_SLICE_HIGH                1000U
    #define MLFQ_TIME_SLICE_MEDIUM              1500U
    #define MLFQ_TIME_SLICE_LOW                 2500U
    #define MLFQ_BOOST_PERIOD_MS                36000000U
    #define MLFQ_PRINT_FREQUENCY_MS             4000U

/* ==========================================================================
 * SCENARIO 3: "Disco Mode" (Fast & Honest)
 * ========================================================================== */
#elif (TEST_SCENARIO == 3)

    #define ENABLE_CPU_GAMING_SIMULATION        0

    #define HEAVY_TASK_ONE_ARRIVAL_TIME         0U
    #define HEAVY_TASK_TWO_ARRIVAL_TIME         5000U
    #define INTERACTIVE_TASK_ONE_ARRIVAL_TIME   7000U
    #define INTERACTIVE_TASK_TWO_ARRIVAL_TIME   10000U

    #define INTERACTIVE_TASK_RUNTIME_MS         100UL
    #define HEAVY_TASK_RUNTIME_MS               30000UL

    #define MLFQ_TIME_SLICE_HIGH                5000U    // Very fast drop
    #define MLFQ_TIME_SLICE_MEDIUM              3000U
    #define MLFQ_TIME_SLICE_LOW                 2000U
    #define MLFQ_BOOST_PERIOD_MS                30000U   // Frequent Boosts
    #define MLFQ_PRINT_FREQUENCY_MS             4000U

#endif

#define INTERACTIVE_TASK_TIME     MS_TO_LOOPS(INTERACTIVE_TASK_RUNTIME_MS)
#define HEAVY_TASK_TIME           MS_TO_LOOPS(HEAVY_TASK_RUNTIME_MS)
#define TIME_SLICE_THRESHOLD      10U

#endif /* MLFQ_CONFIG_H_ */
