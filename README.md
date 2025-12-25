# FreeRTOS Multi-Level Feedback Queue (MLFQ) Scheduler

**Target:** Tiva‑C LaunchPad (TM4C123GH6PM)

An advanced, dynamic scheduling algorithm implemented on top of the FreeRTOS kernel. This project replaces the standard fixed‑priority logic with a Multi‑Level Feedback Queue (MLFQ) system that automatically classifies tasks as **Interactive** or **CPU‑Bound** to optimize responsiveness and fairness.

---

## 📖 Table of Contents

1. [Project Overview](#project-overview)
2. [Key Features](#key-features)
3. [System Architecture](#system-architecture)
4. [Hardware Setup](#hardware-setup)
5. [Configuration](#configuration)
6. [Performance Analysis](#performance-analysis)
7. [File Structure](#file-structure)
8. [Credits](#credits)

---

# 🚀 Project Overview

Standard FreeRTOS uses a fixed‑priority preemptive scheduler. While deterministic, it requires developers to manually tune priorities. If a high‑priority task hogs the CPU, lower‑priority tasks can starve.

This MLFQ Scheduler solves that by:

* **Dynamic Prioritization:** Tasks change priority based on observed behavior.
* **Favoring Interactivity:** I/O‑bound tasks (user input, sensors) stay at high priority for instant response.
* **Punishing CPU Hogs:** Long‑running computations are automatically demoted to lower queues to prevent monopolization.
* **Preventing Starvation:** A **Global Boost** periodically resets all tasks to the highest priority.

---

# ✨ Key Features

| Feature             | Description                                                           |
| ------------------- | --------------------------------------------------------------------- |
| **3‑Level Queue**   | High (Priority 4), Medium (Priority 3), Low (Priority 2).             |
| **Tick Profiler**   | Hooks into `vApplicationTickHook` to measure per‑task CPU usage.      |
| **Aging & Boost**   | Periodically promotes all tasks to prevent starvation.                |
| **Visual Feedback** | Onboard RGB LED indicates the priority of the currently running task. |
| **A/B Testing**     | Built‑in test runner to compare MLFQ vs. standard FreeRTOS.           |
| **Metrics Logging** | Real‑time CSV performance data via UART.                              |

---

# ⚙️ System Architecture

### The MLFQ Logic

* The scheduler logic is **decoupled** from user workloads and runs as a `Supervisor Task` (`schedulerTask`) plus a hardware tick hook.
* **New tasks** enter at the **High Priority Queue**.
* **Demotion**: If a task exhausts its time slice without blocking (i.e., it continuously runs), it is flagged as *CPU Heavy* and demoted one level.
* **Interactive Tasks**: If a task yields (e.g., calls `vTaskDelay`) before its time slice ends, it retains or is restored to high priority.
* **Global Boost**: Every `MLFQ_BOOST_PERIOD_MS` (example: 5 s) all tasks are promoted back to High to ensure fairness.

### Visual Feedback (RGB LED)

* **🔵 Blue:** High Priority (Interactive / New Tasks)
* **🟢 Green:** Medium Priority
* **🔴 Red:** Low Priority (Background / CPU Heavy Tasks)

---

# 🔌 Hardware Setup

* **Microcontroller:** Tiva‑C LaunchPad (TM4C123GH6PM)
* **Interface:** UART0 (USB Virtual COM Port)
* **Baud Rate:** 115200
* **Data Format:** 8‑N‑1

### Pin Connections

| Component | Tiva Pin | Function                  |
| --------- | -------: | ------------------------- |
| UART0 RX  |      PA0 | Serial Logging (Input)    |
| UART0 TX  |      PA1 | Serial Logging (Output)   |
| RGB Red   |      PF1 | Low Priority Indicator    |
| RGB Green |      PF2 | Medium Priority Indicator |
| RGB Blue  |      PF3 | High Priority Indicator   |

---

# 🛠 Configuration

The system behavior is controlled via macros in `scheduler.h` and `test_config.h`.

### 1. Tuning the Scheduler (`scheduler.h`)

```c
// Time in ms before a global reset occurs
#define MLFQ_BOOST_PERIOD_MS    5000

// Maximum tick count allowed before demotion
#define TIME_SLICE_THRESHOLD    10
```

### 2. Switching Test Modes (`test_config.h`)

```c
// 0 = STANDARD_MODE: Fixed priorities (Round Robin).
// 1 = MLFQ_MODE:     Dynamic priorities (MLFQ).
#define TEST_MODE  1
```

---

# 📊 Performance Analysis

A/B Test comparing Standard FreeRTOS vs. MLFQ implementation.

* **Workload:** 1 CPU Heavy Task (math loop) + 1 Interactive Task (simulated I/O) running concurrently.
* **Duration:** 2000 ms.
* **Measurement:** CPU operations per 100 ms window (Throughput).

| Metric                      | Standard FreeRTOS | MLFQ Scheduler |
| --------------------------- | ----------------: | -------------: |
| CPU Throughput (ops/window) |             15.70 |          15.60 |
| Efficiency                  |   100% (Baseline) |         99.36% |
| Overhead Cost               |                0% |          0.64% |

**Conclusion:** The MLFQ logic introduces a negligible ~0.64% CPU overhead while greatly improving responsiveness for interactive tasks. It preserves ~99.36% of raw processing capacity while ensuring interactive work is not blocked by CPU hogs.

---

# 📂 File Structure

```
/src
├── main.c              # Entry point & Test Runner
├── scheduler.c         # MLFQ Core Logic (Demotion, Promotion, Tables)
├── scheduler.h         # Configuration & Defines
├── drivers.c           # Tiva-C UART & GPIO Drivers
├── workloads.c         # Simulated Tasks (CPU Heavy vs Interactive)
├── metrics_logger.c    # Stats formatting helpers
├── FreeRTOSConfig.h    # Kernel configuration
└── startup_tm4c.c      # Startup code & Interrupt Vectors
```

---

# 👥 Credits

**Development Team**

* Hassan Darwish
* Elham Karam
* Omar Ashraf
* Yousef Tantawy
* Ahmed Alaa

**Dependencies**

* FreeRTOS Kernel v10.x
* Texas Instruments TivaWare Driver Library

