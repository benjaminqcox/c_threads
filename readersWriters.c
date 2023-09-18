#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <unistd.h>

// Set the number of read and write threads
#define NUM_READ_THREADS 5
#define NUM_WRITE_THREADS 5

// Set the file name the semaphore will use
#define SEM_WRITER_FNAME "/writer"
// Define the file that will be read and written to
#define FPATH "textdoc.txt"
// Define the file mode
#define READWRITE "w+"

// Declare the writer semaphore and mutex lock
sem_t writerSem;
pthread_mutex_t lock;

// Declare shared file pointer
FILE *fptr;

// Initialise the number of readers currently reading to 0
int readerCount = 0;


void *fileWriter(void *arg)
{ 
    // Loop indefinitely
    while (1)
    {
        // Wait for write semaphore to be available
        sem_wait(&writerSem);
        // Only one writer allowed at a time so a lock has been used
        pthread_mutex_lock(&lock);
        // Add some delay to help with reading the output
        usleep(1000);
        // Write some text to the open file
        fprintf(fptr, "Some text... \n");
        // Ensure the data was written immediately
        fflush(fptr);
        // Release the mutex lock
        pthread_mutex_unlock(&lock);
        // Release the writer
        sem_post(&writerSem);
    }
    
    return NULL;
}

void *fileReader(void *arg)
{
    // Loop indefinitely
    while(1)
    {
        // Activate the mutex lock
        pthread_mutex_lock(&lock);
        // Increment the number of threads reading the file
        readerCount++;
        // If there are more than 0 readers, lock the writer
        if (readerCount > 0)
        {
            printf("reader count: %d", readerCount);
            // Lock the writer so that no writer can write while there is a reader
            sem_wait(&writerSem);
        }
        // Deactivate the mutex lock so other readers can have access
        pthread_mutex_unlock(&lock);
        // Add some delay for output reading
        usleep(10000);
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
        // Lock the reader count resources
        pthread_mutex_lock(&lock);
        // Done reading on this thread so decrement the readerCount
        readerCount--;
        // If there are no more readers, allow the writer to write
        if (readerCount == 0)
        {
            // Signal the writer it can start writing
            sem_post(&writerSem);
        }
        // Release the mutex lock
        pthread_mutex_unlock(&lock);
    } 
    return NULL;
}


int main()
{
    // Open the file in READWRITE mode and verify it openned correctly
    if ((fptr = fopen(FPATH, READWRITE)) == NULL)
    {
        fprintf(stderr, "Failed to open file\n");
        exit(EXIT_FAILURE);
    }

    // Initialise the mutex lock
    pthread_mutex_init(&lock, NULL);

    // Remove any existing semaphores at the chosen file location
    sem_unlink(SEM_WRITER_FNAME);

    // Create a writer semaphore at the given file location
    if (sem_open(SEM_WRITER_FNAME, O_CREAT, 0660, 5) == SEM_FAILED) {
        // If sem_open failed, report this and exit
        perror("sem_open/writerSem");
        exit(EXIT_FAILURE);
    }

    // Declare threads for reading and writing
    pthread_t readerThreads[NUM_READ_THREADS];
    pthread_t writerThreads[NUM_WRITE_THREADS];

    // Create all reader threads
    for (int i = 0; i < NUM_READ_THREADS; i++)
    {
        // Create reader thread and give it the fileReader function
        if (pthread_create(&readerThreads[i], NULL, &fileReader, NULL) != 0)
        {
            printf("Failed to create reader thread.\n");
            exit(EXIT_FAILURE);
        }
    }

    // Create all writer threads
    for (int i = 0; i < NUM_WRITE_THREADS; i++)
    {
        // Create writer thread and give it the fileWriter function
        if (pthread_create(&writerThreads[i], NULL, &fileWriter, NULL) != 0)
        {
            printf("Failed to create writer thread.\n");
            exit(EXIT_FAILURE);
        }
    }
    
    // Join all reader threads
    for (int i = 0; i < NUM_READ_THREADS; i++)
    {
        // Wait for reader thread to finish
        if (pthread_join(readerThreads[i], NULL) != 0)
        {
            printf("Failed to join reader thread.\n");
            exit(EXIT_FAILURE);
        }
    }
    
    // Join all writer threads
    for (int i = 0; i < NUM_WRITE_THREADS; i++)
    {
        // Wait for writer thread to finish
        if (pthread_join(writerThreads[i], NULL) != 0)
        {
            printf("Failed to join writer thread.\n");
            exit(EXIT_FAILURE);
        }
    }
    
    // Close file
    fclose(fptr);
    return 0;
}