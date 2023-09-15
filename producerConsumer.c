#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sys/sem.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>

#define SEM_PRODUCER_FNAME "/producer"
#define SEM_CONSUMER_FNAME "/consumer"
#define NUMPRODUCERS 5
#define NUMCONSUMERS 1
#define NUMRESOURCES 15

// Shared resource between producers and consumers
uint8_t resourcesUsed = 0;

// Setup the mutex lock
pthread_mutex_t lock;

void cleanup_threads(pthread_t *threads, int numThreads)
{
    // Go through all given threads and close them
    for (int i = 0; i < numThreads; i++) {
        // Need to check the thread actually exists
        pthread_exit((void *)&(threads[i]));
    }
}

void *produceRes(void *arg)
{
    // Open the created semaphores
    sem_t *semProd = sem_open(SEM_PRODUCER_FNAME, 0);
    sem_t *semCons = sem_open(SEM_CONSUMER_FNAME, 0);
    // Loop indefinitely
    while (1==1)
    {
        // Wait to gain access to the producer semaphore
        if (sem_wait(semProd) == -1)
        {
            // If the signal fails (sem_wait == -1), report where the error occured and exit
            perror("sem_wait/add");
            exit(EXIT_FAILURE);
        }
        // This code is ran during semaphore access
        // Lock the resourcesUsed global variable so no other threads modify the value
        pthread_mutex_lock(&lock);
        // If there are fewer resources than the chosen maximum
        if (resourcesUsed < NUMRESOURCES)
        {
            sleep(2);
            // Print the thread currently modifying the resources
            printf("Thread being used %d\n", *(unsigned char *)arg);
            // Update the number of resources
            resourcesUsed++;
            // Print the number of resources available and the command ran
            printf("Resource added: %d\n", resourcesUsed);
        }
        // Unlock the resourcesUsed global variable ready for another thread to modify
        pthread_mutex_unlock(&lock);

        // Signal the consumer semaphore that the produce semaphore has added a resource
        if (sem_post(semCons) == -1)
        {
            // If the signal fails (sem_post == -1), report where the error occured and exit
            perror("sem_post/add");
            exit(EXIT_FAILURE);
        }
    }
    return NULL;
}

void *consumeRes(void *arg)
{
    // Open the created semaphores
    sem_t *semCons = sem_open(SEM_CONSUMER_FNAME, 0);
    sem_t *semProd = sem_open(SEM_PRODUCER_FNAME, 0);
    // Loop indefinitely
    while (1==1)
    {
        // Wait to gain access to the consumer semaphore
        if (sem_wait(semCons) == -1)
        {
            // If the signal fails (sem_wait == -1), report where the error occured and exit
            perror("sem_wait/remove");
            exit(EXIT_FAILURE);
        }
        // This code is ran during semaphore access
        // Lock the resourcesUsed global variable so no other threads modify the value
        pthread_mutex_lock(&lock);
        // If there are resources available
        if (resourcesUsed > 0)
        {
            sleep(1);
            // Print the thread currently modifying the resources
            printf("Thread being used %d\n", *(unsigned char *)arg);
            // Update the number of resources
            resourcesUsed--;
            // Print the number of resources available and the command ran
            printf("Resource taken: %d\n", resourcesUsed);
        }
        // Unlock the resourcesUsed global variable ready for another thread to modify
        pthread_mutex_unlock(&lock);

        // Signal that the consumer semaphore is done
        if (sem_post(semProd) == -1)
        {
            // If the signal fails (sem_post == -1), report where the error occured and exit
            perror("sem_post/remove");
            exit(EXIT_FAILURE);
        }
    }
    return NULL;
}


