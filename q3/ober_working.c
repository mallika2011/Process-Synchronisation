
#define _POSIX_C_SOURCE 199309L //required for clock
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include <limits.h>
#include <fcntl.h>
#include <pthread.h>
#include <inttypes.h>
#include <math.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <semaphore.h>
#include <time.h>
#include <errno.h>
#define ll long long

ll interval, t, mwt, rd, endPoolTime, n, m, k, count = 0, poolFlag[1000], timeoutFlag[1000];
sem_t cab, riders, payment, pool;
pthread_t cabThread[10000], paymentThread, poolThread, timeoutThread;
pthread_mutex_t mutex, poolLock;
struct timespec ts;

struct arg
{
    ll type; // 1=Premier  2=Pool
    ll maxWaitTime, rideTime, arrival;
    ll riderno;
} args[1000];

struct allCabs
{
    int type;      // premier =1, pool =2
    int occupancy; // 0 no riders, 1 one rider, 2 two riders
} allCabs[1000];

char type_char[10000];

// void prompt(ll i)
// {
//     args[i].arrival = rand() % 10;
//     // sleep(interval);
//     args[i].type = rand() % 2 + 1;
//     args[i].maxWaitTime = rand() % 5 + 1;
//     args[i].rideTime = rand() % 10 + 1;
//     args[i].riderno = i;
// }

void prompt(ll i)
{
    // printf("<USAGE> <arrival, type, maxWaitTime, rideTime>\n");
    scanf("%lld", &args[i].arrival);
    scanf("%lld", &args[i].type);
    scanf("%lld", &args[i].maxWaitTime);
    scanf("%lld", &args[i].rideTime);
    args[i].riderno = i;
}

void makePayment(ll no)
{
    // printf("Came into payment\n");
    //wait
    sem_wait(&payment);

    //critical section
    sleep(2);
    printf("\033[1;32m$$ RIDER %lld PAYMENT DONE $$\n\033[0m", no);
    // printf("\033[0m");

    //signal
    sem_post(&payment);
}

void *forPoolCheck(void *a)
{
    struct arg *riderDetails = (struct arg *)a;
    int assigned = 0, cabnumber;
    int cabType = riderDetails->type;
    ll passengerNumber = riderDetails->riderno;

    pthread_mutex_lock(&poolLock);

    while (poolFlag[passengerNumber] == 0 && timeoutFlag[passengerNumber] == 0)
    {   
        for (ll i = 0; i < n; i++)
        {
            if (allCabs[i].occupancy == 1 && allCabs[i].type == 2)
            {
                // printf("%lld came into pool check and got %lld\n", passengerNumber, i);
                poolFlag[passengerNumber] = 1;
                allCabs[i].occupancy = 2;
                assigned = 1;
                cabnumber = i;
                break;
            }
        }
    }
    pthread_mutex_unlock(&poolLock);
}

void *forTimeoutCheck(void *a)
{
    struct arg *riderDetails = (struct arg *)a;
    int assigned = 0, cabnumber;
    int cabType = riderDetails->type;
    ll passengerNumber = riderDetails->riderno, s;
    if (clock_gettime(CLOCK_REALTIME, &ts) == -1)
        return NULL;
    ts.tv_sec += riderDetails->maxWaitTime;

    while ((s = sem_timedwait(&cab, &ts)) == -1 && errno == EINTR)
        continue;

    if (s == -1 && poolFlag[passengerNumber] == 0)
    {
        if (errno == ETIMEDOUT)
        {
            printf("\033[1;31m\nTIMEOUT: NO CABS FOUND FOR RIDER %lld\n\033[0m", passengerNumber);
            timeoutFlag[passengerNumber] = 1;
        }
        else
            perror("sem_timedwait");
        return NULL;
    }
    else if (poolFlag[passengerNumber] == 1)
    {
        if (s != -1)
            sem_post(&cab);
    }
    else if (poolFlag[passengerNumber] == 0)
    {
        // printf("HERE\n");
        timeoutFlag[passengerNumber] = 2; //successful timed wait
    }
    else
        printf("Not supposed to be here\n");
}

