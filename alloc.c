// user/alloc.c
#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

#define KB_PER_FILE 60          // 60KB per file to stay under xv6 max-file-size limits
#define CHUNK_BYTES 512         // write in 512-byte chunks (portable)
#define NAME_MAX 16

// make name "alloc.%03d" without snprintf (xv6 libc is tiny)
static void mkname(int idx, char *name) {
  // base = "alloc."
  name[0]='a'; name[1]='l'; name[2]='l'; name[3]='o'; name[4]='c'; name[5]='.'; 
  int d2 = (idx/100)%10, d1 = (idx/10)%10, d0 = idx%10;	
  name[6] = '0' + d2;
  name[7] = '0' + d1;
  name[8] = '0' + d0;
  name[9] = 0;
}

static int write_kb(int fd, int kb) {
  // write kb * 1024 bytes, in 512-byte chunks => 2 * kb writes
  static char buf[CHUNK_BYTES];
  // fill with non-zero to force real allocation; zeros also work, but either is fine
  for (int i = 0; i < CHUNK_BYTES; i++) buf[i] = (char)(i & 0xFF);

  int writes = kb * (1024 / CHUNK_BYTES); // 2 * kb
  for (int i = 0; i < writes; i++) {
    int n = write(fd, buf, sizeof(buf));
    if (n != sizeof(buf)) {
      // short write or error => stop and signal failure
      return -1;
    }
  }
  return 0;
}

int
main(int argc, char *argv[])
{
  if (argc != 2) {
    printf(2, "usage: alloc N   (N in KB)\n");
    exit();
  }

  int total_kb = atoi(argv[1]);
  if (total_kb <= 0) {
    printf(2, "alloc: N must be > 0\n");
    exit();
  }

  int remaining = total_kb;
  int file_idx = 0;
  int files_created = 0;

  while (remaining > 0) {
    int this_kb = remaining > KB_PER_FILE ? KB_PER_FILE : remaining;

    char name[NAME_MAX];
    mkname(file_idx, name);

    // start fresh for this slot
    unlink(name);
    int fd = open(name, O_CREATE | O_RDWR);
    if (fd < 0) {
      printf(2, "alloc: cannot create %s\n", name);
      break;
    }

    if (write_kb(fd, this_kb) < 0) {
      printf(2, "alloc: write failed on %s\n", name);
      close(fd);
      // If we had a partial file, keep it (it still reserved something).
      // You could also unlink(name) here if you want all-or-nothing semantics.
      break;
    }

    close(fd);
    files_created++;
    remaining -= this_kb;
    file_idx++;
  }

  int allocated_kb = total_kb - remaining;
  if (allocated_kb == total_kb) {
    printf(1, "alloc: success, reserved %d KB across %d file(s)\n", total_kb, files_created);
  } else if (allocated_kb > 0) {
    printf(1, "alloc: partial, reserved %d KB (requested %d KB)\n", allocated_kb, total_kb);
  } else {
    printf(2, "alloc: failed, reserved 0 KB\n");
  }

  exit();
}

