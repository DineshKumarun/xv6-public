#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

#define N_CPU_PROCS 2
#define N_IO_PROCS 2
#define CPU_OPS 200000000
#define IO_OPS 1000

void cpu_task(int id) {
  int i;
  volatile int x = 0; // volatile to prevent optimization
  // printf(1, "CPU Task %d started\n", id);
  for(i = 0; i < CPU_OPS; i++) {
    x = x + 1;
    if(x % 1000000 == 0) {
       // Just to burn cycles
       x = x - 1;
       x = x + 1;
    }
  }
  // printf(1, "CPU Task %d finished\n", id);
  exit();
}

void io_task(int id) {
  int i, fd;
  char name[16];
  char data[512];
  
  // printf(1, "IO Task %d started\n", id);
  
  // Create a unique filename
  name[0] = 't';
  name[1] = 'e';
  name[2] = 's';
  name[3] = 't';
  name[4] = id + '0';
  name[5] = 0;

  for(i = 0; i < IO_OPS; i++) {
    fd = open(name, O_CREATE | O_RDWR);
    if(fd >= 0) {
      write(fd, data, sizeof(data));
      close(fd);
    }
    // Delete and recreate to force metadata updates
    if(i % 10 == 0) {
      unlink(name);
    }
  }
  unlink(name);
  // printf(1, "IO Task %d finished\n", id);
  exit();
}

int main(int argc, char *argv[]) {
  int i;
  int pid;

  printf(1, "Starting Benchmark (CPU=%d, IO=%d)...\n", N_CPU_PROCS, N_IO_PROCS);

  // Fork CPU tasks
  for(i = 0; i < N_CPU_PROCS; i++) {
    pid = fork();
    if(pid == 0) {
      cpu_task(i);
    }
  }

  // Fork IO tasks
  for(i = 0; i < N_IO_PROCS; i++) {
    pid = fork();
    if(pid == 0) {
      io_task(i);
    }
  }

  // Wait for all children
  for(i = 0; i < N_CPU_PROCS + N_IO_PROCS; i++) {
    wait();
  }

  printf(1, "Benchmark Finished\n");
  exit();
}
