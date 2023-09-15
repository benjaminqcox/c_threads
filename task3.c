#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#define NTHREADS 12
#define NUMITERATIONS 100000

pthread_t threads[NTHREADS];
pthread_mutex_t lock;
int count = 0;

void *printThread(void * arg)
{
    int id = *(int *)arg;
    for (int j = 0; j < NUMITERATIONS; j++)
    {
        //sleep(id);
        pthread_mutex_lock(&lock);
        printf("I'm thread %d and my value is %d\n", id, count);
        count++;
        pthread_mutex_unlock(&lock);
    }
    return NULL;
}

int main()
{
    pthread_mutex_init(&lock, NULL);
    int sleepSeconds = 2, threadCode, joinCode;
    
    int *taskIds[NTHREADS];


    for (int i = 0; i < NTHREADS; i++)
    {
        
        taskIds[i] = (int *) malloc(sizeof(int));
        if (taskIds[i] == NULL) {
            printf("Memory allocation failed.\n");
            exit(EXIT_FAILURE);
        }
        *taskIds[i] = i;
        printf("Creating thread %d\n", i);
        threadCode = pthread_create(&(threads[i]), NULL, &printThread, (void *)taskIds[i]);
        // if (i == 1)
        // {
        //     void *retVal;
        //     joinCode = pthread_join(threads[i], retVal);
        //     if (joinCode == 0)
        //     {
        //         printf("Success\n");
        //     }
        // }
        if (threadCode != 0)
        {
            printf("ERROR: return code from pthread_create() is %d\n", threadCode);
            exit(-1);
        }
    }
    for(int j = 0; j < NTHREADS; j++)
    {
        int joinRet = pthread_join(threads[j], NULL);
        if (joinRet != 0)
        {
            printf("ERROR: return code from pthread_join() is %d\n", joinRet);
            exit(-1);
        }
    }
    printf("Final count value: %d\n", count);
    pthread_mutex_destroy(&lock);
    return 0;
}