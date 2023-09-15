#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

void task2()
{
    pid_t pid = fork();
    int status;
    if (pid == 0)
    {
        printf("I'm the child %d\n", getpid());
        //abort();
        exit(1);
    }
    else
    {
        pid_t terminatedChild = wait(&status);
        printf("Child %d terminated with status code %d\n", terminatedChild, WEXITSTATUS(status));
        printf("I'm the parent: %d\n", getppid());
    }
    
   

}