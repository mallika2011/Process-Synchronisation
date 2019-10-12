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
#define NMAX 1000

ll vessels[NMAX];
ll m, n, k, remaining, w, r, freeSlots = 0, count = 0, table[NMAX], students_now, waiting_students = 0;
pthread_mutex_t container_lock[NMAX], chef_lock[NMAX], container_lock2[NMAX], student_lock;
pthread_t chefThread[NMAX], containerThread[NMAX], studentThread[NMAX];
pthread_cond_t containerCondThread[NMAX];

struct ch
{
    ll w, r, p, i;
} chefs[NMAX];

struct cont
{
    ll isFull; //0 empty, 1 is Full
    ll p;      //capacity it can serve
    ll num;
    ll slots;
} container[NMAX];

void *wait_for_slot(void *count)
{
    ll c = (ll)count;
    int flag = 0;
    printf("Student %lld looking for a free slot\n", c);

    //polling till you get a free slot
    while (1)
    {
        for (ll i = 0; i < n; i++)
        {
            int chk = pthread_mutex_trylock(&container_lock[i]);
            if (chk == 0)
            {
                // printf("locked this container\n");
                if (container[i].slots > 0)
                {
                    //eat biryani
                    printf("Student %lld has recived biryani from container %lld\n", c, i);
                    container[i].slots--;
                    sleep(5); //assuming 5s to eat biryani
                    flag = 1;
                    pthread_mutex_lock(&student_lock);
                    waiting_students--;
                    pthread_mutex_unlock(&student_lock);

                    pthread_mutex_unlock(&container_lock[i]);
                    break;
                }
                //unlock
                pthread_mutex_unlock(&container_lock[i]);
            }
            else
                continue;
        }
        if (flag == 1)
            break;
    }
}

//serving biryani on container (table) threads
void *ready_to_serve(void *ind)
{
    if (waiting_students == 0)
        pthread_exit(NULL);
    ll i = (ll)ind;
    pthread_mutex_lock(&container_lock[i]);
    ll toBeServed = rand() % 10 + 1; //Randomly generated number of slots to be served
    container[i].slots = toBeServed;
    pthread_mutex_unlock(&container_lock[i]);

    printf("No. of slots = %lld and students now = %lld\n", container[i].slots,waiting_students);
    while (container[i].slots != 0 && waiting_students != 0) //Wait
        ;
    printf("Container %lld cannot serve any more slots\n", i);
    // ready_to_serve((void *)i);
}

void biryani_ready(ll ind)
{
    printf("Entered biryani ready\n");

    while (chefs[ind].r > 0 && waiting_students != 0)
    {
        // printf("Trying to fill a container\n");
        for (ll j = 0; j < n; j++)
        {
            //if a container is empty
            int chk = pthread_mutex_trylock(&container_lock[ind]);
            if (chk == 0)
            {
                if (container[j].isFull == 0)
                {
                    printf("Found an empty container with chef %lld\n", ind);
                    pthread_mutex_lock(&chef_lock[ind]);
                    chefs[ind].r--;
                    pthread_mutex_unlock(&chef_lock[ind]);

                    // printf("Now chef %lld has %lld vessels with slots %lld\n", ind, chefs[ind].r, container[j].slots);
                    container[j].p = chefs[ind].p;
                    container[j].isFull = 1;
                    container[j].num = j;
                    
                    //create a thread for this container (table)
                    pthread_mutex_unlock(&container_lock[ind]);
                    pthread_create(&containerThread[j], NULL, ready_to_serve, (void *)j);
                    break;
                }
                pthread_mutex_unlock(&container_lock[ind]);
            }
            else 
                continue;
        }
    }
}

//creating chef threads
void *create_Cook_Chef(void *arg)
{
    if (waiting_students == 0)
        pthread_exit(NULL);
    ll i = (ll)arg;
    pthread_mutex_lock(&chef_lock[i]);
    chefs[i].w = rand() % 4 + 2;
    chefs[i].r = rand() % 5 + 1;
    chefs[i].p = rand() % 26 + 25;
    chefs[i].i = i;
    pthread_mutex_unlock(&chef_lock[i]);
    printf("\033[1;36mChef %lld beginning to prepare %lld vessels \nfor %lld s to feed %lld students\033[0m\n", chefs[i].i, chefs[i].r, chefs[i].w, chefs[i].p);
    sleep(chefs[i].w);

    //biryani is ready:
    biryani_ready(i);

    //check if all vessels have been emptied, if so restart cooking.
    while (chefs[i].r > 0 && waiting_students != 0)
    {
        ;
    }
    create_Cook_Chef((void *)i);
}

void main()
{

    for (ll i = 0; i < NMAX; i++)
    {
        pthread_cond_init(&containerCondThread[i], NULL);
        pthread_mutex_init(&chef_lock[i], NULL);      //initialising vessel lock
        pthread_mutex_init(&container_lock[i], NULL); //initialising container lock
    }
    pthread_mutex_init(&student_lock, NULL);
    srand(time(0));
    printf("\033[01;33m");
    printf("Enter m,n,k : ");
    printf("\033[0m");
    scanf("%lld%lld%lld", &m, &n, &k);
    waiting_students = k;
    remaining=k;
    
    for (ll i = 0; i < m; i++)
    {
        pthread_create(&chefThread[i], NULL, create_Cook_Chef, (void *)i);
    }
    while (remaining > 0) //loop until all students have arrived
    {
        // students_now = rand() % k + 1;
        sleep(3); // assuming students come every 3 seconds
        students_now = 2;
        for (ll i = 0; i < students_now; i++)
        {
            printf("lal\n");
            pthread_create(&studentThread[count], NULL, wait_for_slot, (void *)count);
            count++;
        }
        printf("decrementing remainig\n");
        remaining -= students_now;
    }

    for(ll i=0; i< m; i++)
        pthread_join(chefThread[i],NULL);
    for(ll j=0; j< n; j++)
        pthread_join(containerThread[j],NULL);
    for(ll i=0;i<k;i++)
        pthread_join(studentThread[i],NULL);

 }