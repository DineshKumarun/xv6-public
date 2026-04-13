#include "types.h"
#include "stat.h"
#include "user.h"
#include "pstat.h"

#define MAX_PROC 64

int
main(int argc, char *argv[])
{
  struct proc_stat stats[MAX_PROC];
  int i;
  
  // RL Parameters for Q-Learning (Bandit)
  // Actions: 0 = Max Wait Time (Fairness), 1 = Min Runtime (Throughput/SJF)
  // We want to learn which policy minimizes the number of active processes (Queue Length).
  int q_val[2] = {0, 0}; 
  int alpha = 500;   // Learning rate 0.5 - Learn fast!
  int epsilon = 10;  // Exploration rate 0.1 (10/100)
  int action = 0;
  int last_action = -1;
  int ticks = 0;

  printf(1, "Starting Optimized RL Scheduler Agent (Throughput Focused)...\n");

  while(1){
    ticks++;
    int n = get_proc_stats(stats);
    if(n < 0){
      printf(1, "Error getting stats\n");
      exit();
    }

    // NEW REWARD FUNCTION: Minimize Active Processes (Queue Length)
    // Little's Law: Minimize N -> Minimize Response Time.
    // Previous logic (Minimize Wait Time) was flawed: picking a long-wait job
    // "hid" it from the wait-sum by moving it to RUNNING, fooling the agent.
    // Now we count both RUNNABLE (3) and RUNNING (4).
    int active_procs = 0;
    for(i=0; i<n; i++){
      if(stats[i].pid > 2 && (stats[i].state == 3 || stats[i].state == 4)){ 
        active_procs++;
      }
    }
    
    // Reward = Negative Queue Length. 
    // Agent tries to maximize this -> Minimize Queue Length.
    int reward = -active_procs * 100; 

    // Update Q-value for the *previous* action
    if(last_action != -1){
      // Q[a] = Q[a] + alpha * (Reward - Q[a])
      q_val[last_action] = q_val[last_action] + (alpha * (reward - q_val[last_action])) / 1000;
    }
    
    // Select Action (Epsilon-Greedy)
    int r = uptime() % 100;
    if(r < epsilon){
      action = (uptime() % 2); // Explore
    } else {
      if(q_val[1] >= q_val[0]) action = 1; // Prefer Throughput/SJF if equal
      else action = 0;
    }

    // Execute Action
    int best_idx = -1;
    if(action == 0){ // Fairness (Max Wait)
      int max_w = -1;
      for(i=0; i<n; i++){
        if(stats[i].state == 3 && stats[i].pid > 2){ 
           if(stats[i].waittime > max_w){
             max_w = stats[i].waittime;
             best_idx = i;
           }
        }
      }
    } else { // Throughput (Min Runtime / SJF)
      int min_r = 2147483647;
      for(i=0; i<n; i++){
        if(stats[i].state == 3 && stats[i].pid > 2){
           if(stats[i].runtime < min_r){
             min_r = stats[i].runtime;
             best_idx = i;
           }
        }
      }
    }

    if(best_idx != -1){
      set_next_process(stats[best_idx].pid);
      /*
      // Debug print occasionally
      if(ticks % 20 == 0) {
        printf(1, "RL: Act=%s, Q_Fair=%d, Q_SJF=%d, Active=%d\n", 
               action==0 ? "Fair" : "SJF", 
               q_val[0], q_val[1], active_procs);
      }
      */
    }
    
    last_action = action;

    sleep(5); // Run more frequently (5 ticks) to catch short jobs
  }
  exit();
}
