#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void sieve(int from) {
  // MUST close from and wait from child before return
  // exit after return
  int prime;
  if (read(from, &prime, sizeof(prime)) == 0) {
    close(from);
    return;
  }

  fprintf(1, "prime %d\n", prime);
  int p[2];
  pipe(p);
  int r = p[0], w = p[1];

  if (fork() == 0) {
    close(w);
    sieve(r);
    exit(0);
  }

  close(r);
  int next;
  while (read(from, &next, sizeof(next)) != 0) {
    // read from the previous pipe
    if (next % prime != 0) {
      // write to the next pipe
      write(w, &next, sizeof(next));
    }
  }
  close(from);
  close(w);
  wait(0);
}

int main(int argc, char *argv[]) {
  // [read, write]
  int p[2];
  pipe(p);
  int r = p[0], w = p[1];

  if (fork() == 0) {
    close(w);
    sieve(r);
    exit(0);
  }
  close(r);
  for (int i = 2; i <= 35; i++) {
    write(w, &i, sizeof(i));
  }
  close(w);
  wait(0);
  exit(0);
}
