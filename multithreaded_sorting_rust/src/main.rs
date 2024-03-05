use std::thread;

static ARR: [i32; 14] = [16, 26, 53, 44, 65, 36, 77, 89, 91, 106, 51, 62, 123, 69];


fn main() {
    // Split the array into 2 slices at middle index
    let mid = ARR.len()/2;
    let (first_half, second_half) = ARR.split_at(mid);

    let sorting_thread1 = thread::spawn(move || { merge_sort(first_half.to_vec()) });
    let sorting_thread2 = thread::spawn(move || { merge_sort(second_half.to_vec()) });

    let sorted_first_half = sorting_thread1.join().unwrap();
    let sorted_second_half = sorting_thread2.join().unwrap();

    let merging_thread = thread::spawn(move || { merge(sorted_first_half, sorted_second_half) });

    let final_sorted_array = merging_thread.join().unwrap();
    println!("Final sorted array: {:?}", final_sorted_array);
}

fn merge_sort<T: PartialOrd + Copy>(data: Vec<T>) -> Vec<T> {
    // Base case
    if data.len() <= 1 { return data; }

    let middle = data.len()/2;

    // Split the vector in half recursively until there is only one element
    let left = merge_sort(data[..middle].to_vec());
    let right = merge_sort(data[middle..].to_vec());

    // Merge and sort the vector elements
    merge(left, right)
}

fn merge<T: PartialOrd + Copy>(left: Vec<T>, right: Vec<T>) -> Vec<T> {
    // Instantiate sorted vector we will return
    let mut result = Vec::with_capacity(left.len() + right.len());

    // 2 pointers to compare elements in each vector
    // i - left vector
    // j - right vector
    let (mut i, mut j) = (0, 0);
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
    if i < left.len() { result.extend_from_slice(&left[i..]); }
    if j < right.len() {
        result.extend_from_slice(&right[j..]);
    }

    // return sorted vector
    result
}
