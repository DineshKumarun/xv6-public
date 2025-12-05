#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "pstat.h"

int
sys_fork(void)
{
  return fork();	
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return myproc()->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

int
sys_trace(void){
	int n;
	if(argint(0, &n) < 0)
    	return -1;
	
	myproc() -> trace_mask = n;
	return 0;
	
}


int
sys_countsys(void){
	int n;
	if(argint(0, &n) < 0)
    	return -1;
	
	myproc() -> count_mask = n;
	
	//for(int i = 0; i <= 24; i++)
		//myproc() -> syscall_count[i] = 0;
	return 0;
	
}
int
sys_printcount(void){
    struct proc *p = myproc();
    //cprintf("Print called\n");
    for (int i = 0; i < MAXSYSCALL; i++) {
    	//cprintf("Count: %d\n", p->syscall_count[i]);
    	
        if (p->syscall_count[i] > 0) {
            cprintf("Process: %d (%s) syscall %d called %d times\n",
                    p->pid, p->name, i, p->syscall_count[i]);
        }
    }
    return 0;
}

int sys_pgtbl(void){
	struct proc *p = myproc();
	
	vmprint(p->pgdir);
	return 0;
}

int
sys_get_proc_stats(void)
{
  struct proc_stat *st;
  if(argptr(0, (void*)&st, sizeof(struct proc_stat) * NPROC) < 0)
    return -1;
  return get_proc_stats(st);
}

int
sys_set_next_process(void)
{
  int pid;
  if(argint(0, &pid) < 0)
    return -1;
  return set_next_process(pid);
}
