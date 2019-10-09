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
    printf("\033[0m");
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
    printf("\033[0m");
}

void *makePayment(void *a)
{
    //wait
    sem_wait(&payment);

    //critical section
    sleep(2);
    printf("\033[0;32m");
    printf("$$  PAYMENT DONE $$\n");
    printf("\033[0m");

    //signal
    sem_post(&payment);
}

void *bookCab(void *a)
{
    struct arg *args = (struct arg *)a;

    //wait
    printf("\033[0;33m");
    printf("\nLooking for cabs...\n");
    printf("\033[0m");
    int s;
    if (clock_gettime(CLOCK_REALTIME, &ts) == -1)
        return -1;

    ll e = ts.tv_sec;
    ts.tv_sec += args->maxWaitTime;

    if (args->type == 2)
    {
        endPoolTime = e + args->rideTime;
        latestPoolThread = cabThread;
        // printf("Latest pool thread = %d\n", latestPoolThread);
    }

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
    printf("\033[0;32m");
    printf("\n-----------------------------------\n");
    printf("RIDING NOW (Ride time : %lld)\n", args->rideTime);
    printf("-----------------------------------\n");
    printf("\033[0m");
    sleep(args->rideTime);

    //signal
    sem_post(&cab);
    if (poolFlag == 1)
        poolFlag = 0;

    printf("\033[0;35m");
    printf("\n### RIDE COMPLETED ###\n");
    printf("\033[0m");
    pthread_create(&paymentThread, NULL, makePayment, NULL);
}

int main(void)
{
    printf("\033[0m");
    printf("\n**************************************\n");
    printf("\nPlease enter the number of cab, riders & payment servers\n");
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
            sleep(100);
            break;
        }
        // printf("count %lld\n", count);
        count++;
        prompt();
        if (args.type == 2 && poolFlag == 1)
        {
            printf("\033[0;33m");
            printf("POOLED INTO AN EXISTING CAB\n");
            printf("\033[0m");

            struct timespec s;
            if (clock_gettime(CLOCK_REALTIME, &s) == -1)
                return -1;

            printf(" ^^^^^^^^^^^^^^  End pool time = %lld            CUrrent time %ld              New ridetime %lld\n", endPoolTime, s.tv_sec, args.rideTime);
            if (endPoolTime - s.tv_sec > args.rideTime)
            {
                // printf("---------continuing------------\n");
                continue;
            }
            else
            {
                // printf("Cancelling the latest thread %d\n",latestPoolThread);
                // pthread_cancel(latestPoolThread);
                pthread_cancel(latestPoolThread);
                printf("Cancelled sem =%d\n", cab);
                sem_post(&cab);
                printf("new sem =%d\n", cab);

                pthread_create(&cabThread, NULL, bookCab, &args);
            }
            poolFlag = 0;
        }
        else
        {
            pthread_create(&cabThread, NULL, bookCab, &args);
        }

        if (args.type == 2 && poolFlag == 0)
        {
            // printf("GOING IN \n");
            poolFlag = 1;
        }
    }
    return 0;
}
