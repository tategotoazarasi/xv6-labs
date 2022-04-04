#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  int arr[34];
  for(int i=0;i<34;i++){
    arr[i] = i+2;
  }
  int count = 34;
  while(1){
    int p[2];
    pipe(p);
    for(int i=0;i<count;i++){
      char msg[1];
      msg[0] = arr[i];
      write(p[1],msg,1);
    }
    char msg[1];
    msg[0] = 0;
    write(p[1],msg,1);
    if(fork()==0){
      int factor = 0;
      char msg[0];
      int num;
      count = 0;
      while(read(p[0],msg,1)){
        num = msg[0];
        if(num==0){
          if(factor==0){
            close(p[0]);
            close(p[1]);
            exit(0);
          }
          break;
        }
        if(factor==0){
          factor = num;
          printf("prime %d\n",num);
        }
        if(num%factor!=0){
          arr[count++] = num;
        }
      }
      close(p[0]);
      close(p[1]);
    }else{
      close(p[0]);
      close(p[1]);
      int o;
      wait(&o);
      exit(0);
    }
  }
  exit(0);
}