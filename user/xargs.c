#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/param.h"

char *substr(char *str, int start, int len) {
  char *ans = (char *)malloc(len + 1);
  int i = 0;
  for (; i < len; i++) {
    ans[i] = str[start + i];
  }
  ans[i] = '\0';
  return ans;
}

void handle_line(char *buff, int l, int r, int argc, char *argv[], int arg_start, char* cmd) {
  //printf("l = %d,\tr = %d,\targ_start = %d,\tcmd = %s\n", l, r, arg_start, cmd);
  int argcount = 1;
  char **new_argv = (char **)malloc(sizeof(char *) * MAXARG);
  new_argv[0] = cmd;
  for(int i=arg_start;i<argc;++i){
    new_argv[argcount++] = argv[i];
  }
  int p1 = l;
  int p2 = l;
  while (p1 < r && p2 < r) {
    while (p1 < r && buff[p1] == ' ') {
      p1++;
    }
    p2 = p1;
    while (p2 < r && buff[p2] != ' ') {
      p2++;
    }
    char *word = substr(buff,p1,p2-p1);
    p2++;
    p1 = p2;
    //printf("word = %s\n",word);
    new_argv[argcount++] = word;
  }
  char **new_argv2 = (char **)malloc(sizeof(char *) * argcount);
  for(int i=0;i<argcount;i++){
    new_argv2[i] = new_argv[i];
    //printf("%s ",new_argv2[i]);
  }
  printf("\n");
  if(fork()==0){
    exec(cmd,new_argv2);
    fprintf(2,"exec error\n");
    exit(1);
  }else{
    int _;
    wait(&_);
  }
}

int main(int argc, char *argv[]) {
  int arg_start = 2;
  if (argc < 2) {
    fprintf(2, "argc not enough");
    exit(1);
  }
  if (strcmp(argv[1], "-n") == 0) {
    if (argc < 4) {
      fprintf(2, "argc not enough");
      exit(1);
    } else {
      arg_start = 4;
    }
  }
  char buff[2048];
  int m = 0;
  int n;
  while ((n = read(0, buff + m, sizeof(buff) - m)) != 0) {
    m += n;
  }
  int p1 = 0;
  int p2 = 0;
  while (p2 < m) {
    while (p2 < m && buff[p2] != '\n') {
      p2++;
    }
    //printf("p1 = %d,\tp2 = %d\n",p1,p2);
    handle_line(buff, p1, p2, argc, argv, arg_start, argv[arg_start-1]);
    p2++;
    p1=p2;
  }
  exit(0);
}