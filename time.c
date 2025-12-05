#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char *argv[])
{
  int pid;
  int start_ticks, end_ticks;

  if(argc < 2){
    printf(2, "Usage: time <command> [args...]\n");
    exit();
  }

  start_ticks = uptime();

  pid = fork();
  if(pid < 0){
    printf(2, "time: fork failed\n");
    exit();
  }

  if(pid == 0){
    // Child process
    exec(argv[1], argv + 1);
    printf(2, "time: exec %s failed\n", argv[1]);
    exit();
  }

  // Parent process
  wait();
  
  end_ticks = uptime();
  
  printf(1, "\nReal Time: %d ticks\n", end_ticks - start_ticks);
  
  exit();
}
