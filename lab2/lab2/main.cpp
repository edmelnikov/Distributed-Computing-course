#include <iostream>
#include <fstream>
#include <stdlib.h> // rand 

#include "mpi.h"

#define N 100000000 // array size


/* Calculate sequentially sum of an array (subarray) with a specified length */
double arr_sum_seq(double* arr, int len) {
	double sum = 0;
	for (int i = 0; i < len; i++) {
		sum += arr[i];
	}
	return sum;
}

int main(int argc, char** argv) {
	std::ofstream file;

	int size, rank;
	int root = 0; // root processor's index
	
	MPI_Init(&argc, &argv); // initialize
	MPI_Comm_size(MPI_COMM_WORLD, &size); // number of avaliable processors		
	MPI_Comm_rank(MPI_COMM_WORLD, &rank); // id of a current processor 

	double* arr = new double[N]; // array with all integers
	int n = N / size; // (N/p) size of a subarray that a process has to work on	
	double* sub_arr = new double[n]; // subarray that a process has to work on 

	if (rank == root) { // work performed by root processor
		/* Create and fill up the array*/
		srand(0); // initialize the seed
		for (int i = 0; i < N; i++) {
			//arr[i] = rand() % 100; 
			arr[i] = i;
		}
	}
	
	double start_time = MPI_Wtime();
	MPI_Scatter(arr, n, MPI_DOUBLE, sub_arr, n, MPI_DOUBLE, root, MPI_COMM_WORLD); // distribute the subarrays across processors

	double sub_sum = arr_sum_seq(sub_arr, n); // sum of a subarray of a processor
	double* sub_sums = new double[size]; // array with all sums of subarrays, calculated by other processors 
	
	MPI_Gather(&sub_sum, 1, MPI_DOUBLE, sub_sums, 1, MPI_DOUBLE, root, MPI_COMM_WORLD); // gather all the subarray sums from all processors to a root processor

	double total_sum = 0;
	if (rank == 0) {
		total_sum = arr_sum_seq(sub_sums, size);
		double end_time = MPI_Wtime() - start_time;	
		std::cout << "Size: " << size << " Time: " << end_time << " Sum: " << total_sum << std::endl;
		file.open("out.txt", std::ios::app);
		if (file.is_open()) {
			file << "Size: " << size << " Time: " << end_time << " Sum: " << total_sum << std::endl;
		}
		file.close();
	}

	/* Clean up the memory */
	delete[] arr;
	MPI_Finalize();


	//int size, rank;
	//MPI_Init(&argc, &argv); // initialize
	//MPI_Comm_size(MPI_COMM_WORLD, &size); // number of avaliable processors		
	//int fragment_size = N / size; // (N/p) size of a subarray that a process has to work on	
	//MPI_Comm_rank(MPI_COMM_WORLD, &rank); // id of a current processor 

	//double recv_arr_sum = 0;
	//double* arr;
	//double* recv_arr;
	//if (rank == 0) { // work performed by 0th processor
	//	
	//	/* Create and fill up the array*/
	//	arr = new double[N];
	//	srand(0); // initialize the seed
	//	for (int i = 0; i < N; i++) {
	//		//arr[i] = rand() % 100; 
	//		arr[i] = i;
	//	}

	//	/* Mark the start time */
	//	double start_time = MPI_Wtime();
	//	
	//	/* Send subarrays of size fragment_size to every avaliable processor */
	//	for (int p = 1; p < size; p++) {
	//		MPI_Send(&(arr[p * fragment_size]), fragment_size, MPI_DOUBLE, p, 1337, MPI_COMM_WORLD); // 1337 is a tag of a message
	//	}

	//	/* Calculate the sum on the zero processor and add it to total_sum */
	//	int total_sum = 0; 
	//	total_sum += arr_sum_seq(arr, fragment_size); // 0th processor's sum
	//			
	//	/* Receive the sums of subbarays calculated by other processors */
	//	double p_sum = 0; 
	//	for (int p = 1; p < size; p++) {
	//		MPI_Recv(&p_sum, 1, MPI_DOUBLE, p, 1338, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	//		total_sum += p_sum;
	//	}

	//	/* Mark total time */
	//	double total_time = MPI_Wtime() - start_time;
	//	std::cout << "Sum: " << total_sum << " Running time: " << total_time << std::endl;

	//	/* Clean up the memory */
	//	delete[] arr;
	//}

	//else { // work performed by all the other processors
	//	recv_arr = new double[fragment_size];
	//	MPI_Recv(recv_arr, fragment_size, MPI_DOUBLE, 0, 1337, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	//	recv_arr_sum = arr_sum_seq(recv_arr, fragment_size);
	//	delete[] recv_arr;
	//	MPI_Send(&recv_arr_sum, 1, MPI_DOUBLE, 0, 1338, MPI_COMM_WORLD);
	//}

	return 0;
}