#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <fcntl.h>

extern char ** get_args();
void executeCommand(char ** args){
  pid_t pid;
  pid_t wpid;
  int status;
  pid = fork();
  if(pid == 0){
    execvp(*args, args);
    exit(1);
  }else if (pid < 0){
    perror("Failed to fork process\n");
    exit(1);
  }else{
    wpid = wait(&status);
    if (WEXITSTATUS(status) == 1){
      printf("Invalid command\n");
    }
    if (WIFEXITED(status) || WIFSIGNALED(status)){
    }
  }
}
void executeCommandChain(char ** currentCmd){
  pid_t pid;
  pid_t wpid;
  int status;
  pid = fork();
  if(pid == 0){
    execvp(*currentCmd, currentCmd);
    exit(1);
  }else if (pid < 0){
    perror("Failed to fork process\n");
    exit(1);
  }else{
    wpid = wait(&status);
    if (WEXITSTATUS(status) == 1){
      printf("Invalid command\n");
    }
    if (WIFEXITED(status) || WIFSIGNALED(status)){
    }
  }
}
void inputRedirect(char ** args, int i){
  pid_t pid;
  pid_t wpid;
  int status;
  int fd;
  pid = fork();
  if(pid == 0){
    fd = open(args[i+1], O_RDONLY);
    if (fd == -1){
      perror("Failed to open");
      exit(1);
    }
    dup2(fd, STDIN_FILENO);
    args[i] = NULL;
    args[i+1]=NULL;
    execvp(*args, args);
    close(fd);
    exit(1);
  }else if (pid < 0){
    perror("Failed to fork process\n");
    exit(1);
  }else{
    wpid = wait(&status);
    if (WIFEXITED(status) || WIFSIGNALED(status)){
    }
  }
}
void outputRedirect(char ** args, int i){
  pid_t pid;
  pid_t wpid;
  int status;
  int fd;
  fd = open(args[i+1], O_WRONLY|O_CREAT, 0666);
  if (fd == -1){
    perror("Failed to open");
    exit(1);
  }
  pid = fork();
  if(pid == 0){
    args[i] = NULL;
    args[i+1]=NULL;
    close(1);
    dup(fd);
    execvp(*args, args);
    close(fd);
    exit(1);
  }else if (pid < 0){
    perror("Failed to fork process\n");
    exit(1);
  }else{
    wpid = wait(&status);
    if (WIFEXITED(status) || WIFSIGNALED(status)){
    }
  }
}
void outputRedirectApp(char ** args, int i){
  pid_t pid;
  pid_t wpid;
  int status;
  int fd;
  fd = open(args[i+1], O_WRONLY|O_APPEND, 0666);
  if (fd == -1){
    outputRedirect(args, i);
  }
  pid = fork();
  if(pid == 0){
    args[i] = NULL;
    args[i+1]=NULL;
    close(1);
    dup(fd);
    execvp(*args, args);
    close(fd);
    exit(1);
  }else if (pid < 0){
    perror("Failed to fork process\n");
    exit(1);
  }else{
    wpid = wait(&status);
    if (WIFEXITED(status) || WIFSIGNALED(status)){
    }
  }
}
void outputPiped(char ** args, int i, char ** cmd1, char ** cmd2){
////Based on FreeBSD Manual
  int pipe1[2];
  pid_t pid1;
  pid_t pid2;

  if (pipe(pipe1) == -1) {
    perror("pipe");
    exit(1);
  }

  pid1	= fork();
  if (pid1 == -1) {
    perror("fork");
    exit(1);
  }

  if (pid1 == 0) {
    close(pipe1[1]);
    dup2(pipe1[0], STDIN_FILENO);
    close(pipe1[0]);
    execvp(*cmd2, cmd2);
    exit(0);

  }else{
    pid2 = fork();
    if (pid2 < 0) {
      perror("fork");
      exit(1);
    }
    if (pid2 == 0) {
      close(pipe1[0]);
      dup2(pipe1[1], STDOUT_FILENO);
      close(pipe1[1]);
      execvp(*cmd1, cmd1);
      exit(0);
    }else{
      wait(NULL);
      wait(NULL);
    }
  }
}

