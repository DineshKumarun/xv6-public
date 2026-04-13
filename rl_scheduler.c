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
  
  // RL Parameters for Q-Learning (Contextual MDP)
  // States: 0 = Low Load (<=2), 1 = Medium Load (<=5), 2 = High Load (>5)
  // Actions: 0 = Max Wait Time (Fairness), 1 = Min Runtime (Throughput/SJF)
  int q_table[3][2];
  for(i = 0; i < 3; i++) {
    q_table[i][0] = 0;
    q_table[i][1] = 0;
  }
  int alpha = 500;   // Learning rate 0.5 - Learn fast!
  int epsilon = 10;  // Exploration rate 0.1 (10/100)
  int action = 0;
  int last_action = -1;
  int current_state = 0;
  int last_state = -1;
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

    // Determine Current State (System Load)
    if (active_procs <= 2) current_state = 0;
    else if (active_procs <= 5) current_state = 1;
    else current_state = 2;
    
    // Reward = Negative Queue Length. 
    // Agent tries to maximize this -> Minimize Queue Length.
    int reward = -active_procs * 100; 

    int gamma = 800; // Discount factor 0.8 (800/1000)

    // Update Q-value for the *previous* state and action using the Full MDP Formula
    if(last_action != -1 && last_state != -1){
      // Find max Q(s', a') for the CURRENT state (which is s')
      int max_q_next = q_table[current_state][0];
      if (q_table[current_state][1] > max_q_next) {
         max_q_next = q_table[current_state][1];
      }
      
      // Standard Formula: Q(s,a) = Q(s,a) + alpha * [ Reward + gamma * max_Q(s') - Q(s,a) ]
      // Handled with integer math (/1000)
      int next_future_reward = (gamma * max_q_next) / 1000;
      int temporal_error = reward + next_future_reward - q_table[last_state][last_action];
      
      q_table[last_state][last_action] = q_table[last_state][last_action] + (alpha * temporal_error) / 1000;
    }
    
    // Select Action (Epsilon-Greedy)
    int r = uptime() % 100;
    if(r < epsilon){
      action = (uptime() % 2); // Explore
    } else {
      if(q_table[current_state][1] >= q_table[current_state][0]) action = 1; // Prefer Throughput/SJF if equal
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
        printf(1, "RL: State=%d, Act=%s, Q_Fair=%d, Q_SJF=%d, Active=%d\n", 
               current_state,
               action==0 ? "Fair" : "SJF", 
               q_table[current_state][0], q_table[current_state][1], active_procs);
      }
      */
    }
    
    last_action = action;
    last_state = current_state;

    sleep(1); // Poll incredibly fast to micromanage jobs
  }
  exit();
}
