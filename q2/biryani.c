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

ll vessels[1000];
ll m, n, k, remaining, w, r, freeSlots=0;
pthread_mutex_t vessel_lock, container_lock, availability;
pthread_t chefThread[1000], containerThread[1000];

struct ch
{
    ll w, r, p, i;
} chefs[1000];

struct cont
{
    ll full; //0 empty, 1 full
    ll p;    //capacity it can serve
} container[1000];

void *wait_for_slot()
{
    //wait till you get a free slot
    while(freeSlots==0);

    //lock a slot
    pthread_mutex_lock(&availability);
    //eat biryani
    pthread_mutex_unlock(&availability);
    
}

//serving biryani on container (table) threads
void *ready_to_serve(void *a)
{
    struct cont *myContainer = (struct cont *)a;
    ll toBeServed=rand()%10+1;  //Randomly generated number of slots to be served
    freeSlots+=toBeServed;


}

void biryani_ready(ll ind)
{
    pthread_mutex_lock(&vessel_lock); //lock vessel mutex

    for (ll i = chefs[ind].r; i > 0; i--)
    {
        pthread_mutex_lock(&container_lock);
        for (ll j = 0; j < n; j++)
        {
            //if a container is empty
            if (container[j].full == 0)
            {
                chefs[ind].r--;
                container[j].p = chefs[ind].p;
                container[j].full = 1;
                //create a thread for this container (table)
                pthread_create(&containerThread[j],NULL,ready_to_serve,&container[j]);
            }
        }
        pthread_mutex_unlock(&container_lock);
    }

    pthread_mutex_unlock(&vessel_lock);
}

//creating chef threads
void *create_Cook_Chef(void *arg)
{
    ll i = (ll)arg;
    chefs[i].w = rand() % 4 + 2;
    chefs[i].r = rand() % 10 + 1;
    chefs[i].p = rand() % 26 + 25;
    chefs[i].i = i;
    vessels[i] = r;
    sleep(w);
    biryani_ready(i);
}

void main()
{
    pthread_mutex_init(&vessel_lock, NULL);    //initialising vessel lock
    pthread_mutex_init(&container_lock, NULL); //initialising container lock

    srand(time(0));
    printf("\033[01;33m");
    printf("Enter m,n,k : ");
    printf("\033[0m;");
    scanf("%lld%lld%lld", &m, &n, &k);
    remaining = k;

    while (remaining > 0) //loop until all students have arrived
    {
        ll x = rand() % k + 1;
        for (ll i = 0; i < x; i++)
        {
        }
    }
}