# Q2 Automating Biryani Serving

## Usage

```bash
    <m:number of chefs>
    <n:number of tables/containers>
    <k:number of students>
```

The program then randomly generates the the remaining parameters.

## Important functions used

* void *wait_for_slot(void *count)
Invoked whenever a new student arrives. The students then constantly poll on the containers until he/she finds a table with an empty slot.

* void *ready_to_serve(void *ind)
Invoked when the a container on a table is filled with a biryani vessel from any one of the chefs.
This then generates a random number of slots available on the table for students.

* void biryani_ready(ll ind)
Invoked when a chef is ready after spending w seconds to prepare r vessels of biryani.

* void *create_Cook_Chef(void *arg)
Invoked by each chef thread wherein, the chef cooks r vessels of biryani for w seconds with a capacity to feed p students.

## Explanation

* An array of mutext locks is used for the containers and for the chefs.
```c
pthread_t chefThread[NMAX], containerThread[NMAX], studentThread[NMAX];
```

* Each chef is a thread
```c
    for (ll i = 0; i < m; i++)
    {
        pthread_create(&chefThread[i], NULL, create_Cook_Chef, (void *)i);
    }
```

* The chefs are constantly polling ont he biryani containers to find an empty container.
```c
void biryani_ready(ll ind)
{
    // printf("Entered biryani ready\n");

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
                    printf("\033[01;33mChef %lld found an empty container at table %lld\n\033[0m", ind, j);
                    pthread_mutex_lock(&chef_lock[ind]);
                    printf("\033[01;33mChef %lld emptying biryani\n\033[0m", ind);
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
```

* As an when a student arrives, it checks for an empty slot on any biryani table and acquires it.

* It is assumed that x number of students come every 3 seconds.
```c
    while (remaining > 0) //loop until all students have arrived
    {
        students_now = rand() % remaining + 1;
        sleep(3); // assuming students come every 3 seconds
        // students_now = 2;
        
        printf("\033[1;36m\n************************************************\nTOTAL OF %lld STUDENTS ARRIVED\n************************************************\n\n\033[0m",students_now);

        for (ll i = 0; i < students_now; i++)
        {
            // printf("lala\n");
            pthread_create(&studentThread[count], NULL, wait_for_slot, (void *)count);
            count++;
        }
        // printf("decrementing remainig\n");
        remaining -= students_now;
    }
```

* The slots cannot be reused by students.
