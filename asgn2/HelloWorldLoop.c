#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/resource.h>
int main()
{
  int which = PRIO_PROCESS;
  // time_t Overallstart = time(NULL);
  pid_t pid1 = fork();
  setpriority(which, pid1, 1);
  pid_t pid2 = fork();
  setpriority(which, pid2, 3);
  pid_t pid3 = fork();
  setpriority(which, pid3, 7);

  // int which = PRIO_PROCESS;
  // printf("Start: Current scheduling priority: \n");
  // printf("pid 1: %d\n", getpriority(which, pid1));
  // printf("pid 2: %d\n", getpriority(which, pid2));
  // printf("pid 3: %d\n", getpriority(which, pid3));

  // setpriority(which, pid3, 7);
  // setpriority(which, pid2, 3);
  // setpriority(which, pid1, -1);

  printf("After setting priority: Current scheduling priority: \n");
  printf("pid 1: %d\n", getpriority(which, pid1));
  printf("pid 2: %d\n", getpriority(which, pid2));
  printf("pid 3: %d\n", getpriority(which, pid3));
  time_t Overallstart = time(NULL);
  if (pid1==0) {
    int ctr = 0;
    time_t end;
    time_t start = time(NULL);
    end =  start + 4;
    while (start < end) {
      start = time(NULL);
    }
    time_t endChild = time(NULL);
    int totalChildTime = endChild-Overallstart;
    printf("Hello World 1: %d\nID: %d, %d\n", totalChildTime, pid2, pid3);
  }
  if (pid2==0) {
    int ctr = 0;
    time_t end;
    time_t start = time(NULL);
    end =  start + 4;
    while(start < end){
      start = time(NULL);
    }
    time_t endChild = time(NULL);
    int totalChildTime = endChild-Overallstart;
    printf("Hello World 2: %d\nID: %d, %d\n", totalChildTime, pid1, pid3);
  }
  if (pid3==0) {
    int ctr = 0;
    time_t end;
    time_t start = time(NULL);
    end =  start + 4;
    while(start < end){
      start = time(NULL);
    }
    time_t endChild = time(NULL);
    int totalChildTime = endChild-Overallstart;
    printf("Hello World 3: %d\nID: %d, %d\n", totalChildTime, pid1, pid2);
  }

  if (pid1 > 0 && pid2 > 0 && pid3 > 0) {
    int ctr = 0;
    time_t end;
    time_t start = time(NULL);
    end =  start + 4;
    while(start < end){
      start = time(NULL);
    }
    printf("Hello World Parent\n");
    time_t Overallend = time(NULL);
    int totalTime = Overallend-Overallstart;
    printf("Total time: %d\n", totalTime);
    return 0;
  }
  return 0;
}
