#include <stdio.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <pthread.h>

// --------- globals --------- //

struct arg_struct {
  int l;
  int r;
};

int arr[] = { 12, 11, 13, 6, 7, 30, 45, 2, 0, 16, 44, 81, 100, 200, 300, 250, 600, 1000 };

// --------------------------- //

// merges and sorts two sub-arrays
void merge(int l, int m, int r) {
  int i, j, k;
  int n1 = m - l + 1;
  int n2 = r - m;

  /* create temp arrays */
  int L[n1], R[n2];

  /* Copy data to temp arrays L[] and R[] */
  for (i = 0; i < n1; i++)
    L[i] = arr[l + i];
  for (j = 0; j < n2; j++)
    R[j] = arr[m + 1 + j];

  /* Merge the temp arrays back into arr[l..r]*/
  i = 0; // Initial index of first subarray
  j = 0; // Initial index of second subarray
  k = l; // Initial index of merged subarray
  while (i < n1 && j < n2) {
    if (L[i] <= R[j]) {
      arr[k] = L[i];
      i++;
    }
    else {
      arr[k] = R[j];
      j++;
    }
    k++;
  }

  /* Copy the remaining elements of L[], if there
  are any */
  while (i < n1) {
    arr[k] = L[i];
    i++;
    k++;
  }

  /* Copy the remaining elements of R[], if there
  are any */
  while (j < n2) {
    arr[k] = R[j];
    j++;
    k++;
  }
}

// recursively sort an array
void mergeSort(int l, int r) {
  if (l < r) {
    int m = l + (r - l) / 2;

    mergeSort(l, m);
    mergeSort(m + 1, r);

    merge(l, m, r);
  }
}

void* parallel_sort(void *arguments) {
  struct arg_struct *fnc_args = arguments;
  mergeSort(fnc_args->l, fnc_args->r);
  free(arguments); arguments = NULL;
  pthread_exit(NULL);
}

void printArray(int A[], int size) {
  int i;
  for (i = 0; i < size; i++)
    printf("%d ", A[i]);
  printf("\n");
}

int main(int argc, char *argv[]) {
  if (argc != 4) {
    printf("Usage:\n\tpsort <input file> <output file> <# of threads>\n");
    exit(1);
  }

  int arr_size = sizeof(arr) / sizeof(arr[0]); // TODO: sizeof(global_arr) / 100
  int thread_count = 4; // atoi(argv[3])
  pthread_t threads[thread_count];
  int subarray_size = arr_size / thread_count;

  printf("Given array is \n");
  printArray(arr, arr_size);

  // create threads
  for (int i = 0; i < thread_count; i++) {
    struct arg_struct *args = malloc(sizeof(struct arg_struct) * 1);
    if (i == thread_count - 1) {
      // last thread
      args->l = i * subarray_size;
      args->r = arr_size - 1;
    }
    else {
      args->l = i * subarray_size;
      args->r = (i + 1) * subarray_size - 1;
    }
    pthread_create(&threads[i], NULL, parallel_sort, args);
  }

  // wait for threads
  for (int i = 0; i < thread_count; i++) {
    pthread_join(threads[i], NULL);
  }

  int m = subarray_size - 1;
  int r = (subarray_size * 2) - 1;
  // merging the final sub-arrays
  for (int i = 0; i < thread_count - 1; i++) {
    merge(0, m, r);
    m += subarray_size; r += subarray_size;
  }

  printf("\nSorted array is \n");
  printArray(arr, arr_size);

  return 0;
}

