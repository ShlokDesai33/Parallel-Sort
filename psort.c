#include <stdio.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/stat.h>
#include <fcntl.h>

// --------- globals --------- //

struct arg_struct {
  int l;
  int r;
};

int **arr = NULL;

// --------------------------- //


// --------- helper functions --------- //

// merges and sorts two sub-arrays
void merge(int l, int m, int r) {
  int i, j, k;
  int n1 = m - l + 1;
  int n2 = r - m;

  /* create temp arrays */
  int *L[n1], *R[n2];

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
    if (*L[i] <= *R[j]) {
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

// thread execution starting point
void* parallel_sort(void *arguments) {
  struct arg_struct *fnc_args = arguments;
  mergeSort(fnc_args->l, fnc_args->r);
  free(arguments); arguments = NULL;
  pthread_exit(NULL);
}

// ------------------------------------ //


// --------- main function --------- //

int main(int argc, char *argv[]) {
  if (argc != 4) {
    printf("Usage:\n\tpsort <input file> <output file> <# of threads>\n");
    exit(1);
  }

  // set the number of threads to be used
  int thread_count = atoi(argv[3]);
  pthread_t threads[thread_count];

  // read file size
  struct stat st;
  if (stat(argv[1], &st) == -1) {
    printf("Error: Could not open input file.\n");
    exit(1);
  }
  int size = st.st_size;
  // count number of records in file
  int arr_size = size / 100;
  int subarray_size = arr_size / thread_count;

  // map file into proc addr space and init array
  int fd = open(argv[1], O_RDONLY, S_IRUSR | S_IWUSR);
  char *file = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
  arr = malloc(sizeof(int*) * arr_size);

  for (int i = 0, j = 0; i < size; i += 100) {
    arr[j] = (int*)&file[i];
    j++;
  }

  //  IO ends here; parallel sort starts here

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

  //  IO starts here; parallel sort ends here

  // open output file and write to it
  FILE *output_file = fopen(argv[2], "w");
  if (output_file == NULL) {
    printf("Error: Cannot open output file.\n");
    exit(1);
  }

  for (int i = 0; i < arr_size; i++) {
    char *ptr = (char*)arr[i];
    // copy the next 100 bytes into the output file
    for (int j = 0; j< 100; j++) {
      fprintf(output_file, "%c", *(ptr + j));
    }
  }

  return 0;
}

// ------------------------------- - //
