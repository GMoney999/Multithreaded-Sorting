//// MULTITHREADED SORTING PROGRAM
/// Note: No Mutex is used
/// To incorporate the mutex into your program, you would typically do so in areas where threads access or modify shared resources concurrently.
/// This program does not need a Mutex for 2 reasons:
///     1. There's no evident concurrent modification of shared resources by threads that would necessitate a Mutex for synchronization.
///     2. The sorting and merging operations are structured to work on distinct data segments or are sequenced in a way (sorting first, followed by merging) that inherently avoids concurrent access issues.

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

//// GLOBALS
/// These variables hold our global data which we will handle across threads

// A SIZE constant which we can use when allocating memory for each subarray
#define SIZE (sizeof(list) / sizeof(*list))
// Original unsorted global array
int list[] = {7, 12, 19, 3, 18, 4, 2, -5, 6, 15, 8};
// Global array to hold original array when sorted
int result[SIZE] = {0};


//// STRUCTS
/// These structs encapsulate and organize the necessary data for sorting and merging operations in a way that's easy to manage and pass between threads.

// For the 2 sorting threads
typedef struct {
    // a pointer to the beginning of the subarray that the thread will sort.
    // In C, arrays are often managed through pointers
    int* subArray;
    // This tells the thread how many elements are in the subarray it needs to sort.
    unsigned int size;
} SortingThreadParameters;

// For merging thread
typedef struct {
    // Uses the same SortingThreadParameters structure to say, "Here's the first sorted part you need to merge."
    SortingThreadParameters left;
    SortingThreadParameters right;
} MergingThreadParameters;


//// FUNCTION PROTOTYPES
void* sorting_thread(void* arg);
void* merging_thread(void* arg);
void sort(int* subArray, unsigned int size);
void merge_sort(int* arr, int start, int end, int* result);
void merge(int* arr, int start, int mid, int end, int* result);


int main() {
    //// INITIALIZE THREADS
    pthread_t leftThread, rightThread, mergeThread;


    //// ALLOCATION OF SORTING THEAD PARAMETERS
    /// 1. Create a pointer to new allocated memory to hold thread parameters
    /// 2. Set each thread parameter (pointer to subArray and size)

    //  This allocates memory on the heap that is large enough to store a SortingThreadParameters structure.
    //  The malloc function returns a pointer to the allocated memory.
    SortingThreadParameters* paramsLeft = malloc(sizeof(SortingThreadParameters));
    // Set the subArray pointer to the beginning of global array (left half)
    // Essentially, this tells the thread that its working array starts at the beginning of list.
    paramsLeft->subArray = list;
    // Set the size to half the total size of the array.
    // It indicates how many elements the thread should consider for sorting.
    // If SIZE is the total number of elements in list, then SIZE / 2 is half of that, rounding down if SIZE is odd.
    paramsLeft->size = SIZE / 2;

    // Prepare another set of thread parameters for the right half of the array
    // "paramsRight" variable points to the beginning of the memory chunk holding the parameters.
    SortingThreadParameters* paramsRight = malloc(sizeof(SortingThreadParameters));
    // Set the starting point of the right half of the array to the midpoint of the array
    // "list" points to the beginning of the array, and "+ paramsLeft->size" moves the pointer
    // to the end of the left array, which is where the right half begins.
    paramsRight->subArray = list + paramsLeft->size;
    // Set the size of the right half of the array, which is the full size of the array minus the left half
    paramsRight->size = SIZE - paramsLeft->size;


    //// CREATING THE SORTING THREADS
    /// pthread_create()  is used to create a new thread in a POSIX-compliant system.
    /// takes 4 arguments:
    ///   1. pthread_t *thread
    ///         A pointer to a pthread variable that will store the thread ID of the newly created thread.
    ///         This variable can be used later to join the thread
    ///   2. const pthread_attr_t *attr
    ///         Points to the attributes for the thread.
    ///         Passing NULL means the thread is created with default attributes.
    ///   3. void *(*start_routine) (void *)
    ///         The function that the thread will execute once it starts.
    ///         This function must take a single void* argument and return a void*
    ///  4. void *arg
    ///         The argument that will be passed to the start_routine function.
    ///         This allows you to pass data to the thread when it starts.

    //  Create a new thread (leftThread), which will run the sorting_thread function with paramsLeft as its argument.
    //  This will handle the left half of the array that needs sorting.
    if (pthread_create(&leftThread, NULL, sorting_thread, (void*)paramsLeft) != 0) {
        perror("Failed to create thread1");
        return 1;
    }

    // Create a second thread for the right half of the array.
    // Calls the same 'sorting_thread' function, only with the right side parameters instead of left side.
    if (pthread_create(&rightThread, NULL, sorting_thread, (void*)paramsRight) != 0) {
        perror("Failed to create thread1");
        return 1;
    }


    //// WAIT FOR SORTING THREADS TO COMPLETE
    /// The pthread_join function is used to wait for a specific thread to finish executing

    // Tells the main thread to wait here until thread1 (the left sorting thread) has finished executing.
    // If thread1 is already complete by the time this line runs, pthread_join will return immediately.
    // Otherwise, it will block/pause the main thread until thread1 completes.
    // The NULL argument indicates that the main thread does not need to capture any exit status from thread1.
    pthread_join(leftThread, NULL);
    pthread_join(rightThread, NULL);


    //// ALLOCATION OF MERGING THEAD PARAMETERS
    MergingThreadParameters* paramsMerge = malloc(sizeof(MergingThreadParameters));
    // The left field of the paramsMerge structure is populated with the data pointed to by paramsLeft.
    // This effectively copies the sorting parameters for the left half of the array
    // (which include the pointer to the subarray and its size) into the left part of the paramsMerge structure.
    // The dereference operator * is used here because paramsLeft is a pointer to a SortingThreadParameters structure,
    // and you need to store the actual structure (not the pointer) in paramsMerge->left.
    paramsMerge->left = *paramsLeft;
    // Similarly, the right field of the paramsMerge structure is populated with the data pointed to by paramsRight,
    // copying the sorting parameters for the right half of the array into the right part of the paramsMerge structure.
    paramsMerge->right = *paramsRight;


    //// CREATE THE MERGING THREAD
    /// Pass in the merging thread parameters that contain both the left and right array
    /// parameters and execute the thread
    pthread_create(&mergeThread, NULL, merging_thread, (void*)paramsMerge);
    pthread_join(mergeThread, NULL);


    //// CLEAN UP MEMORY
    /// Free the previously allocated memory after the merge thread is complete.
    free(paramsLeft);
    free(paramsRight);
    free(paramsMerge);


    //// VERIFY CORRECT RESULTS
    /// Print the array to make sure it is sorted
    for (unsigned int i = 0; i < SIZE; i++) {
        printf("%d ", result[i]);
    }
    printf("\n");

    return 0;
}

