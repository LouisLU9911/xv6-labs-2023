#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/param.h"
#include "user/user.h"

int
readline(char *line, int max)
{
  // [0, max-2]
  int p = 0;
  char c;
  while (read(0, &c, 1) == 1) {
    if (p > max - 2) {
      fprintf(2, "grep: command too long\n");
      return -1;
    }
    if (c == '\n') break;
    line[p++] = c;
  }
  line[p] = '\0';
  // fprintf(1, "%s\n", line);
  return p;
}

int
main(int argc, char *argv[])
{
  int i;
  char *newArgv[MAXARG];
  for(i = 1; i < argc; i++){
    newArgv[i-1] = argv[i];
  }

  char line[512];
  while(readline(line, 512) > 0) {
    newArgv[argc-1] = line;
    newArgv[argc] = 0;
    // for(i=0; i<=argc; i++) fprintf(1, "newArgv[%d]: %s\n", i, newArgv[i]);
    if(fork() == 0){
      exec(newArgv[0], newArgv);
    }
    wait(0);
  }
  exit(0);
}
