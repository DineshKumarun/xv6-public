# Project Report: ML-Driven Scheduler (Reinforcement Learning) in xv6

## 1. Introduction
This project implements a **Reinforcement Learning (RL) based Process Scheduler** in the xv6 operating system. The goal is to replace the static Round-Robin scheduler with a dynamic agent that learns to optimize system performance (balancing fairness and throughput) using a Q-Learning algorithm.

---

## 2. Implementation Overview

*   **User-Space Agent**: The scheduling logic resides in a user program (`rl_scheduler`), not the kernel.
*   **Q-Learning Algorithm**: The agent learns from the environment (OS) by observing states (process wait/run times) and receiving rewards.
*   **Kernel Hooks**: The kernel is modified to:
    *   Track detailed process statistics (`rtime`, `wtime`, `stime`).
    *   Expose these statistics to the user agent.
    *   Allow the user agent to override the default scheduler.
*   **Performance Metrics**:
    *   **Fairness**: Measured by minimizing Wait Time.
    *   **Throughput**: Measured by minimizing Run Time (Shortest Job First).

---

## 3. List of Modified & Created Files

The following files were modified or created to implement the RL Scheduler.

### 1. `rl_scheduler.c` (New File)
*   **Purpose**: The "Brain" of the scheduler.
*   **Logic**:
    *   Runs as a background daemon.
    *   Fetches process stats via `get_proc_stats()`.
    *   Updates Q-Table based on reward: $R = -(WaitTime)$.
    *   Selects next process using $\epsilon$-Greedy policy.
    *   Enforces decision via `set_next_process()`.

**[Paste Screenshot of `rl_scheduler.c` Here]**
<br><br><br>
<br><br><br>

---

### 2. `proc.h` (Modified)
*   **Purpose**: Process structure definition.
*   **Changes**:
    *   Added `uint rtime` (Total Run Time).
    *   Added `uint wtime` (Total Wait Time).
    *   Added `uint stime` (Total Sleep Time).
    *   Defined `struct proc_stat` for user-space communication.

**[Paste Screenshot of `proc.h` Changes Here]**
<br><br><br>
<br><br><br>

---

### 3. `proc.c` (Modified)
*   **Purpose**: Kernel process management.
*   **Changes**:
    *   `scheduler()`: Added logic to check `enforced_pid` and run it immediately.
    *   `update_proc_stats()`: New function to increment `wtime`/`rtime` every tick.
    *   `set_next_process()`: Sets the `enforced_pid`.
    *   `get_proc_stats()`: Copies process data to user space.

**[Paste Screenshot of `proc.c` Changes Here]**
<br><br><br>
<br><br><br>

---

### 4. `trap.c` (Modified)
*   **Purpose**: Interrupt handling.
*   **Changes**:
    *   Calls `update_proc_stats()` inside the Timer Interrupt handler (`case T_IRQ0 + IRQ_TIMER`).

**[Paste Screenshot of `trap.c` Changes Here]**
<br><br><br>
<br><br><br>

---

### 5. `sysproc.c` (Modified)
*   **Purpose**: System call implementations.
*   **Changes**:
    *   Implemented `sys_get_proc_stats()`.
    *   Implemented `sys_set_next_process()`.

**[Paste Screenshot of `sysproc.c` Changes Here]**
<br><br><br>
<br><br><br>

---

### 6. `syscall.h` (Modified)
*   **Purpose**: System call numbers.
*   **Changes**:
    *   Defined `SYS_get_proc_stats`.
    *   Defined `SYS_set_next_process`.

**[Paste Screenshot of `syscall.h` Changes Here]**
<br><br><br>
<br><br><br>

---

### 7. `syscall.c` (Modified)
*   **Purpose**: System call table.
*   **Changes**:
    *   Added function prototypes.
    *   Added entries to the `syscalls[]` array.

**[Paste Screenshot of `syscall.c` Changes Here]**
<br><br><br>
<br><br><br>

---

### 8. `user.h` (Modified)
*   **Purpose**: User-space function declarations.
*   **Changes**:
    *   Declared `int get_proc_stats(struct proc_stat*);`
    *   Declared `int set_next_process(int);`

**[Paste Screenshot of `user.h` Changes Here]**
<br><br><br>
<br><br><br>

---

### 9. `usys.S` (Modified)
*   **Purpose**: System call assembly stubs.
*   **Changes**:
    *   Added `SYSCALL(get_proc_stats)`
    *   Added `SYSCALL(set_next_process)`

**[Paste Screenshot of `usys.S` Changes Here]**
<br><br><br>
<br><br><br>

---

### 10. `defs.h` (Modified)
*   **Purpose**: Kernel function definitions.
*   **Changes**:
    *   Added prototypes for `update_proc_stats`, `set_next_process`, `get_proc_stats`.

**[Paste Screenshot of `defs.h` Changes Here]**
<br><br><br>
<br><br><br>

---

### 11. `Makefile` (Modified)
*   **Purpose**: Compilation rules.
*   **Changes**:
    *   Added `_rl_scheduler`, `_benchmark`, and `_time` to `UPROGS`.

**[Paste Screenshot of `Makefile` Changes Here]**
<br><br><br>
<br><br><br>

---

### 12. `benchmark.c` (New File)
*   **Purpose**: Testing and Validation.
*   **Logic**:
    *   Spawns CPU-bound processes (heavy computation).
    *   Spawns I/O-bound processes (file operations).
    *   Used to generate traffic for the RL agent to manage.

**[Paste Screenshot of `benchmark.c` Here]**
<br><br><br>
<br><br><br>

---

### 13. `time.c` (New File)
*   **Purpose**: Performance Measurement.
*   **Logic**:
    *   Executes a command and reports the number of ticks it took.

**[Paste Screenshot of `time.c` Here]**
<br><br><br>
<br><br><br>

---

## 4. How to Run

1.  **Compile the Kernel**:
    ```bash
    make qemu
    ```

2.  **Start the RL Scheduler**:
    Inside the xv6 shell:
    ```bash
    rl_scheduler &
    ```

3.  **Run the Benchmark**:
    ```bash
    benchmark
    ```

---

## 6. Conclusion
This project successfully demonstrates the feasibility of integrating Machine Learning into an operating system's process scheduler. By moving the scheduling logic to user space and utilizing a Reinforcement Learning agent, the system can dynamically adapt to varying workloads.

**Key Takeaways:**
*   **Flexibility:** The user-space agent allows for complex scheduling algorithms without bloating the kernel.
*   **Adaptability:** The Q-Learning model learns to prioritize processes based on real-time system performance (Wait Time vs. Run Time).
*   **Extensibility:** The framework provides a solid foundation for experimenting with more advanced ML models (e.g., Deep Q-Networks) or different reward functions.

While the current implementation uses a basic tabular Q-Learning approach, it proves that intelligent, data-driven decisions can be made at the OS level to optimize resource allocation.

