#ifndef _PSTAT_H_
#define _PSTAT_H_

#include "param.h"

struct proc_stat {
  int pid;
  int runtime;   // Time running on CPU
  int waittime;  // Time waiting in RUNNABLE state
  int sleeptime; // Time sleeping
  int state;     // Process state
  int size;      // Memory size
};

#endif
