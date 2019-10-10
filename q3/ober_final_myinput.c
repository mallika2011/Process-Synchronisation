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

ll interval, t, mwt, rd, endPoolTime, n, m, k, count = 0, poolFlag = 0.;
sem_t cab, riders, payment;
pthread_t cabThread[10000], paymentThread, latestPoolThread;
pthread_mutex_t mutex;
struct timespec ts;

struct arg
{
    ll type; // 1=Premier  2=Pool
    // char *status; //waitState, onRidePremier, onRidePoolOne, onRidePoolFull
    ll maxWaitTime, rideTime,arrival;
    ll riderno;
} args[1000];

struct allCabs
{
    int type;      // premier =1, pool =2
    int occupancy; // 0 no riders, 1 one rider, 2 two riders
    int poolflag;
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
    // printf("\nEnter the following details to book a cab\n1.Premier\n2.Pool\n");
    printf("Arrival: ");
    scanf("%lld", &args[i].arrival);
    printf("Type : ");
    scanf("%lld", &args[i].type);
    printf("\n");
    printf("Max Wait Time: ");
    scanf("%lld", &args[i].maxWaitTime);
    printf("\n");
    printf("Ride Time : ");
    scanf("%lld", &args[i].rideTime);
    printf("\n");
    printf("**************************************\n");
    args[i].riderno=i;
}

void *makePayment(void *a)
{
    //wait
    sem_wait(&payment);

    //critical section
    sleep(2);
    printf("\033[1;32m");
    printf("$$  PAYMENT DONE $$\n");
    printf("\033[0m");

    //signal
    sem_post(&payment);
}

void *bookPoolCab(void *a)
{
    struct arg *riderDetails = (struct arg *)a;
    int assigned = 0, cabnumber;
    int cabType = riderDetails->type;
    ll passengerNumber = riderDetails->riderno;
    sleep(riderDetails->arrival);               //At t=arrival this rider arrives

    pthread_mutex_lock(&mutex);
    for (ll i = 0; i < n; i++)
    {
        if (allCabs[i].occupancy == 1 && allCabs[i].type == 2) //singly occupied
        {
            allCabs[i].occupancy = 2;
            assigned = 1;
            cabnumber = i;
            break;
        }
    }
    pthread_mutex_unlock(&mutex);

    //wait
    printf("\033[01;33m");
    // printf("\nLooking for cabs...\n");
    printf("Assigned = %d\n",assigned);
    printf("\033[0m");
    int s;
    if (assigned == 1)
        ;
    else
    {
        if (clock_gettime(CLOCK_REALTIME, &ts) == -1)
            return -1;
        ts.tv_sec += riderDetails->maxWaitTime;

        while ((s = sem_timedwait(&cab, &ts)) == -1 && errno == EINTR)
            continue;
        if (s == -1)
        {
            if (errno == ETIMEDOUT)
            {
                printf("\033[1;31m");
                printf("\nTIMEOUT: NO CABS FOUND\n");
                printf("\033[0m");
            }
            else
                perror("sem_timedwait");
            return NULL;
        }
        // sem_wait(&cab);
    }
    //critical section
    pthread_mutex_lock(&mutex);
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
    pthread_mutex_unlock(&mutex);
    printf("\033[1;32m");
    printf("\n-----------------------------------\n");
    printf("RIDER %lld RIDING NOW IN %d\n", passengerNumber, cabnumber);
    printf("-----------------------------------\n");
    printf("\033[0m");
    sleep(riderDetails->rideTime);
    // printf("AFTER SLEEEP RIDERNO = %lld         %lld\n", riderDetails->riderno, passengerNumber);
    //signal
    sem_post(&cab);
    allCabs[cabnumber].occupancy--;
    printf("\033[1;35m");
    printf("\n### RIDER %lld RIDE COMPLETED IN %d ###\n", passengerNumber, cabnumber);
    printf("\033[0m");
    pthread_create(&paymentThread, NULL, makePayment, NULL);
}

void *bookPremierCab(void *a)
{
    struct arg *riderDetails = (struct arg *)a;
    int assigned = 0, cabnumber;
    int cabType = riderDetails->type;
    ll passengerNumber = riderDetails->riderno;
    sleep(riderDetails->arrival);               //At t=arrival this rider arrives

    //wait
    printf("\033[01;33m");
    printf("\nLooking for cabs...\n");
    printf("\033[0m");
    int s;

    if (clock_gettime(CLOCK_REALTIME, &ts) == -1)
        return -1;
    ts.tv_sec += riderDetails->maxWaitTime;

    while ((s = sem_timedwait(&cab, &ts)) == -1 && errno == EINTR)
        continue;
    if (s == -1)
    {
        if (errno == ETIMEDOUT)
        {
            printf("\033[1;31m");
            printf("\nTIMEOUT: NO CABS FOUND\n");
            printf("\033[0m");
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

    printf("\033[1;32m");
    printf("\n-----------------------------------\n");
    printf("RIDER %lld RIDING NOW IN %d\n", passengerNumber, cabnumber);
    printf("-----------------------------------\n");
    printf("\033[0m");
    sleep(riderDetails->rideTime);

    //signal
    sem_post(&cab);
    allCabs[cabnumber].occupancy--;
    printf("\033[1;35m");
    printf("\n### RIDER %lld RIDE COMPLETED IN %d ###\n", passengerNumber, cabnumber);
    printf("\033[0m");
    pthread_create(&paymentThread, NULL, makePayment, NULL);
}

int main(void)
{
    srand(time(0));
    printf("\033[0m");
    printf("\n**************************************\n");
    printf("\nPlease enter the number of cab, riders & payment servers\n");

    scanf("%lld%lld%lld", &n, &m, &k);

    sem_init(&cab, 1, n);             //initalising cab semaphore
    sem_init(&riders, 1, m);          //initialising rider semaphore
    sem_init(&payment, 1, k);         //initialising payment semaphore
    pthread_mutex_init(&mutex, NULL); //initialising a mutex lock

    for (ll i = 0; i < n; i++)
        allCabs[i].occupancy = 0; //initialising all cabs as 0 occupancy

    for (ll i = 0; i < m; i++) //random value generation for all parameters
        prompt(i);

    printf("\033[1;34m");
    printf("\nCAB SERVICE DETAILS:\n");
    for (ll i = 0; i < m; i++)
    {
        printf("At T=%lld    Passenger: %lld    Cab Type: %lld    MaxWaitTime: %lld    RideTime: %lld\n",args[i].arrival, args[i].riderno,args[i].type, args[i].maxWaitTime,args[i].rideTime);
    }
    printf("\033[0m");

    for (ll i = 0; i < m; i++)
    {
        if (args[i].type == 1)
            pthread_create(&cabThread[i], NULL, bookPremierCab, &args[i]);
        else if (args[i].type == 2)
            pthread_create(&cabThread[i], NULL, bookPoolCab, &args[i]);
    }

    for (ll i = 0; i < n; i++)
        pthread_join(cabThread[i], NULL);
    sleep(60);
    sem_destroy(&cab);
    pthread_mutex_destroy(&mutex);
    return 0;
}
