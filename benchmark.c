#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

#define N_MASSIVE 1
#define N_MICRO 20
#define MASSIVE_OPS 400000000
#define MICRO_OPS 30000000

void cpu_task(int id, int ops) {
  int i;
  volatile int x = 0; // volatile to prevent optimization
  for(i = 0; i < ops; i++) {
    x = x + 1;
    if(x % 1000000 == 0) {
       x = x - 1;
       x = x + 1;
    }
  }
  exit();
}

int main(int argc, char *argv[]) {
  int i;
  int pid;

  printf(1, "Starting Asymmetric Benchmark (1 Massive, 20 Micro)...\n");

  // Fork Massive task
  for(i = 0; i < N_MASSIVE; i++) {
    pid = fork();
    if(pid == 0) {
      cpu_task(i, MASSIVE_OPS);
    }
  }

  // Fork Micro tasks
  for(i = 0; i < N_MICRO; i++) {
    pid = fork();
    if(pid == 0) {
      cpu_task(N_MASSIVE + i, MICRO_OPS);
    }
  }

  int start = uptime();
  
  // Wait for all children and calculate turnaround time
  int total_turnaround = 0;
  for(i = 0; i < N_MASSIVE + N_MICRO; i++) {
    wait();
    int turn = uptime() - start;
    total_turnaround += turn;
  }

  printf(1, "Total Benchmark Time: %d ticks\n", uptime() - start);
  printf(1, "Average Process Turnaround Time: %d ticks\n", total_turnaround / (N_MASSIVE + N_MICRO));
  exit();
}
