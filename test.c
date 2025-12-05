#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char **argv)
{
  int id = fork();
  if(id == 0)
  {
  	  fork();
  	  fork();
  }
	   wait();
  exit();
}
