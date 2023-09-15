#include <stdio.h>
#include <unistd.h>
void task1()
{
    pid_t pid = fork();
    printf("PID: %d\n", pid);
    if (pid == 0)
    {
        printf("I'm %d, child process of %d\n", getpid(), getppid());
    }
    else
    {
        printf("I'm the parent. My child is: %d\n", pid);
        
    }
}