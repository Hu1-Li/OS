#include <stdio.h>
#include <stdlib.h>
int main(void)
{
        pid_t p1, p2, p3;
        if((p1 = fork()) == 0) {  
                execv("./2/get", NULL);
        } else {
                if((p2 = fork()) == 0) {
                        execv("./2/copy", NULL);
                } else {
                        if((p3 = fork()) == 0) {
                                 execv("./2/put", NULL);
                        }
                }
        }                
        p1 = wait(&p1);
        p2 = wait(&p2);
        p3 = wait(&p3);
        return;
}
