//
// Computes the minimum of a list using multiple threads
//
// Warning: Return values of calls are not checked for error to keep 
// the code simple.
//
// Compilation command on ADA ($ sign is the shell prompt):
//  $ module load intel/2017A
//  $ icc -o list_minimum.exe list_minimum.c -lpthread -lc -lrt
//
// Sample execution and output ($ sign is the shell prompt):
//  $ ./list_minimum.exe 1000000 9
// Threads = 9, minimum = 148, time (sec) =   0.0013
//  
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define MAX_THREADS     65536
#define MAX_LIST_SIZE   268435456


int num_threads;		// Number of threads to create - user input 

int thread_id[MAX_THREADS];	// User defined id for thread
pthread_t p_threads[MAX_THREADS];// Threads
pthread_attr_t attr;		// Thread attributes 

pthread_mutex_t lock_minimum;	// Protects minimum, count
double mean = 0.0;
double std_dev = 0.0;
int count;			// Count of threads that have updated minimum


int list[MAX_LIST_SIZE];	// List of values
int list_size;			// List size

// Thread routine to compute minimum of sublist assigned to thread; 
// update global value of minimum if necessary
void *find_minimum (void *s) {
    int j;
    int my_thread_id = *((int *)s);

    int block_size = list_size/num_threads;
    int my_start = my_thread_id*block_size;
    int my_end = (my_thread_id+1)*block_size-1;
    if (my_thread_id == num_threads-1) my_end = list_size-1;

    // Thread computes minimum of list[my_start ... my_end]
    int num_sum = 0;
    double sum = list[my_start]; 
    for (j = my_start+1; j <= my_end; j++) {
        if(list[j] > -1)
        {
            num_sum++;
            sum += list[j]; 
        }
    }

    // Thread updates minimum 
    // *
    // *
    // Put your code here ...
    // *
    // *
    pthread_mutex_lock(&lock_minimum);
    count++;
    if(count == num_threads){
        mean += sum;
	//printf("thread_sum = %f, thread = %d \n", mean,count);
        mean = mean/list_size;
	double std_dev_sum = 0.0;
	for(int x = 0; x < list_size; x++)
	{
	    std_dev_sum += ((list[x] - mean) * (list[x] - mean));
	}
	std_dev = std_dev_sum/list_size;
	std_dev = sqrt(std_dev);

    }
    else
    {
        mean += sum; 
    }
    pthread_mutex_unlock(&lock_minimum);
    // Thread exits
    pthread_exit(NULL);
}

// Main program - set up list of randon integers and use threads to find
// the minimum value; assign minimum value to global variable called minimum
int main(int argc, char *argv[]) {

    struct timespec start, stop;
    double total_time, time_res;
    int i, j; 
    double true_mean = 0.0;

    if (argc != 3) {
	printf("Need two integers as input \n"); 
	printf("Use: <executable_name> <list_size> <num_threads>\n"); 
	exit(0);
    }
    if ((list_size = atoi(argv[argc-2])) > MAX_LIST_SIZE) {
	printf("Maximum list size allowed: %d.\n", MAX_LIST_SIZE);
	exit(0);
    }; 
    if ((num_threads = atoi(argv[argc-1])) > MAX_THREADS) {
	printf("Maximum number of threads allowed: %d.\n", MAX_THREADS);
	exit(0);
    }; 
    if (num_threads > list_size) {
	printf("Number of threads (%d) < list_size (%d) not allowed.\n", num_threads, list_size);
	exit(0);
    }; 

    // Initialize mutex and attribute structures
    pthread_mutex_init(&lock_minimum, NULL); 
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    // Initialize list, compute minimum to verify result
    int mean_count = 0;
    srand48(0); 	// seed the random number generator 
    // true_minimum = list[0];
    for (j = 0; j < list_size; j++) {
	list[j] = lrand48();
        if(list[j] > -1)
        {
            true_mean += list[j];
            mean_count++;
        } 
    }
    //printf("sum = %f \n", true_mean);
    true_mean = true_mean/mean_count;

    // Initialize count
    count = 0;

    // Create threads; each thread executes find_minimum
    clock_gettime(CLOCK_REALTIME, &start);
    for (i = 0; i < num_threads; i++) {
	thread_id[i] = i; 
	pthread_create(&p_threads[i], &attr, find_minimum, (void *) &thread_id[i]); 
    }
    // Join threads
    for (i = 0; i < num_threads; i++) {
	pthread_join(p_threads[i], NULL);
    }

    // Compute time taken
    clock_gettime(CLOCK_REALTIME, &stop);
    total_time = (stop.tv_sec-start.tv_sec)
	+0.000000001*(stop.tv_nsec-start.tv_nsec);

    // Check answer
    if (true_mean != mean) {
	printf("Houston, we have a problem!, mean = %f \n",true_mean); 
    }
    // Print time taken
    printf("Threads = %d, mean = %f, standard deviation = %f, time (sec) = %8.4f\n", 
	    num_threads, mean, std_dev, total_time);

    // Destroy mutex and attribute structures
    pthread_attr_destroy(&attr);
    pthread_mutex_destroy(&lock_minimum);
}

