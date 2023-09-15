#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <unistd.h>

#define SEM_READER_FNAME "/reader"
#define SEM_WRITER_FNAME "/writer"
#define FPATH "textdoc.txt"
#define READWRITE "a"
#define READONLY "r"

sem_t readerSem, writerSem;

pthread_mutex_t lock;

// Shared file
FILE *fptr;


void *fileWriter(void *arg)
{ 
    // Loop indefinitely
    while (1)
    {
        // Wait for resources
        sem_wait(&writerSem);
        // Activate the mutex lock
        pthread_mutex_lock(&lock);
        // Open the file in READWRITE mode and verify it openned correctly
        if ((fptr = fopen(FPATH, READWRITE)) == NULL)
        {
            fprintf(stderr, "Failed to open file\n");
            exit(EXIT_FAILURE);
        }
        // Sleep for 10_000 microseconds
        usleep(10000);
        // Write some text to the open file
        fprintf(fptr, "Some text... \n");
        // Ensure the data was written immediately
        fflush(fptr);
        // Close the file
        fclose(fptr);
        // Release the mutex lock
        pthread_mutex_unlock(&lock);
        // Signal the reader to start reading
        sem_post(&readerSem);
    }
    
    return NULL;
}

void *fileReader(void *arg)
{
    // Loop indefinitely
    while(1)
    {
        // Wait for resources
        sem_wait(&readerSem);
        // Activate the mutex lock
        pthread_mutex_lock(&lock);
        // Open the file in READONLY mode and verify it openned correctly
        if ((fptr = fopen(FPATH, READONLY)) == NULL)
        {
            fprintf(stderr, "Failed to open file\n");
            exit(EXIT_FAILURE);
        }
       
        // Sleep for 1_000 microseconds
        usleep(1000);
        // Declare a buffer to store string (file contents)
        char buffer[BUFSIZ];
        // Get a pointer to the initial character in the file
        fseek(fptr, 0, SEEK_SET);
        // Loop through file while there are still characters to be read
        while (fgets(buffer, sizeof(buffer), fptr) != NULL)
        {
            // Print the file contents
            printf("%s", buffer);
        }
        // Close the file
        fclose (fptr);
        // Release the mutex lock
        pthread_mutex_unlock(&lock);
        // Signal the writer to start writing
        sem_post(&writerSem);
    }
    // Close the file
    
    return NULL;
}


int main()
{
    // Initialise the mutex lock
    pthread_mutex_init(&lock, NULL);

    // Remove any existing semaphores at the chosen file location
    sem_unlink(SEM_READER_FNAME);
    sem_unlink(SEM_WRITER_FNAME);

    // Create a writer semaphore at the given file location
    if (sem_open(SEM_READER_FNAME, O_CREAT, 0660, 1) == SEM_FAILED) {
        // If sem_open failed, report this and exit
        perror("sem_open/readerSem");
        exit(EXIT_FAILURE);
    }

    // Create a writer semaphore at the given file location
    if (sem_open(SEM_WRITER_FNAME, O_CREAT, 0660, 0) == SEM_FAILED) {
        // If sem_open failed, report this and exit
        perror("sem_open/writerSem");
        exit(EXIT_FAILURE);
    }

    pthread_t readerThread, writerThread;
    // Create reader thread and give it the fileReader function
    if (pthread_create(&readerThread, NULL, &fileReader, NULL) != 0)
    {
        printf("Failed to create reader thread.\n");
        exit(EXIT_FAILURE);
    }

    // Create writer thread and give it the fileWriter function
    if (pthread_create(&writerThread, NULL, &fileWriter, NULL) != 0)
    {
        printf("Failed to create writer thread.\n");
        exit(EXIT_FAILURE);
    }
    
    // Wait for reader thread to finish
    if (pthread_join(readerThread, NULL) != 0)
    {
        printf("Failed to join reader thread.\n");
        exit(EXIT_FAILURE);
    }

    // Wait for writer thread to finish
    if (pthread_join(writerThread, NULL) != 0)
    {
        printf("Failed to join writer thread.\n");
        exit(EXIT_FAILURE);
    }
    return 0;
}