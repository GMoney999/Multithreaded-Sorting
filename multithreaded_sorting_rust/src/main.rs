/// Overview
/// This program is a template for how to split up global data and perform a computation-heavy task
/// (like sorting) concurrently by spinning multiple threads.
/// Basically a Divide and Conquer while avoiding data races.

/// Problem
/// Because static mutable variables are inherently unsafe due to potential data races,
/// Rust does not directly allow mutable statics without an unsafe block.
/// Also, the size of mutable static variables must be known at compile time, but this cannot
/// be done with a vector since its size is dynamic.

/// Solution
/// 'Mutex' ensures that access to the array is synchronized, preventing data races in multithreaded contexts
/// 'lazy_static!' allows to define SORTED_ARR as a static reference to a Mutex-protected array, providing safe, global mutable access.
/// The array is initialized the first time it's accessed, avoiding the limitations of Rust's const-eval system for static initializers

extern crate lazy_static;
use std::thread;
use std::sync::{Mutex};
use lazy_static::lazy_static;

//// GLOBALS
// Immutable global array remains the same
static ARR: [i32; 14] = [16, 26, 53, 44, 65, 36, 77, 89, 91, 106, 51, 62, 123, 69];
// Mutable global empty array that will hold the sorted array
// Global variables are usually supposed to be initialized at compile time, but "lazy_static" lets
// you postpone the initialization until the first time it is used, avoiding this compile time issue
// while ensuring safe access
lazy_static! {
    static ref SORTED_ARR: Mutex<[i32; 14]> = Mutex::new([0; 14]);
}

// SORTING FUNCTIONS
// Generic "T" is used with traits "Partial Order" and Copy.
// This allows the sorting algorithm to work with integers (signed/unsigned) and floats

// Helper function to recursively split the array
fn merge_sort<T: PartialOrd + Copy>(data: Vec<T>) -> Vec<T> {
    // Base case
    if data.len() <= 1 {
        return data;
    }

    // Get the midpoint
    let middle = data.len()/2;

    // Split the vector in half recursively until there is only one element
    let left = merge_sort(data[..middle].to_vec());
    let right = merge_sort(data[middle..].to_vec());

    // Merge and sort the vector elements
    merge(left, right)
}

// Sorting algorithm to merge two vectors into a single sorted vector
fn merge<T: PartialOrd + Copy>(left: Vec<T>, right: Vec<T>) -> Vec<T> {
    // Instantiate sorted vector we will return
    let mut result = Vec::with_capacity(left.len() + right.len());

    // 2 pointers to compare elements in each vector
    // i - left vector
    // j - right vector
    let (mut i, mut j) = (0, 0);
    // Loop continues as long as there are elements in both vectors that need to be compared and merged
    while i < left.len() && j < right.len() {
        if left[i] <= right[j] {
            result.push(left[i]);
            i+=1;
        } else {
            result.push(right[j]);
            j+=1;
        }
    }

    // Add elements left over from other vector
    // We can assume the rest of the array is sorted
    if i < left.len() {
        result.extend_from_slice(&left[i..]);
    }
    if j < right.len() {
        result.extend_from_slice(&right[j..]);
    }

    // return sorted vector
    result
}




fn main() {
    // Split the array into 2 slices at middle index
    let mid = ARR.len()/2;
    let (first_half, second_half) = ARR.split_at(mid);

    let sorting_thread1 = thread::spawn(move || { merge_sort(first_half.to_vec()) });
    let sorting_thread2 = thread::spawn(move || { merge_sort(second_half.to_vec()) });

    let sorted_first_half = sorting_thread1.join().unwrap();
    let sorted_second_half = sorting_thread2.join().unwrap();

    // Merging thread modifies mutable static array
    let merging_thread = thread::spawn(move || {
        // Obtain a mutable reference to the static array protected by the mutex guard
        let mut sorted_arr = SORTED_ARR.lock().unwrap();
        // Get the full sorted array
        let merged = merge(sorted_first_half, sorted_second_half);
        // Copy sorted array into global array

        for (i, v) in merged.iter().enumerate() {
            sorted_arr[i] = *v;
        }
    });

    // Execute merging thread
    merging_thread.join().unwrap();

    // Since global state is locked in a Mutex, we must unlock it to access the data
    println!("Sorted array: {:?}", *SORTED_ARR.lock().unwrap());
}