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

ll n, m, k, count = 0, poolFlag = 0;
sem_t cab, riders, payment;
pthread_t cabThread, paymentThread, latestPoolThread;
ll endPoolTime;
struct timespec ts;

struct arg
{
    ll type; // 1=Premier  2=Pool
    // char *status; //waitState, onRidePremier, onRidePoolOne, onRidePoolFull
    ll maxWaitTime, rideTime;
};
struct arg args;
char type_char[10000];

void prompt()
{
    printf("\nEnter the following details to book a cab\n1.Premier\n2.Pool\n");
    printf("Type : ");
    scanf("%lld", &args.type);
    printf("\n");
    printf("Max Wait Time: ");
    scanf("%lld", &args.maxWaitTime);
    printf("\n");
    printf("Ride Time : ");
    scanf("%lld", &args.rideTime);
    printf("\n");
    printf("**************************************\n");
}

void *makePayment(void *a)
{
    //wait
    sem_wait(&payment);

    //critical section
    sleep(2);
    printf("$$  PAYMENT DONE $$\n");

    //
    sem_post(&payment);
}

void *bookCab(void *a)
{
    struct arg *args = (struct arg *)a;

    //wait
    printf("Looking for cabs...\n");
    int s;
    if (clock_gettime(CLOCK_REALTIME, &ts) == -1)
        return -1;

    ts.tv_sec += args->maxWaitTime;

    // if (args->type == 2)
    // {
    //     endPoolTime = ts.tv_sec;
    //     latestPoolThread = cabThread;
    // }

    while ((s = sem_timedwait(&cab, &ts)) == -1 && errno == EINTR)
        continue;
    if (s == -1)
    {
        if (errno == ETIMEDOUT)
            printf("TIMEOUT: NO CABS FOUND\n");
        else
            perror("sem_timedwait");
        return NULL;
    }

    //critical section
    printf("\n-----------------------------------\n");
    printf("RIDING NOW (Ride time : %lld)\n", args->rideTime);
    printf("-----------------------------------\n");
    sleep(args->rideTime);

    //signal
    sem_post(&cab);
    // if(poolFlag==1)
    //     poolFlag=0;
    printf("### RIDE COMPLETED ###\n");
    pthread_create(&paymentThread, NULL, makePayment, NULL);
}

int main(void)
{
    printf("\n**************************************\n");
    printf("Please enter the number of cab, riders & payment servers\n");
    scanf("%lld%lld%lld", &n, &m, &k);
    sem_init(&cab, 1, n); //initalising cab semaphore
    sem_init(&riders, 1, m);
    sem_init(&payment, 1, k);

    while (1)
    {

        if (count == m)
        {
            sem_destroy(&cab);
            sem_destroy(&riders);
            sem_destroy(&payment);
            // return 0;
            sleep(60);
            break;
        }
        printf("count %lld\n", count);
        count++;
        prompt();
        // if (args.type == 2 && poolFlag == 1)
        // {
        //     printf("POOLED INTO AN EXISTING CAB\n");
        //     struct timespec s;
        //     if (clock_gettime(CLOCK_REALTIME, &s) == -1)
        //         return -1;
        //     if (endPoolTime-s.tv_sec  > args.rideTime)
        //         continue;

        //     else
        //         pthread_cancel(latestPoolThread);
        //     pthread_create(&cabThread,NULL,bookCab,&args);
        //     poolFlag = 0;
        // }
        // if (args.type == 2 && poolFlag == 0)
        // {
        //     poolFlag = 1;
        // }
        pthread_create(&cabThread, NULL, bookCab, &args);
    }
    return 0;
}