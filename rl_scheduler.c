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
  int q_val[2] = {0, 0}; // Scaled by 1000
  int alpha = 100; // Learning rate 0.1
  int epsilon = 100; // Exploration rate 0.1
  int action = 0;
  int last_action = -1;
  int ticks = 0;

  printf(1, "Starting RL Scheduler Agent...\n");

  while(1){
    ticks++;
    int n = get_proc_stats(stats);
    if(n < 0){
      printf(1, "Error getting stats\n");
      exit();
    }

    // Calculate Reward: Negative Total Wait Time
    // We want to minimize wait time, so maximize -WaitTime.
    // To see improvement, we can look at change in wait time?
    // Or just absolute value.
    int total_wait = 0;
    for(i=0; i<n; i++){
      if(stats[i].state == 3){ // RUNNABLE
        total_wait += stats[i].waittime;
      }
    }
    
    // Reward calculation (simplified)
    // If wait time decreased, positive reward.
    // If wait time increased, negative reward.
    // But wait time monotonically increases.
    // We should look at average wait time or rate of increase.
    // Let's use -total_wait as the signal.
    int reward = -total_wait;

    // Update Q-value for the *previous* action
    if(last_action != -1){
      // Q[a] = Q[a] + alpha * (Reward - Q[a])
      // We scale down reward to avoid overflow, just for demo
      int scaled_reward = reward / 100; 
      q_val[last_action] = q_val[last_action] + (alpha * (scaled_reward - q_val[last_action])) / 1000;
    }
    
    // Select Action (Epsilon-Greedy)
    // Bias towards Throughput (Action 1) for performance
    int r = uptime() % 1000;
    if(r < epsilon){
      action = (uptime() % 2); // Explore
    } else {
      if(q_val[1] >= q_val[0]) action = 1; // Prefer Throughput if equal
      else action = 0;
    }

    // Execute Action
    int best_idx = -1;
    if(action == 0){ // Max Wait Time
      int max_w = -1;
      for(i=0; i<n; i++){
        if(stats[i].state == 3 && stats[i].pid > 2){ 
           if(stats[i].waittime > max_w){
             max_w = stats[i].waittime;
             best_idx = i;
           }
        }
      }
    } else { // Min Runtime
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
      if(ticks % 20 == 0) {
        printf(1, "RL: Action=%s, PID=%d, Reward=%d, Q=[%d, %d]\n", 
               action==0 ? "Fairness" : "Throughput", 
               stats[best_idx].pid, 
               reward, 
               q_val[0], q_val[1]);
      }
      */
    }
    
    last_action = action;

    sleep(10); // Run every 10 ticks to reduce overhead
  }
  exit();
}
