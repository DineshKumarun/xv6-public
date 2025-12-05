#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char **argv)
{
  if(argc != 2){
    printf(2, "usage: sleep pid...\n");
    exit();
  }
  sleep(atoi(argv[1])*100);
  exit();
}