int main()
{
    // Initialise the mutex and ensure it was successful
    if(pthread_mutex_init(&lock, NULL) != 0)
    {
        // If an error occured (init != 0), report this and exit
        perror("mutex init");
        exit(EXIT_FAILURE);
    }
    
    // Setup array of taskIds that will store the thread id
    unsigned char *taskIds[NUMCONSUMERS];

    // Unlink possible previous semaphores with the same names
    sem_unlink(SEM_PRODUCER_FNAME);
    sem_unlink(SEM_CONSUMER_FNAME);

    // Initialise semaphores for producers
    sem_t *semProd = sem_open(SEM_PRODUCER_FNAME, O_CREAT, 0660, NUMRESOURCES);
    if (semProd == SEM_FAILED) {
        // If sem_open failed, report this and exit
        perror("sem_open/producer");
        exit(EXIT_FAILURE);
    }
    // Initialise semaphores for consumers
    sem_t *semCons = sem_open(SEM_CONSUMER_FNAME, O_CREAT, 0660, 0);
    if (semCons == SEM_FAILED) {
        // If sem_open failed, report this and exit
        perror("sem_open/consumer");
        exit(EXIT_FAILURE);
    }

    // Declare producers and consumer threads
    pthread_t producerThreads[NUMPRODUCERS];
    pthread_t consumerThreads[NUMCONSUMERS];

    // Declare a variable that stores the return code from pthread_create
    int createdSuccessful;

    // Create producer threads
    for (unsigned char i = 0; i < NUMPRODUCERS; i++)
    {
        // Allocate memory for the taskIds to be stored
        taskIds[i] = (unsigned char *) malloc(sizeof(unsigned char));
        // Ensure the memomry was allocated correctly
        if (taskIds[i] == NULL) {
            printf("Memory allocation failed.\n");
            exit(EXIT_FAILURE);
        }
        // Initialise taskId[i]'s value to the current value of i
        *taskIds[i] = i;
        // Create a thread that runs the produceRes function
        createdSuccessful = pthread_create(&(producerThreads[i]), NULL, &produceRes, (void *)taskIds[i]);
        // If not created successfully, let the user know, cleanup and exit safely
        if (createdSuccessful != 0)
        {
            printf("Failed to create thread.");
            // Exit the existing producer threads as they have all been created successfully
            cleanup_threads(producerThreads, NUMPRODUCERS);
            // Number of consumer threads created up to this point is i as the i element failed
            cleanup_threads(consumerThreads, i-1);
            exit(EXIT_FAILURE);
        }
    }

    // Create consumer threads
    for (unsigned char i = 0; i < NUMCONSUMERS; i++)
    {
        // Allocate memory for the taskIds to be stored
        taskIds[i] = (unsigned char *) malloc(sizeof(unsigned char));
        // Ensure the memomry was allocated correctly
        if (taskIds[i] == NULL) {
            printf("Memory allocation failed.\n");
            exit(EXIT_FAILURE);
        }
        // Initialise taskId[i]'s value to the current value of i
        *taskIds[i] = i;
        // Create a thread that runs the consumeRes function
        createdSuccessful = pthread_create(&(consumerThreads[i]), NULL, &consumeRes, (void *)taskIds[i]);
        // If not created successfully, let the user know, cleanup and exit safely
        if (createdSuccessful != 0)
        {
            printf("Failed to create thread.");
            // Exit the existing producer threads as they have all been created successfully
            cleanup_threads(producerThreads, NUMPRODUCERS);
            // Number of consumer threads created up to this point is i as the i element failed
            cleanup_threads(consumerThreads, i-1);
            exit(EXIT_FAILURE);
        }
    }
    // Loop through all threads and wait for them to finish
    for (unsigned char i = 0; i < NUMPRODUCERS; i++)
    {
        if (pthread_join(producerThreads[i], NULL) != 0)
        {
            perror("pthread_join/producer");
            exit(EXIT_FAILURE);
        }
    }

    for (unsigned char i = 0; i < NUMCONSUMERS; i++)
    {
        if (pthread_join(consumerThreads[i], NULL))
        {
            perror("pthread_join/consumer");
            exit(EXIT_FAILURE);
        }     
    }

    // Cleanup all threads and resources
    sem_unlink(SEM_PRODUCER_FNAME);
    sem_unlink(SEM_CONSUMER_FNAME);
    cleanup_threads(producerThreads, NUMPRODUCERS);
    cleanup_threads(consumerThreads, NUMCONSUMERS);
    pthread_exit(NULL);
    
    return 0;
}