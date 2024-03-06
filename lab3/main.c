//
//  main.c
//  Lab3Test2
//
//  Created by Thomas Dean on 2023-10-13.
//

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sched.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <signal.h>

// Parameter strucutre for threads
struct threadParm{
    // name of file to read or write
    char fileName[20];
    // thread num for debug messages
    int threadNum;
};

// Global Vars
// the current test number
int testNum = 0;

// function prototypes
void simulate_interrupt(void);

// mutexes
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t empty  = PTHREAD_COND_INITIALIZER;
pthread_cond_t full  = PTHREAD_COND_INITIALIZER;

//*********Begin Shared Variables*************
// number of running producers
int numProdRunning = 0;
// buffer
#define numSlots 3
int numElements = 0;
int head = 0;
int tail = 0;
int buffer[numSlots];
//*********End Shared Variables*************

//+
// Function: producer
//
// Purpose:  This function reads from the file and writes to the buffer.
//           The parameter is a pointer to a struct threadParam which
//           gives the name of the output file and the number of the thread.
//-

void * producer(void * parm){
    struct threadParm *prodParm = (struct threadParm *) parm;
    const unsigned int linelen= 1024;
    char line[linelen];
    int lineNo = 0;
    int value = 0;

    printf("Enter producer %d\n",prodParm->threadNum);

    FILE * inFile = fopen(prodParm->fileName,"r");
    if (inFile == NULL){
        perror(prodParm->fileName);
        printf("Exit because producer %d can't open file\n",prodParm->threadNum);
        exit(1);
    }

    while(fgets(line, linelen, inFile)){
        lineNo++;
        value = atoi(line);

        // lock
        pthread_mutex_lock(&mutex);
        printf("Producer thread %d obtaining lock for %d: %d\n", prodParm -> threadNum,lineNo, value);

        //wait if full
        if (numElements >= numSlots) pthread_cond_wait(&full, &mutex), printf("Producer thread %d waiting full\n", prodParm->threadNum);
	    //printf("Producer thread %d waiting full\n", prodParm->threadNum);

        printf("Producer thread %d adding %d: %d at position %d\n", prodParm->threadNum, lineNo, value, head);
        numElements++;
        buffer[head++] = value; //add value to buffer
        if(head >= numSlots) head = 0; 
        
        pthread_cond_signal(&empty); //signal empty
        printf("Producer thread %d signaling empty\n", prodParm->threadNum);

        pthread_mutex_unlock(&mutex); //release
        printf("Producer thread %d releasing lock\n", prodParm->threadNum);
    }

    pthread_mutex_lock(&mutex);
    // printf("Number of producers %d\n", numProdRunning);
    numProdRunning--; // decrement the number of running producers
    if (numProdRunning <= 0) pthread_cond_broadcast(&empty); // broadcast signal if we are the last one. 
    pthread_mutex_unlock(&mutex);

   
    // done. 
    fclose(inFile);
    printf("Exit producer %d\n",prodParm->threadNum);
    return NULL;
}

//+
// Function: consumer
//
// Purpose:  This function reads from the buffer and writes to a file.
//           The parameter is a pointer to a struct threadParam which
//           gives the name of the output file and the number of the thread.
//-

void * consumer(void * parm){
    struct threadParm *consParm = (struct threadParm *) parm;
    int lineNo = 0;
    int value = 0;
    int location;

    printf("Enter consumer %d\n",consParm->threadNum);

    FILE * outFile = fopen(consParm->fileName,"w");
    if (outFile == NULL){
        perror(consParm->fileName);
        printf("Exiting because consumer %d can't open file\n",consParm->threadNum);
        exit(1);
    }

    while(1){
        lineNo++;

        pthread_mutex_lock(&mutex); // lock
        printf("Consumer thread %d aquiring lock\n", consParm->threadNum);

        //wait if empty and there are producers
        if (numElements <= 0 && numProdRunning > 0) printf("Consumer thread %d waiting on empty\n", consParm->threadNum), pthread_cond_wait(&empty, &mutex); 
	    //printf("Consumer thread %d waiting on empty\n", consParm->threadNum);

        // if the buffer is empty and no producers, then 
        // release the lock and break the loop
        if (numElements <= 0 && numProdRunning <= 0) {
            pthread_cond_signal(&full); 
            pthread_mutex_unlock(&mutex);
            break;
        }

        value = buffer[tail]; // read value from to buffer
        tail = (tail + 1) % numSlots; //Maintain tail within bounds
        numElements--; //Decrement
        
        pthread_cond_signal(&full); //signal full
        printf("Consumer thread %d signaling full\n", consParm->threadNum);

        pthread_mutex_unlock(&mutex); //release
        printf("Consumer thread %d releasing lock\n", consParm->threadNum);

        // write value to the file
        printf("Consumer thread %d pulled %d: %d from position %d\n", consParm -> threadNum, lineNo, value, location);
        fprintf(outFile,"%d\n", value);
    }

    // done.
    fclose(outFile);
    printf("Exiting consumer %d\n",consParm->threadNum);
    return NULL;
}



