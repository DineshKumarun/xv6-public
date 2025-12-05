# RL-Driven Scheduler for xv6

This document details the implementation of a Reinforcement Learning (RL) based scheduler in xv6 and provides instructions on how to verify its performance.

## 1. Implementation Details

The standard xv6 Round-Robin scheduler has been augmented with a user-space RL agent that dynamically selects processes based on a Q-Learning algorithm.

### A. Kernel Modifications
To enable the user-space agent to make scheduling decisions, several hooks were added to the kernel:

1.  **Process Statistics (`struct proc` in `proc.h`)**:
    *   Added fields to track metrics: `rtime` (Total Run Time), `wtime` (Total Wait Time), and `stime` (Sleep Time).
    *   **`trap.c`**: Modified the timer interrupt handler to call `update_proc_stats()`, which increments `wtime` for RUNNABLE processes and `rtime` for RUNNING processes every tick.

2.  **New System Calls**:
    *   `sys_get_proc_stats()`: Exports the state (PID, state, run time, wait time) of all active processes to the user agent.
    *   `sys_set_next_process(pid)`: Allows the user agent to dictate which process runs next.

3.  **Scheduler Logic (`proc.c`)**:
    *   The `scheduler()` loop was modified to check for a global variable `enforced_pid`.
    *   If `enforced_pid` is set (by the agent), the scheduler searches for that specific process and runs it immediately, bypassing the standard Round-Robin order.

### B. User-Space RL Agent (`rl_scheduler.c`)
The agent runs as a background daemon and performs the following loop:
1.  **Observe**: Queries the kernel for process stats (`get_proc_stats`).
2.  **Learn**: Updates Q-values based on a reward function (minimizing total wait time).
3.  **Decide**: Selects an action using an Epsilon-Greedy policy:
    *   **Action 0 (Fairness)**: Selects the process with the highest Wait Time.
    *   **Action 1 (Throughput)**: Selects the process with the lowest Run Time (Shortest Job First).
4.  **Act**: Calls `set_next_process(pid)` to enforce the decision.

### C. Benchmarking Tools
*   **`benchmark.c`**: A synthetic workload generator that spawns:
    *   2 CPU-bound processes (calculating heavy math).
    *   2 I/O-bound processes (writing/deleting files).
*   **`time.c`**: A command-line utility to measure the execution time (in ticks) of any command.

---

## 2. How to Run and Verify

Follow these steps to compare the default Round-Robin scheduler against the RL scheduler.

### Step 1: Compile and Start xv6
Build the kernel and start QEMU:
```bash
make qemu
```

### Step 2: Establish a Baseline (Default Scheduler)
Inside the xv6 shell, run the benchmark using the default scheduler. This gives you the "control" metric.

```bash
$ time benchmark
```
*   **Output**: Note the `Real Time` value (e.g., `306 ticks`).
*   *Note: Without the RL agent running, the kernel falls back to standard Round-Robin scheduling.*

### Step 3: Start the RL Scheduler
Launch the RL agent in the background. It will begin monitoring processes and enforcing scheduling decisions.

```bash
$ rl_scheduler &
```
*   **Output**: `Starting RL Scheduler Agent...`

### Step 4: Run the Benchmark with RL
Run the same benchmark again. The RL agent is now actively managing the CPU queue.

```bash
$ time benchmark
```
*   **Output**: Compare the `Real Time` value with the baseline.

### Step 5: Interpret Results
*   **Lower Time**: The RL scheduler successfully optimized the workload (likely by prioritizing Shortest Jobs/Throughput).
*   **Higher Time**: The overhead of the user-space agent (context switching, system calls) outweighed the scheduling benefits.
    *   *Optimization Note*: The agent is configured to `sleep(10)` ticks between decisions to minimize this overhead.

---

## 3. Troubleshooting
*   If `time benchmark` fails immediately with `exec: fail`, ensure you are not running out of memory. You may need to restart QEMU (`CTRL+A`, `X` to exit).
*   If the RL agent seems stuck, you can kill QEMU and restart. The agent is a simple infinite loop daemon.
