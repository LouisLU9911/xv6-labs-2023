#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void
primePipe(int leftFd) {
  int prime = 0;
  if (read(leftFd, &prime, sizeof(prime)) == 0) {
    close(leftFd);
    return;
  }
  fprintf(1, "prime %d\n", prime);

  int p[2];
  pipe(p);
  if (fork() == 0) {
    close(p[1]);
    primePipe(p[0]);
    exit(0);
  }
  close(p[0]);
  int n;
  while (read(leftFd, &n, sizeof(n)) != 0) {
    if (n % prime != 0) {
      write(p[1], &n, sizeof(n));
    }
  }
  close(leftFd);
  close(p[1]);
  wait(0);
}

int
main(int argc, char *argv[])
{
  int p[2];
  pipe(p);
  if (fork() == 0) {
    close(p[1]);
    primePipe(p[0]);
    exit(0);
  }
  close(p[0]);
  for (int i = 2; i <= 35; i++) {
      write(p[1], &i, sizeof(i));
  }
  close(p[1]);
  wait(0);
  exit(0);
}
