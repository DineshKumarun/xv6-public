# Understanding the RL Scheduler in xv6

This document explains the **Reinforcement Learning (RL) Scheduler** implemented in this project. It breaks down the concepts, the code, and how the system learns to manage processes efficiently.

---

## 1. The Concept: A "Smart" Traffic Cop

Imagine a traffic cop at a busy intersection.
*   **Traditional Scheduler (Round Robin):** The cop just waves one car through from each lane in a fixed circle. Lane 1, Lane 2, Lane 3, repeat. It doesn't care if Lane 1 has an ambulance or if Lane 3 is empty.
*   **RL Scheduler:** This cop **learns**. It watches the traffic. It tries different strategies:
    *   *Strategy A:* Let the car that has been waiting the longest go first (**Fairness**).
    *   *Strategy B:* Let the fastest cars go first to clear the road quickly (**Throughput**).

After trying a strategy, the cop looks at how angry the drivers are (Total Wait Time). If drivers are happy (low wait time), the cop remembers "Strategy B was good!" and uses it more often.

---

## 2. Mapping RL Terms to Operating Systems

In our code, we map standard Reinforcement Learning terms to OS concepts:

| RL Term | OS Equivalent | Description |
| :--- | :--- | :--- |
| **Agent** | `rl_scheduler.c` | A user-space program that runs in the background. |
| **Environment** | The xv6 Kernel | The operating system managing processes. |
| **State** | Process Stats | The current `waittime` and `runtime` of all processes. |
| **Action** | Scheduling Policy | **Action 0**: Pick process with Max Wait Time.<br>**Action 1**: Pick process with Min Runtime. |
| **Reward** | `-TotalWaitTime` | We want to *minimize* wait time. In RL, we maximize reward, so we use negative wait time. |

---

## 3. Code Walkthrough: `rl_scheduler.c`

The file `rl_scheduler.c` is the brain of the operation. Here is what it does, step-by-step:

### Step 1: Setup and Parameters
```c
// Q-Values: The "memory" of how good each action is.
// q_val[0] is for Fairness, q_val[1] is for Throughput.
int q_val[2] = {0, 0}; 

// Epsilon: How often to explore random actions (10%).
int epsilon = 100; 
```
The agent initializes its memory (`q_val`) to zero. It doesn't know which strategy is better yet.

### Step 2: The Infinite Loop (The "Life" of the Agent)
The agent runs inside a `while(1)` loop. It never stops as long as the OS is running.

### Step 3: Observe the State
```c
int n = get_proc_stats(stats);
```
The agent asks the kernel: *"Give me the list of all processes and their current wait times."*

### Step 4: Calculate Reward
```c
int total_wait = 0;
for(i=0; i<n; i++){
  if(stats[i].state == RUNNABLE){
    total_wait += stats[i].waittime;
  }
}
int reward = -total_wait;
```
The agent calculates how "bad" the current situation is. High total wait time = High negative reward (Bad). Low wait time = Low negative reward (Good).

### Step 5: Learn (Update Q-Values)
```c
// Update the score of the LAST action we took based on the CURRENT reward.
q_val[last_action] = q_val[last_action] + alpha * (reward - q_val[last_action]);
```
This is the **Q-Learning equation**. It updates the score of the previous strategy.
*   If the previous strategy led to lower wait times, its score (`q_val`) goes up.
*   If it led to traffic jams, its score goes down.

### Step 6: Choose Next Action (Epsilon-Greedy)
```c
if(random < epsilon) {
    action = random; // Explore: Try something random just to see what happens.
} else {
    // Exploit: Pick the strategy with the highest score.
    if(q_val[1] >= q_val[0]) action = 1; // Prefer Throughput
    else action = 0; // Prefer Fairness
}
```
Most of the time, the agent picks the winner. But 10% of the time, it tries a random strategy to see if the situation has changed.

### Step 7: Execute the Decision
Based on the chosen action (0 or 1), the agent looks through the list of processes (`stats`) and picks a specific `pid` (Process ID).

*   **If Action 0 (Fairness):** It finds the PID with the highest `waittime`.
*   **If Action 1 (Throughput):** It finds the PID with the lowest `runtime` (Shortest Job First).

```c
set_next_process(stats[best_idx].pid);
```
Finally, it tells the kernel: *"Run this PID next!"*

### Step 8: Sleep
```c
sleep(10);
```
The agent goes to sleep for a short time (10 ticks) to let the chosen process run. If it ran continuously, it would use up all the CPU itself!

---

## 4. How the Kernel Helps (`proc.c`)

The agent is just a user program. It needs the Kernel to actually enforce its decisions. We modified `proc.c` to listen to the agent.

1.  **`enforced_pid`**: We added a global variable in the kernel.
2.  **`sys_set_next_process(pid)`**: When the agent calls this, it sets `enforced_pid = pid`.
3.  **`scheduler()`**: The main kernel scheduler loop was changed:
    ```c
    if(enforced_pid > 0) {
        // Find the enforced process and run it IMMEDIATELY.
        // Ignore the usual order.
    }
    ```

## 5. Summary

1.  **Agent wakes up.**
2.  **Checks traffic** (process stats).
3.  **Grades previous performance** (updates Q-table).
4.  **Picks a strategy** (Fairness vs Throughput).
5.  **Tells Kernel** "Run Process X next".
6.  **Kernel obeys** and runs Process X.
7.  **Agent sleeps** and repeats.

Over time, the agent learns that for CPU-heavy workloads (like our benchmark), **Throughput (Action 1)** usually yields better rewards, so it learns to behave like a Shortest-Job-First scheduler automatically.
