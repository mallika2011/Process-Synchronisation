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

ll n, m, k;
sem_t cab, riders, payment;

struct arg
{
    ll type; // 1=Premier  2=Pool
    // char *status; //waitState, onRidePremier, onRidePoolOne, onRidePoolFull
    ll maxWaitTime, rideTime;
};

void *bookCab(void *a)
{
    struct arg *args = (struct arg *)a;
    //wait
    // printf("wait = %d\n", sem_wait(&cab));
    struct timespec ts;
    int s;
    if (clock_gettime(CLOCK_REALTIME, &ts) == -1)
        return -1;

    ts.tv_sec += args->maxWaitTime;
    while ((s = sem_timedwait(&cab, &ts)) == -1 && errno == EINTR)
        continue; /* Restart if interrupted by handler */
    /* Check what happened */
    if (s == -1)
    {
        if (errno == ETIMEDOUT)
            printf("TIMEOUT\n");
        else
            perror("sem_timedwait");
        return NULL;
    }
    else
        printf("sem_timedwait() succeeded\n");

    //critical section

    printf("\n-----------------------------------\n");
    printf("RIDING NOW (Ride time : %lld)\n", args->rideTime);
    printf("-----------------------------------\n");
    sleep(args->rideTime);

    //signal
    sem_post(&cab);
    // printf("### RIDE COMPLETED ###\n");
}

int main(void)
{
    printf("\n**************************************\n");
    printf("Please enter the number of cab, riders & payment servers\n");
    scanf("%lld%lld%lld", &n, &m, &k);
    sem_init(&cab, 1, n); //initalising cab semaphore
    sem_init(&riders, 1, m);
    sem_init(&payment, 1, k);

    struct arg args;
    char type_char[10000];

    while (1)
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

        pthread_t t;
        pthread_create(&t, NULL, bookCab, &args);
    }
}