int main(){
  int         i;
  char **     args;
  char ** cmd1;
  char ** cmd2;
  char rwd[256];
  getcwd(rwd, sizeof(rwd));
  int lastSemi = -1;
  int nextSemi = 0;
  int ctr = 0;
  char ** currentCmd;

  while (1) {
    top:
  	printf ("Command ('exit' to quit): ");
  	args = get_args();
  	for (i = 0; args[i] != NULL; i++) {
  	    printf ("Argument %d: %s\n", i, args[i]);
  	}

  ///chained commands
    for (int index = 0; args[index] != NULL; index++) {
      loop:
      if (!strcmp (args[index], ";")){
        ctr ++;
        currentCmd = &args[lastSemi+1];
        int ctr2=0;
        while(currentCmd[ctr2] != NULL){
          if(!strcmp (currentCmd[ctr2], ";")){
            for (int index2 = ctr2; currentCmd[index2] != NULL; index2++) {
            currentCmd[ctr2] = NULL;
            }
          }
          ctr2++;
        }


        ///input redirection
          for (int j = 0; currentCmd[j] != NULL; j++) {
            if (!strcmp (currentCmd[j], "<") && currentCmd[j+1] != NULL){
              inputRedirect(currentCmd, j);
              goto loop;
            }
          }


        ///output redirection (overwrites)
          for (int k = 0; currentCmd[k] != NULL; k++) {
            if (!strcmp (currentCmd[k], ">") && currentCmd[k+1] != NULL){
              outputRedirect(currentCmd, k);
              goto loop;
            }
          }

        ///output redirection (appends)
          for (int l = 0; currentCmd[l] != NULL; l++) {
            if (!strcmp (currentCmd[l], ">>") && currentCmd[l+1] != NULL){
              outputRedirectApp(currentCmd, l);
              goto loop;
            }
          }


        executeCommandChain(currentCmd);
        lastSemi = index;
      }else if(args[index+1] == NULL && ctr >= 1){
        currentCmd = &args[lastSemi+1];


        ///input redirection
          for (int j = 0; currentCmd[j] != NULL; j++) {
            if (!strcmp (currentCmd[j], "<") && currentCmd[j+1] != NULL){
              inputRedirect(currentCmd, j);
              lastSemi = -1;
              ctr = 0;
              goto top;
            }
          }


        ///output redirection (overwrites)
          for (int k = 0; currentCmd[k] != NULL; k++) {
            if (!strcmp (currentCmd[k], ">") && currentCmd[k+1] != NULL){
              outputRedirect(currentCmd, k);
              lastSemi = -1;
              ctr = 0;
              goto top;
            }
          }

        ///output redirection (appends)
          for (int l = 0; currentCmd[l] != NULL; l++) {
            if (!strcmp (currentCmd[l], ">>") && currentCmd[l+1] != NULL){
              outputRedirectApp(currentCmd, l);
              lastSemi = -1;
              ctr = 0;
              goto top;
            }
          }


        executeCommandChain(currentCmd);
        lastSemi = -1;
        ctr = 0;
        goto top;
      }
    }



  ///input redirection
    for (int j = 0; args[j] != NULL; j++) {
      if (!strcmp (args[j], "<") && args[j+1] != NULL){
        inputRedirect(args, j);
        goto top;
      }
    }


  ///output redirection (overwrites)
    for (int k = 0; args[k] != NULL; k++) {
      if (!strcmp (args[k], ">") && args[k+1] != NULL){
        outputRedirect(args, k);
        goto top;
      }
    }

  ///output redirection (appends)
    for (int l = 0; args[l] != NULL; l++) {
      if (!strcmp (args[l], ">>") && args[l+1] != NULL){
        outputRedirectApp(args, l);
        goto top;
      }
    }

  ///piped output
    cmd1=&args[0];
    for (int j = 0; args[j] != NULL; j++) {
      if (!strcmp (args[j], "|") && args[j+1] != NULL){
        cmd1[j]=NULL;
        cmd1[j+1]=NULL;
        cmd1[j+2]=NULL;
        cmd2=&args[j+1];
        outputPiped(args, j, cmd1, cmd2);
        goto top;
      }
    }

  ///No arguments
  	if (args[0] == NULL) {
  	    printf ("No arguments on line!\n");


  /////Exit command
  	} else if ( !strcmp (args[0], "exit") && args[1] == NULL) {
  	    printf ("Exiting...\n");
  	    exit(0);

  /////// cd command
    }else if (!strcmp (args[0], "cd")){
      if (args[1] == NULL) {
        chdir(rwd);
      } else {
        if (chdir(args[1]) != 0) {
          perror("Error");
        }
      }
    }else{
      executeCommand(args);
    }
  }
}
