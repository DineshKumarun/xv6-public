#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char **argv)
{

  
  int fd1[2];
  int fd2[2];
  char buf[1];
  
  pipe(fd1);
  pipe(fd2);
  
  int pid = fork();
  
  if(pid){
  	close(fd1[0]);
  	close(fd2[1]);
  	
  	write(fd1[1], "a", 1);
  	
  	read(fd2[0], buf, 1);
  	printf(2, "%d: recieved pong\n", getpid());
  	
  	close(fd1[1]);
  	close(fd2[0]);
  	
  	wait();
  	
  	exit();
  }
  else{
  	close(fd1[1]);
  	close(fd2[0]);
  	
  	read(fd1[0], buf, 1);
  	printf(2, "%d: recieved ping\n", getpid());
  	
  	write(fd2[1], "a", 1);
  	
  	close(fd1[0]);
  	close(fd2[1]);
  	
  	exit();
  }
  exit();
}
