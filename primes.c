#include "types.h"
#include "stat.h"
#include "user.h"

void sieve(int in_fd){
	int p;
	
	if(read(in_fd, &p, sizeof(p)) <= 0){
		exit();
	}
	
	printf(1, "prime %d\n", p);
	
	int pipe_fd[2];
	
	if(pipe(pipe_fd) < 0)
		exit();
	
	int pid = fork();
	
	if(pid < 0)
		exit();
	if(pid == 0){
		close(pipe_fd[1]);
		close(in_fd);
		sieve(pipe_fd[0]);
		//exit();
	}
	else{
		close(pipe_fd[0]);
		
		int x;
		while(read(in_fd, &x, sizeof(x)) > 0){
			if(x%p != 0)
				write(pipe_fd[1], &x, sizeof(x));
		}
		close(pipe_fd[1]);
		close(in_fd);
		
		wait();
		exit();
	}
	
}

int
main(int argc, char **argv)
{
	int fd[2];
	
	if(pipe(fd) < 0){
		exit();
	}
	
	int pid = fork();
	
	if(pid < 0){
	exit();
	}
	if(pid == 0){
		close(fd[1]);
		sieve(fd[0]);
		exit();
	}
	
	else{
		close(fd[0]);
		for(int i = 2; i <= 1000; i++){
			write(fd[1], &i, sizeof(i));
		}
		close(fd[1]);
		wait();
		exit();
	}
}