//// THREADS
/// Includes a sorting thread for both halves of global array, and a merging thread that copies
/// both sorted halves into the global result array

void* sorting_thread(void* arg) {
    // The arg argument is cast from void* to SortingThreadParameters*.
    // This allows you to access the sorting parameters (subArray and size) passed to the thread.
    // The arg pointer is expected to point to a SortingThreadParameters struct containing the subset of the array this thread will sort.
    SortingThreadParameters* params = (SortingThreadParameters*) arg;

    // Calls the merge_sort function on the subarray defined by params
    merge_sort(params->subArray, 0, params->size - 1, result);

    // Returning NULL  is a common practice for thread routines
    // that perform work but don't need to directly communicate a result back
    return NULL;
}


void* merging_thread(void* arg) {
    //// PARAMETER EXTRACTION
    /// Casts the void* argument back to a MergingThreadParameters*.
    /// This allows the function to access the parameters for the left and right subarrays that need to be merged
    MergingThreadParameters* params = (MergingThreadParameters*) arg;

    //// PREPARING FOR MERGE
    // Initialize parameters to pass to merge function
    int start = 0; // Start of the first subarray
    int mid = params->left.size - 1; // End of the first subarray, assuming 'start' is 0
    int end = params->left.size + params->right.size - 1; // End of the second subarray


    //// MEMORY ALLOCATION
    /// Reserves memory for a temporary array (tempResult) that will store the merged result of the two subarrays.
    /// The size is end + 1 to accommodate the total number of elements from both subarrays.
    int* tempResult = (int*)malloc(sizeof(int) * (end + 1));

    //// ERROR HANDLING
    // Handle memory allocation failure
    if (tempResult == NULL) {
        fprintf(stderr, "Failed to allocate memory for merging.\n");
        exit(EXIT_FAILURE);
    }

    //// MERGING SUBARRAYS
    // Calls the merge function to merge the two sorted subarrays into tempResult.
    merge(params->left.subArray, start, mid, params->left.size + params->right.size - 1, tempResult);


    //// COPY THE MERGED RESULT
    // Copy the merged array back into the appropriate place in the global 'result' array
    for (int i = 0; i <= end; i++) {
        result[i] = tempResult[i];
    }

    //// CLEAN UP ALLOCATED MEMORY
    free(tempResult);

    return NULL;
}


//// SORTING FUNCTIONS (MERGE SORT)

// Helper function that recursively breaks the array in half
void merge_sort(int* arr, int start, int end, int* result) {
    // Base case
    // checks if the starting index is less than the ending index.
    // stops the recursion when the segment to be sorted is reduced to a single element or when the start and end cross over, indicating an invalid range.
    if (start < end) {
        // Get midpoint
        int mid = start + (end - start) / 2;
        // Recursively break the array in half until there is only 1 element
        merge_sort(arr, start, mid, result);
        merge_sort(arr, mid + 1, end, result);
        // Repeatedly merge the two arrays until everything is added to the global sorted array
        merge(arr, start, mid, end, result);
        // Copy the sorted result back into the original array segment
        for (int i = start; i <= end; i++) {
            arr[i] = result[i];
        }
    }
}

//
void merge(int* arr, int start, int mid, int end, int* result) {
    // Initialize three indices.
    // i starts at the beginning of the first sorted subarray.
    // j starts at the beginning of the second sorted subarray (mid + 1).
    // k is used to track the current position in the result array where the
    // next smallest element from either subarray will be placed. It starts at the same position as i
    int i = start, j = mid + 1, k = start;

    // Continues as long as there are elements in both subarrays yet to be compared and merged
    while (i <= mid && j <= end) {
        // Compares the current elements of both subarrays.
        // If the element in the first subarray (arr[i]) is smaller,
        // it is placed into the result array at k, and both i and k are incremented.
        // If the element in the second subarray (arr[j]) is smaller or equal,
        // it is placed into result at k, and both j and k are incremented.
        // This ensures that the merged array is in ascending order.
        if (arr[i] < arr[j]) {
            result[k++] = arr[i++];
        } else {
            result[k++] = arr[j++];
        }
    }
    // if any elements are left in the left or right subarray after the main loop exits,
    // these loops adds the remaining elements from the  subarrays to result.
    while (i <= mid) {
        result[k++] = arr[i++];
    }
    while (j <= end) {
        result[k++] = arr[j++];
    }
}