void *bookPoolCab(void *a)
{
    struct arg *riderDetails = (struct arg *)a;
    int assigned = 0, cabnumber;
    int cabType = riderDetails->type;
    ll passengerNumber = riderDetails->riderno;
    sleep(riderDetails->arrival); //At t=arrival this rider arrives

    pthread_mutex_lock(&mutex);
    for (ll i = 0; i < n; i++)
    {
        if (allCabs[i].occupancy == 1 && allCabs[i].type == 2) //singly occupied
        {
            // printf("%lld found cab %lld\n", passengerNumber, i);
            allCabs[i].occupancy = 2;
            assigned = 1;
            cabnumber = i;
            poolFlag[passengerNumber] = 1;
            break;
        }
    }
    pthread_mutex_unlock(&mutex);
    //wait
    printf("\033[01;33m\nLooking for cabs...\n\033[0m");
    // printf("Assigned %d\n", assigned);
    printf("\n");
    int s;
    if (assigned == 1)
        ;
    else
    {
        //create one thread to check on the timeout
        pthread_create(&timeoutThread, NULL, forTimeoutCheck, (void *)riderDetails);
        //create one thread to check ont the pool status
        pthread_create(&poolThread, NULL, forPoolCheck, (void *)riderDetails);

        //busy wait on the threads' result
        while (poolFlag[passengerNumber] == 0 && timeoutFlag[passengerNumber] == 0)
            ;
    }
    //critical section
    // printf("In critical section pool flag =%lld     timeout flag = %lld\n", poolFlag[passengerNumber],timeoutFlag[passengerNumber] );
    pthread_mutex_lock(&mutex);    
    if (poolFlag[passengerNumber] == 0 && timeoutFlag[passengerNumber] == 2) //no vacancy in pool cabs but a fresh cab found
    {
        for (ll i = 0; i < n && assigned != 1; i++)
        {
            if (allCabs[i].occupancy == 0)
            {
                cabnumber = i;
                allCabs[i].occupancy = 1;
                allCabs[i].type = 2;
                break;
            }
        }
    }
    pthread_mutex_unlock(&mutex);

    printf("\033[1;32m\n-----------------------------------\nRIDER %lld RIDING NOW IN %d\n-----------------------------------\n\033[0m", passengerNumber, cabnumber);
    sleep(riderDetails->rideTime);

    //signal
    int sig = 0;
    pthread_mutex_lock(&mutex);
    allCabs[cabnumber].occupancy--;
    if (allCabs[cabnumber].occupancy == 0)
        sig = 1;
    pthread_mutex_unlock(&mutex);
    if (sig == 1)
    {
        sem_post(&cab);
    }
    printf("\033[1;35m\n### RIDER %lld RIDE COMPLETED IN %d ###\n\033[0m", passengerNumber, cabnumber);
    makePayment(passengerNumber);
}

void *bookPremierCab(void *a)
{
    struct arg *riderDetails = (struct arg *)a;
    int assigned = 0, cabnumber;
    int cabType = riderDetails->type;
    ll passengerNumber = riderDetails->riderno;
    sleep(riderDetails->arrival); //At t=arrival this rider arrives

    //wait
    printf("\033[01;33m\nLooking for cabs...\n\033[0m");
    int s;

    if (clock_gettime(CLOCK_REALTIME, &ts) == -1)
        return NULL;
    ts.tv_sec += riderDetails->maxWaitTime;

    while ((s = sem_timedwait(&cab, &ts)) == -1 && errno == EINTR)
        continue;
    if (s == -1)
    {
        if (errno == ETIMEDOUT)
        {
            printf("\033[1;31m\nTIMEOUT: NO CABS FOUND\n\033[0m");
        }
        else
            perror("sem_timedwait");
        return NULL;
    }

    //critical section
    pthread_mutex_lock(&mutex);
    for (ll i = 0; i < n; i++)
    {
        if (allCabs[i].occupancy == 0)
        {
            cabnumber = i;
            allCabs[i].occupancy = 1;
            allCabs[i].type = 1;
            break;
        }
    }
    pthread_mutex_unlock(&mutex);

    printf("\033[1;32m\n-----------------------------------\nRIDER %lld RIDING NOW IN %d\n-----------------------------------\n\033[0m", passengerNumber, cabnumber);
    sleep(riderDetails->rideTime);

    //signal
    sem_post(&cab);
    allCabs[cabnumber].occupancy--;
    printf("\033[1;35m\n### RIDER %lld RIDE COMPLETED IN %d ###\n\033[0m", passengerNumber, cabnumber);
    makePayment(passengerNumber);
}

int main(void)
{
    srand(time(0));
    printf("\033[0m");
    scanf("%lld%lld%lld", &n, &m, &k);

    sem_init(&cab, 1, n);                //initalising cab semaphore
    sem_init(&riders, 1, m);             //initialising rider semaphore
    sem_init(&payment, 1, k);            //initialising payment semaphore
    pthread_mutex_init(&mutex, NULL);    //initialising a mutex lock
    pthread_mutex_init(&poolLock, NULL); //initialising a pool lock

    for (ll i = 0; i < n; i++)
        allCabs[i].occupancy = 0; //initialising all cabs as 0 occupancy

    for (ll i = 0; i < m; i++) //random value generation for all parameters
        prompt(i);

    printf("\033[1;34m");
    printf("\nCAB SERVICE DETAILS:\n");
    for (ll i = 0; i < m; i++)
    {
        printf("At T=%lld    Passenger: %lld    Cab Type: %lld    MaxWaitTime: %lld    RideTime: %lld\n", args[i].arrival, args[i].riderno, args[i].type, args[i].maxWaitTime, args[i].rideTime);
    }
    printf("\033[0m");

    for (ll i = 0; i < m; i++)
    {
        poolFlag[i] = 0;
        timeoutFlag[i] = 0;
        if (args[i].type == 1)
            pthread_create(&cabThread[i], NULL, bookPremierCab, &args[i]);
        else if (args[i].type == 2)
            pthread_create(&cabThread[i], NULL, bookPoolCab, &args[i]);
    }

    for (ll i = 0; i < n; i++)
        pthread_join(cabThread[i], NULL);
    sem_destroy(&cab);
    pthread_mutex_destroy(&mutex);
    return 0;
}
