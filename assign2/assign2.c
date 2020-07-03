/*
 * assign2.c
 *
 * Name:
 * Student Number:
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h> 
#include <pthread.h>
#include <semaphore.h>
#include "train.h"

/*
 * If you uncomment the following line, some debugging
 * output will be produced.
 *
 * Be sure to comment this line out again before you submit 
 */

#define DEBUG	1 

void ArriveBridge (TrainInfo *train);
void CrossBridge (TrainInfo *train);
void LeaveBridge (TrainInfo *train);

int *eastQueue;
int *westQueue;
volatile int eastConsec = 0;
volatile int currentCrossing = -1; 
volatile int currentEast = 0;
volatile int currentWest = 0;
volatile int eastCount = 0;
volatile int westCount = 0;
volatile int eastIndex = 0;
volatile int westIndex = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;


/*
 * This function is started for each thread created by the
 * main thread.  Each thread is given a TrainInfo structure
 * that specifies information about the train the individual 
 * thread is supposed to simulate.
 */
void * Train ( void *arguments )
{
	TrainInfo	*train = (TrainInfo *)arguments;

	/* Sleep to simulate different arrival times */
	usleep (train->length*SLEEP_MULTIPLE);

	ArriveBridge (train);
	CrossBridge  (train);
	LeaveBridge  (train); 

	/* I decided that the paramter structure would be malloc'd 
	 * in the main thread, but the individual threads are responsible
	 * for freeing the memory.
	 *
	 * This way I didn't have to keep an array of parameter pointers
	 * in the main thread.
	 */
	free (train);
	return NULL;
}

/*
 * You will need to add code to this function to ensure that
 * the trains cross the bridge in the correct order.
 */
void ArriveBridge ( TrainInfo *train )
{	
	#ifdef DEBUG
	printf ("Train %2d arrives going %s\n", train->trainId, 
		(train->direction == DIRECTION_WEST ? "West" : "East"));
	#endif

	if (train->direction == DIRECTION_WEST){
		westQueue[westIndex] = train->trainId;
		if (westCount == 0){
			currentWest = 0;
		}
		westIndex++;
		westCount++;
	}
	else {
		eastQueue[eastIndex] = train->trainId;
		if (eastCount == 0){
			currentEast = 0;
		}
		eastIndex++;
		eastCount++;
	}

	//tell the first train that arrives to go 
	if(currentCrossing == -1){
		currentCrossing = train->trainId;
		if (train->direction == DIRECTION_WEST){
			currentWest++;
		} else {
			currentEast++;
		}
	}

	pthread_mutex_lock(&mutex);
	while(currentCrossing != train->trainId) {
		pthread_cond_wait(&cond, &mutex);
	} 
	pthread_mutex_unlock(&mutex);
}

/*
 * Simulate crossing the bridge.  You shouldn't have to change this
 * function.
 */
void CrossBridge ( TrainInfo *train )
{	
	#ifdef DEBUG
	printf ("Train %2d is ON the bridge (%s)\n", train->trainId,
			(train->direction == DIRECTION_WEST ? "West" : "East"));
	fflush(stdout);
	#endif
	/* 
	 * This sleep statement simulates the time it takes to 
	 * cross the bridge.  Longer trains take more time.
	 */
	usleep (train->length*SLEEP_MULTIPLE);

	printf ("Train %2d is OFF the bridge(%s)\n", train->trainId, 
			(train->direction == DIRECTION_WEST ? "West" : "East"));
	fflush(stdout);
}

/*
 * Add code here to make the bridge available to waiting
 * trains...
 */
void LeaveBridge ( TrainInfo *train )
{
	//printf("West: %d, East: %d, Consec; %d\n",westCount,eastCount,eastConsec);
	pthread_mutex_lock(&mutex);
	if (train->direction == DIRECTION_WEST){
		westCount--;
		eastConsec = 0;
		if(eastCount > 0){
			currentCrossing = eastQueue[currentEast]; 
			currentEast++;
		} else if(westCount > 0){
			currentCrossing = westQueue[currentWest];
			currentWest++;
		} 
	} else {
		eastCount--;
		eastConsec++;
		if (westCount == 0){
			currentCrossing = eastQueue[currentEast]; 
			currentEast++;
		} else {
			if (eastConsec < 2){
				currentCrossing = eastQueue[currentEast]; 
				currentEast++;
			} else {
				currentCrossing = westQueue[currentWest];
				currentWest++;
			}
		}
	}
	pthread_mutex_unlock(&mutex);
	pthread_cond_broadcast(&cond);
}

int main ( int argc, char *argv[] )
{
	int		trainCount = 0;
	char 		*filename = NULL;
	pthread_t	*tids;
	int		i;

		
	/* Parse the arguments */
	if ( argc < 2 )
	{
		printf ("Usage: part1 n {filename}\n\t\tn is number of trains\n");
		printf ("\t\tfilename is input file to use (optional)\n");
		exit(0);
	}
	
	if ( argc >= 2 )
	{
		trainCount = atoi(argv[1]);
	}
	if ( argc == 3 )
	{
		filename = argv[2];
	}	
	
	initTrain(filename);
	
	/*
	 * Since the number of trains to simulate is specified on the command
	 * line, we need to malloc space to store the thread ids of each train
	 * thread.
	 */
	tids = (pthread_t *) malloc(sizeof(pthread_t)*trainCount);
	
	/*
	 * Create all the train threads pass them the information about
	 * length and direction as a TrainInfo structure
	 */
	int tempEastQueue[trainCount];
	int tempWestQueue[trainCount];
	eastQueue = tempEastQueue;
	westQueue = tempWestQueue;
	
	for (i=0;i<trainCount;i++)
	{
		TrainInfo *info = createTrain();

		#ifdef DEBUG
		printf ("Train %2d headed %s length is %d\n", info->trainId,
			(info->direction == DIRECTION_WEST ? "West" : "East"),
			info->length );
		#endif 

		if ( pthread_create (&tids[i],0, Train, (void *)info) != 0 )
		{
			printf ("Failed creation of Train.\n");
			exit(0);
		}
	}

	/*
	 * This code waits for all train threads to terminate
	 */
	for (i=0;i<trainCount;i++)
	{
		pthread_join (tids[i], NULL);
	}
	
	free(tids);
	return 0;
}

