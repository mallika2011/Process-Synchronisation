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
#include <time.h>
#include <pthread.h>
#include <inttypes.h>
#include <math.h>

long long int n;

int *shareMem(size_t size)
{
    key_t mem_key = IPC_PRIVATE;
    int shm_id = shmget(mem_key, size, IPC_CREAT | 0666);
    return (int *)shmat(shm_id, NULL, 0);
}

int partition(int arr[], int l, int r)
{
    srand(time(NULL));
    int random = l + rand() % (r - l);
    int swap = arr[random];
    arr[random] = arr[r];
    arr[r] = swap;

    int pivot = arr[r];
    // printf("Pivot = %d\n", pivot);
    int i = (l);
    for (int j = l; j < r; j++)
    {
        if (arr[j] <= pivot)
        {
            int temp = arr[i];
            arr[i] = arr[j];
            arr[j] = temp;
            i++;
        }
    }
    int temp = arr[i];
    arr[i] = arr[r];
    arr[r] = temp;
    return (i);
}

void quickSort(int arr[], int l, int r)
{
    if (r - l + 1 <= 5)
    {
        for (int i = l + 1; i < r; i++)
        {
            int k = i - 1;
            int t = arr[i];
            while (k >= 0 && arr[k] > t)
            {
                arr[k + 1] = arr[k];
                k--;
            }
            arr[k + 1] = t;
        }
    }
    if (l < r)
    {
        int ind = partition(arr, l, r);

        pid_t p1 = fork();
        pid_t p2;

        if (p1 == 0)
        {
            quickSort(arr, l, ind - 1);
            _exit(1);
        }
        else
        {
            p2 = fork();
            if (p2 == 0)
            {
                quickSort(arr, ind + 1, r);
                _exit(1);
            }
            else
            {
                int status;
                waitpid(p1, &status, 0);
                waitpid(p2, &status, 0);
            }
        }
        return;
    }
    else
    {
        _exit(1);
    }
}

struct arg
{
    int l;
    int r;
    int *arr;
};

void *t_quickSort(void *a)
{
    //note that we are passing a struct to the threads for simplicity.
    struct arg *args = (struct arg *)a;

    int l = args->l;
    int r = args->r;
    int *arr = args->arr;
    if (l > r)
        return NULL;

    //insertion sort
    if (r - l + 1 <= 5)
    {
        for (int i = l + 1; i < r; i++)
        {
            int k = i - 1;
            int t = arr[i];
            while (k >= 0 && arr[k] > t)
            {
                arr[k + 1] = arr[k];
                k--;
            }
            arr[k + 1] = t;
        }
    }

    int ind = partition(arr, l, r);

    //sort left half array
    struct arg left;
    left.l = l;
    left.r = ind - 1;
    left.arr = arr;
    pthread_t t1;
    pthread_create(&t1, NULL, t_quickSort, &left);

    //sort right half array
    struct arg right;
    right.l = ind + 1;
    right.r = r;
    right.arr = arr;
    pthread_t t2;
    pthread_create(&t2, NULL, t_quickSort, &right);

    //wait for the two halves to get sorted
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
}

void runSorts(long long int n)
{
}

int main()
{
    scanf("%lld", &n);

    //CREATING SHARED MEMORY
    int *arr = shareMem(sizeof(int) * (n + 1));
    printf("came her1\n");

    int *arr2; // copy arry
    printf("came her2\n");

    for (int i = 0; i < n; i++)
        scanf("%d", arr + i);
    

    //RUNNING MULTIPROCESS QUICKSORT:
    quickSort(arr, 0, n - 1);
    printf("came her3\n");


    for (int i = 0; i < n; i++)
        printf("%d ", arr[i]);
    
    //RUNNING MULTITHREADED QUICKSORT:
    pthread_t tid;
    struct arg a;
    a.l = 0;
    a.r = n - 1;
    a.arr = arr2;
    
    pthread_create(&tid, NULL, t_quickSort, &a);
    pthread_join(tid, NULL);

    for (int i = 0; i < n; i++)
        printf("%d ", arr2[i]);

    shmdt(arr);
    return 0;
}