//+
// Function: main
//
// Purpose:  This function decodes the command line and then starts up the producer
//           and consumer threads.
//-

int main(int argc, const char * argv[]) {
    
    // constants
    const unsigned int maxProducers = 5;
    const unsigned int maxConsumers = 5;
    
    // thread vars
    pthread_t prod_thread[maxProducers];
    pthread_t cons_thread[maxConsumers];
    struct threadParm prod_parm[maxConsumers];
    struct threadParm cons_parm[maxProducers];

    int numProducers = 0;
    int numConsumers = 0;

    // seed the random number generator
    srand48(time(NULL));

    // check that there are 4 arguments, error if otherwise
    if (argc != 4){
        fprintf(stderr,"Usage: %s testNum numProducers numconsumers\n", argv[0]);
        exit(1);
    }
    // convert the testNumber on the command line (argument 1) from string to number.
    // An invalid number will convert as zero
    if ((testNum = atoi(argv[1]))==0){
        fprintf(stderr, "testNum must be greater than 0, you said %s\n",argv[1]);
        exit(1);
    }
    // convert the number of producers on the command line (arg 2) from string to number.
    // An invalid number will convert as zero
    if ((numProducers = atoi(argv[2]))==0){
        fprintf(stderr, "must be at least one producer, you said %s\n",argv[2]);
        exit(1);
    }
    if (numProducers > maxProducers){
        fprintf(stderr, "No more than %d Producers, you said %d\n",maxProducers, numProducers);
        exit(1);
    }
    // convert the number of consumers on the command line (arg 2) from string to number.
    // An invalid number will convert as zero
    if ((numConsumers = atoi(argv[3]))==0){
        fprintf(stderr, "must be at least one consumer, you said %s\n", argv[3]);
        exit(1);
    }
    if (numConsumers > maxConsumers){
        fprintf(stderr, "No more than %d Producers, you said %d\n",maxProducers, numProducers);
        exit(1);
    }
    printf("Test Number %d\n", testNum);
    printf("Number of producers %d\n", numProducers);
    printf("Number of consumers %d\n", numConsumers);

    // start the producers
    for (int i = 0; i < numProducers; i++){
        // race condition. If the consumers start before the producers
        // then they may not see running producers, so incrmeent here.
        pthread_mutex_lock(&mutex);
        numProdRunning++;
        pthread_mutex_unlock(&mutex);

	// specify input data file and thread number
        sprintf(prod_parm[i].fileName,"t%d%d.dat",testNum,i);
        prod_parm[i].threadNum = i;
        printf("Main: starting producer %d with file %s\n", i, prod_parm[i].fileName);
        pthread_create(&prod_thread[i],NULL,producer,&prod_parm[i]);
    }

    for (int i = 0; i < numConsumers; i++){
	// specify output data file and thread number
        sprintf(cons_parm[i].fileName,"out%d%d.dat",testNum,i);
        cons_parm[i].threadNum = i;
        printf("Main: starting consumer %d with file %s\n", i, cons_parm[i].fileName);
        pthread_create(&cons_thread[i],NULL,consumer,&cons_parm[i]);
    }
   
    // wait for threads to complete 
    for (int i = 0; i < numProducers; i++){
        pthread_join(prod_thread[i],NULL);
    }
    for (int i = 0; i < numConsumers; i++){
        pthread_join(cons_thread[i],NULL);
    }

    return 0;
}

//+
// Function: simulate_interrupt
//
// Purpose:  To make the assignment more non-determanistic, this randomly
//           inserts calls to sched_yeild to simulate an interrupt in various
//           locations in the code.
//-

void simulate_interrupt(void){
    if (drand48() < 0.33){
        // 33 peercent chance of yielding
        sched_yield();
    }
}
