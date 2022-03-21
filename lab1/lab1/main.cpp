#include <iostream>
#include <stdlib.h> // rand 
#include "omp.h"

using namespace std;

const int N = 12000; // matrix and vector dimensions

/* Scalar product of two vectors */
double vec_mul (double* vec1, double* vec2) {
	double product = 0;
	for (int j = 0; j < N; j++) {
		product += vec1[j] * vec2[j];
	}
	return product;
}

/* The following 3 functions return their running times and return the product to vec_res (resulting vector) passed as a pointer*/
/* Sequential matrix-vector product */
double mat_vec_mul_seq(double(*mat)[N], double* vec, double* vec_res) {
	double start_time = omp_get_wtime();
	for (int i = 0; i < N; i++) {
		vec_res[i] = vec_mul(mat[i], vec);
	}
	return omp_get_wtime() - start_time;
}

/* Matrix-vector product using "parallel for" */
double mat_vec_mul_par_for(double(*mat)[N], double* vec, double* vec_res, int num_threads) {
	omp_set_num_threads(num_threads);	
	double start_time = omp_get_wtime();
	# pragma omp parallel for
	for (int i = 0; i < N; i++) {
		vec_res[i] = vec_mul(mat[i], vec);
	}
	return omp_get_wtime() - start_time;
}

/* Matrix-vector product using manual omp parallelism */
double mat_vec_mul_par_manual(double(*mat)[N], double* vec, double* vec_res, int num_threads) {
	omp_set_num_threads(num_threads);
	int n = N / omp_get_max_threads(); // number of vector multiplication operations each thread has to perform
	double start_time = omp_get_wtime();
	# pragma omp parallel 
	{
		int curr_thread = omp_get_thread_num(); // current thread number		
		for (int i = curr_thread * n; i < curr_thread*n + n; i++) {
			vec_res[i] = vec_mul(mat[i], vec);
		}	
	}	
	return omp_get_wtime() - start_time;
}

int main() {
	const int max_threads = omp_get_num_threads();
	cout << "The total amount of available threads is " << max_threads << endl;

	if (N % max_threads == 0) {

		/* Create and fill up the vector and the matrix*/
		auto mat = new double[N][N]; // NxN matrix
		auto vec = new double[N];
		auto vec_res = new double[N]; // the resulting vector after the multiplication

		srand(0); // initialize the seed
		# pragma omp parallel for
		for (int i = 0; i < N; i++) {
			vec[i] = rand() % 100; // fill up the vector
			for (int j = 0; j < N; j++) {
				mat[i][j] = rand() % 100; // fill up the matrix
			}
		}
		
		/* Run the experiments */
		const int num_experiments = 10;
		const int num_threads = 4;
		double seq_time = 0; 
		double par_for_time[num_threads]; // mean running time where index is the number of threads
		double par_manual_time[num_threads];
		
		for (int i = 0; i < num_threads; i++) {
			par_for_time[i] = 0;
			par_manual_time[i] = 0;
		}

		/* first, collect total running times*/
		for (int i = 0; i < num_experiments; i++) { // iterate over experiments
			seq_time += mat_vec_mul_seq(mat, vec, vec_res);
			for (int thread = 1; thread <= num_threads; thread++) { // iterate over different numbers of workers
				par_for_time[thread - 1] += mat_vec_mul_par_for(mat, vec, vec_res, thread);
				par_manual_time[thread - 1] += mat_vec_mul_par_manual(mat, vec, vec_res, thread);
			}
		}

		/* then calculate mean running times across the experiments*/
		seq_time = seq_time / num_experiments;
		cout << "Mean sequential time: " << seq_time << endl;

		/* Calculate speedup and efficiency and print the values*/
		double speedup;
		double efficiency;

		for (int i = 0; i < num_threads; i++){
			par_for_time[i] = par_for_time[i] / num_experiments;
			par_manual_time[i] = par_manual_time[i] / num_experiments;
			
			cout << "--- Number of threads: " << i+1 << " ---" << endl;
			cout << "'Parallel for' metrics: " << endl;
			cout << "Mean time: " << par_for_time[i] << " Speedup: " << seq_time / par_for_time[i];
			cout << " Efficiency: " << (seq_time / par_for_time[i]) / (i + 1) << endl;

			cout << "Manual parallelism metrics: " << endl;
			cout << "Mean time: " << par_manual_time[i] << " Speedup: " << seq_time / par_manual_time[i];
			cout << " Efficiency: " << (seq_time / par_manual_time[i]) / (i + 1) << endl;
		}

		/* clean up the memory */
		delete[] mat;
		delete[] vec;
		delete[] vec_res;
	}
	else {
		cout << "The dimensions of a matrix and a vector N = " << N << " are not divisible by the number of avaliable threads  ";
		cout << max_threads << endl;
	}

	return 0;
}