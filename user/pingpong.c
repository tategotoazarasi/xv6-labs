#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  int p[2];
  pipe(p);
  if(fork()==0){
    char s[5];
    read(p[0], &s, sizeof(s));
    printf("%d: received ping\n",getpid());
    write(p[0],"pong",5);
    close(p[0]);
    close(p[1]);
  }else{
    char s[1];
    write(p[1],"ping",5);
    int w;
    wait(&w);
    read(p[1], &s, sizeof(s));
    printf("%d: received pong\n",getpid());
    close(p[0]);
    close(p[1]);
  }
  exit(0);
}