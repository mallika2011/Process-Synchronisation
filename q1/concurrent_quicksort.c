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
struct arg
{
    int l;
    int r;
    int *arr;
};

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

void quickSort(int *arr, int l, int r)
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




void *t_quickSort(void* a){

    //note that we are passing a struct to the threads for simplicity.
    struct arg *args = (struct arg*) a;

    int l = args->l;
    int r = args->r;
    int *arr = args->arr;
    if(l>r) return NULL;    
    
    //insertion sort
    if(r-l+1<=5){

        for(int i=l+1-1;i<r;i++)
        {

            int k=i-1; 
            int t=arr[i];
            while(k>=0 && arr[k]>t)
            {
                arr[k+1]=arr[k];
                k--;
            }
            arr[k+1]=t;
        }

        return NULL;
    }

    int ind = partition(arr, l, r);
    //sort left half array
    struct arg a1;
    a1.l = l;
    a1.r = ind-1;
    a1.arr = arr;
    pthread_t tid1;
    pthread_create(&tid1, NULL, t_quickSort, &a1);
    
    //sort right half array
    struct arg a2;
    a2.l = ind+1;
    a2.r = r;
    a2.arr = arr;
    pthread_t tid2;
    pthread_create(&tid2, NULL, t_quickSort, &a2);
    
    //wait for the two halves to get sorted
    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);
}

int main(void)
{
    struct timespec ts;
    scanf("%lld", &n);

    //Creating shared memory
    int *arr = shareMem(sizeof(int) * (n + 1));
    int *arr2=arr; // copy array
    for (int i = 0; i < n; i++)
        scanf("%d", arr + i);

    //Running Multiprocess quicksort:
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    long double start = ts.tv_nsec / (1e9) + ts.tv_sec;

    quickSort(arr, 0, n - 1);

    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    long double end = ts.tv_nsec / (1e9) + ts.tv_sec;
    long double t1 = end - start;

    printf("\nSorted using multiprocess quicksort:\n");
    for (int i = 0; i < n; i++) //Printing the sorted array
        printf("%d ", arr[i]);
    printf("\n");

    //Running multithreaded quicksort:
    pthread_t tid;
    struct arg a;
    a.l = 0;
    a.r = n - 1;
    a.arr = arr2;

    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    start = ts.tv_nsec / (1e9) + ts.tv_sec;

    pthread_create(&tid, NULL, t_quickSort, &a);
    pthread_join(tid, NULL);
    
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    end = ts.tv_nsec / (1e9) + ts.tv_sec;
    long double t2 = end - start;

    printf("\nSorted using multiprocess quicksort:\n");
    for (int i = 0; i < n; i++) // Printing the sorted array
        printf("%d ", arr2[i]);
    printf("\n");

    printf("\nRunning time for individual sorts : \nMultiprocess: %Lf\nMultithreaded: %Lf\n", t1, t2);
    shmdt(arr);

    return 0;
}