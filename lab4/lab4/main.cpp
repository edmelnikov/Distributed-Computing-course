#include <iostream>
#include <math.h>
#include <vector>
#include "mpi.h"
#include <cstring>


const int arr_size = 100000;
int root = 0;
double subarr_frac = 0.75; // fraction of a subarray sent by a process

/* Swaps two values */
void swap(int *num1, int *num2) {
	int buf = *num2;
	*num2 = *num1;
	*num1 = buf;
}

/* Sorts an array and returns number of swaps*/
int bubble_sort(int *arr, int size) {
	int num_swaps = 0;
	for (int i = 0; i < size - 1; i++) {
		for (int j = i + 1; j < size; j++) {
			if (arr[i] > arr[j]) {
				swap(&(arr[i]), &(arr[j]));
				num_swaps += 1;
			}
		}
	}
	return num_swaps;
}

int main(int argc, char** argv) {
	
	MPI_Init(&argc, &argv);
	int size, rank;
	
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	int subarr_size = arr_size / size; // size of subarrays on each process

	int *arr = new int[arr_size]; // array that has to be sorted
	int* subarr = new int[subarr_size]; // part of an array that is stored in a process

	/* fill up the array on the root process */
	if (rank == root) {
		for (int i = 0; i < arr_size; i++) {
			arr[i] = arr_size - i;
		}
	}

	/* Scatter the array to all processes */
	MPI_Scatter(arr, subarr_size, MPI_INT, subarr, subarr_size, MPI_INT, root, MPI_COMM_WORLD);

	/* The odd-even transposition parallel bubble sort algorithm */	
	int subarr_chunk_size = subarr_size * subarr_frac; // size of a chunk of a subarray that is sent between processes 
	int coupled_arr_size = subarr_size + subarr_chunk_size; // size of a coupled array (processor's subarray + received chunk of the adjacent processor)
	int* coupled_arr = new int[coupled_arr_size]; // array constructed from current process subarray and sender chunk of its subarray
	// the array is sorted on recipient process and the LAST "subarr_chunk_size" number of ints is returned to the sender

	int num_swaps = 0; // number of swaps on a process
	int global_num_swaps = 0; // number of swaps across all processes
	int iter_no_swap = 0; // number of iterations without swaps
	bool stop_flag = false;

	double start_time = MPI_Wtime();
	
	for (int iter_ind = 0;; iter_ind++) {
		num_swaps = 0;
		if (iter_ind % 2 == 0) { // iteration index is even
			if (rank % 2 == 0 && rank + 1 < size) { // receive the data on even process and sort it 
				std::memcpy(coupled_arr, subarr, subarr_size * sizeof(int)); // copy subarray of current process to coupled_arr
				MPI_Recv(&(coupled_arr[subarr_size]), subarr_chunk_size, MPI_INT, rank + 1, 0,
					MPI_COMM_WORLD, MPI_STATUS_IGNORE); // receive a chunk of a subarray from an odd process on the right and APPEND it to coupled_arr
				num_swaps += bubble_sort(coupled_arr, coupled_arr_size); // sort the coupled_arr
				std::memcpy(subarr, coupled_arr, subarr_size * sizeof(int)); // copy the subarray part from coupled_arr
				MPI_Send(&(coupled_arr[subarr_size]), subarr_chunk_size, MPI_INT, rank + 1, 1, MPI_COMM_WORLD); // send back the sorted chunk to the sender
			}
			else if (rank % 2 != 0 && rank - 1 >= 0) { // send the data on/from odd process to even process on the left
				MPI_Send(subarr, subarr_chunk_size, MPI_INT, rank - 1, 0, MPI_COMM_WORLD); // send a chunk of a subarray to even process on the left
				MPI_Recv(subarr, subarr_chunk_size, MPI_INT, rank - 1, 1,
					MPI_COMM_WORLD, MPI_STATUS_IGNORE); // receive the sorted chunk from the recipient and write it sender subarray				
			}
		}
	
		else if (iter_ind % 2 != 0) { // iteration index is odd
			if (rank % 2 != 0 && rank + 1 < size) { // receive the data on odd process and sort it 
				std::memcpy(coupled_arr, subarr, subarr_size * sizeof(int)); // copy subarray of current process to coupled_arr
				MPI_Recv(&(coupled_arr[subarr_size]), subarr_chunk_size, MPI_INT, rank + 1, 2,
					MPI_COMM_WORLD, MPI_STATUS_IGNORE); // receive a chunk of a subarray from an even process on the right and APPEND it to coupled_arr
				num_swaps += bubble_sort(coupled_arr, coupled_arr_size); // sort the coupled_arr
				std::memcpy(subarr, coupled_arr, subarr_size * sizeof(int)); // copy the subarray part from coupled_arr
				MPI_Send(&(coupled_arr[subarr_size]), subarr_chunk_size, MPI_INT, rank + 1, 3, MPI_COMM_WORLD); // send back the sorted chunk to the sender
			}
			else if (rank + 1 >= size) {// last processor sort (the last subarr*subarr_frac items of the last processor
					// are left untouched, that's why whe forcibly sort its subarray (the last processor doesn't receive data from any other processor)). 
				num_swaps += bubble_sort(subarr, subarr_size);
			}
			else if (rank % 2 == 0 && rank - 1 > 0) { // send the data on/from even process to odd process on the left
				MPI_Send(subarr, subarr_chunk_size, MPI_INT, rank - 1, 2, MPI_COMM_WORLD); // send a chunk of a subarray to odd process on the left
				MPI_Recv(subarr, subarr_chunk_size, MPI_INT, rank - 1, 3,
					MPI_COMM_WORLD, MPI_STATUS_IGNORE); // receive the sorted chunk from the recipient and write it sender subarray
			}
		}
	
		MPI_Reduce(&num_swaps, &global_num_swaps, 1, MPI_INT, MPI_SUM, root, MPI_COMM_WORLD); // collect numbers of swaps on current iteration from all processes
		if (iter_ind % 2 == 0 && rank == root && iter_ind > 0) { // we check every two iterations after first 2 iterations whether there have been swaps 
			if (global_num_swaps == 0) {
				stop_flag = true;
			}
		}
		MPI_Bcast(&stop_flag, 1, MPI_C_BOOL, root, MPI_COMM_WORLD); // broadcast the flag across all processes
		if (stop_flag) {
			break;
		}
	}
	double total_time = MPI_Wtime() - start_time;

	MPI_Gather(subarr, subarr_size, MPI_INT, arr, subarr_size, MPI_INT, root, MPI_COMM_WORLD);
		
	if (rank == 0) {
		//for (int i = 0; i < arr_size; i++) {
		//	std::cout << arr[i] << ", ";
		//}
		std::cout << std::endl;
		std::cout << "time: " << total_time << std::endl;
		//std::cout << "Number of iterations: " << << std::endl;
	}

	MPI_Finalize();

	return 0;
}