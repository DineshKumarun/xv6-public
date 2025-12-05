// user/dealloc.c
#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

#define NAME_MAX 16
#define MAX_FILES 1000  

static void mkname(int idx, char *name) {
  name[0]='a'; name[1]='l'; name[2]='l'; name[3]='o'; name[4]='c'; name[5]='.'; 
  int d2 = (idx/100)%10, d1 = (idx/10)%10, d0 = idx%10;
  name[6] = '0' + d2;
  name[7] = '0' + d1;
  name[8] = '0' + d0;
  name[9] = 0;
}

int
main(int argc, char *argv[])
{
  int count = 0;
  char name[NAME_MAX];

  for (int i = 0; i < MAX_FILES; i++) {
    mkname(i, name);
    if (unlink(name) == 0) {
      count++;
    }
  }

  if (count > 0) {
    printf(1, "dealloc: freed %d file(s)\n", count);
  } else {
    printf(1, "dealloc: nothing to free\n");
  }

  exit();
}

