#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"

char *concat(char *a,char *b){
  char *ret = malloc((strlen(a)+strlen(b)+2)*sizeof(char));
  int j = 0;
  for(int i=0;a[i]!='\0';++i,++j){
    ret[j]=a[i];
  }
  ret[j++]='/';
  for(int i=0;b[i]!='\0';++i,++j){
    ret[j]=b[i];
  }
  ret[j]='\0';
  return ret;
}

void recurse(char *path, char *filename, char *target){
  struct stat st;
  int fd;
  if((fd = open(path, O_RDONLY)) < 0){
    fprintf(2, "ls: cannot open %s.\n", path);
    return;
  }
  if(fstat(fd, &st) < 0){
    fprintf(2, "ls: cannot stat %s\n", path);
    close(fd);
    return;
  }
  struct dirent de;
  if(st.type == T_DIR){
    //printf("%s\n",filename);
    while(read(fd, &de, sizeof(de)) == sizeof(de)){
      if(strcmp(".",de.name)!=0 && strcmp("..",de.name)!=0 && strcmp("",de.name)!=0){
        char *name = concat(path,de.name);
        //printf("%s\n",de.name);
        recurse(name,de.name,target);
      }
    }
  }else{
    if(strcmp(target,filename)==0) {
      printf("%s\n", path);
    }
  }
  close(fd);
}

int
main(int argc, char *argv[])
{
  if(argc<3){
    printf("no enough params\n");
    exit(1);
  }
  recurse(argv[1],argv[1],argv[2]);
  exit(0);
}