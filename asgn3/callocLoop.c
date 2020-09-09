#include <stdio.h>
#include <stdlib.h>
int main()
{
  int ctr = 0;
   while(ctr < 1000000){
     (int*)calloc(100, sizeof(int));
     printf("%d\n", ctr);
     ctr ++;
   }
}
