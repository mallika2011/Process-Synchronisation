# Q1 CONCURRENT QUICK SORT

## Usage

Enter the following arguments:

```bash
    n
    [Integers from 1-n]
```

## Functions used

* To create shared memory:

```c
    int *shareMem(size_t size)
```

* Multiprocess Quick Sort:

```c
    void quickSort(int *arr, int l, int r)
```

* Threaded Quick Sort:

```c
    void *t_quickSort(void* a)
```

* Partition Function :

```c
    int partition(int arr[], int l, int r)
```

## Overview

The program takes an input 'n' followed by 'n' integer inputs.
First it is sorted by the multiprocess quick sort.
Next a copy of the same array is sorted by a multithreaded quicksort.
The running time for each of these is noted.
It is observed that the Multithreaded quicksort is faster than the multiprocess quicksort.

## Explanation

* In the Multithreaded quicksort, each of the two halves of the array (left and write) is sorted on separate threads.
The function waits for the threads to then join once they have independently finished sorting.
For array size < 5 an insertion sort is performed.

```c
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
```

* In the Multiprocess quicksort, each of the two halves of the array is sorted as a different process by forking twice.
The parent process then waits for the two children processes to finish sorting and integrates them together.

* The Mutithreaded quicksort is faster than Multiprocess since creation of threads is easier (thread pool), there is lesser context switching and threads can communicate between each other more easily.

```c
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
```
