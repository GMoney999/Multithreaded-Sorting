# Concurrent Merge Sort in C and Rust
This project demonstrates the implementation of a concurrent merge sort algorithm in both C and Rust, showcasing the differences in handling concurrency and memory safety between the two languages.


## Usage

### Compiling in C
 
    cd multithreaded_sorting_c
    gcc main.c -o main -lpthread
    ./main


### Compiling in Rust

    cd multithreaded_sorting_rust
    cargo build
    cargo run

---

## Program Functionality
The program sorts an integer array using the merge sort algorithm, leveraging concurrency for improved performance. It splits the array into two halves, sorts each half in separate threads, and then merges the sorted halves back together in a final step.



## Features
- `Concurrency` - Utilizes threads to parallelize sorting of array halves.
- `Merge Sort Algorithm` - a divide-and-conquer strategy for sorting.
- `Language-Specific Safety and Concurrency Models` - Showcases Rust's safety guarantees and C's manual memory and thread management.
Design Choices

## C Implementation
- `Thread Management` - Uses POSIX threads (pthread) for creating and managing threads.
- `Memory Management` - Explicitly allocates and frees memory for thread parameters and temporary arrays used during merging.
- `Synchronization` - While a mutex is initialized, it's ultimately not used due to the design ensuring threads operate on distinct segments of data, avoiding concurrent writes.
- `Error Handling` - Manual checks for errors, such as failed thread creation or memory allocation failures.

## Rust Implementation
- `Safety and Concurrency` - Rust's ownership system, alongside types like Mutex, ensures memory safety and data race prevention at compile time.
- `Thread Management` Leverages Rust's std::thread for spawning threads, with a safer API compared to C.
- `Memory Management`  Utilizes Rust's ownership and borrowing rules, reducing the need for explicit memory management seen in C.
- `Synchronization` - Employs a Mutex to safely share and mutate global state across threads, showcasing Rust's approach to handling shared mutable state.

---

## Key Differences
1. `Memory Safety` - Rust provides compile-time guarantees that prevent data races and unsafe memory access, a contrast to C where such safety is the programmer's responsibility.

2. `Concurrency Model` - Rust enforces safe concurrency practices through its type system and standard library, whereas C requires manual synchronization and error handling.


## Difference between Rust threads and POSIX threads

1. `Thread Management` - In Rust, the thread::spawn function makes it easy to create and manage threads. In C, the pthread_create function is used to create threads, requiring a function pointer for the thread's start routine and manage thread identifiers (pthread_t) explicitly.

2. `Data Sharing and Synchronization` - Rust's Mutex provides a high-level abstraction for thread synchronization and shared ownership. In C, usually POSIX mutexes (pthread_mutex_t) are used for synchronization, but there is no concurrent modification of shared resources by threads that would necessitate a Mutex for synchronization. Additionally, the sorting and merging operations are structured to work on distinct data segments or are sequenced in a way (sorting first, followed by merging) that inherently avoids concurrent access issues.

3. `Static Global Variables` - In C, you can directly operate on global arrays. In Rust, in order to safely initialize a global mutable array, you must make it a lazy allocation. This creates a static reference to the array and postpones the initialization until the first time it is used, avoiding compile time issues.

4. `Memory Management` - Rust automatically manages memory for you, ensuring safety. In C, you must manually allocate and deallocate memory. 

--- 











