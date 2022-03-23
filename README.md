# Distributed-Computing-course

This repository contains assignments for Distributed Computing course. 

## Assignment 1. Itroduction to OpenMP

Task 1. Implement matrix-vector multiplication using:

```C 
#pragma omp parallel for
```

Task 2. Implement matrix-vector multiplication manually distributing the data across processes with:  

```C
#pragma omp parallel 
```

The implementatins running times have to be measured on different numbers of avaliable threads (Lab1_metrics.xlsx)

## Assignment 2. Introduction to MPI

Task: Given an array of N items, calculate the sum of the array distributing the array across processes. Each process calculates its subarray and returns the sum to the root process.
Measure the time on different numbers of threads (Lab2_metrics.xlsx)

## Assignment 3. Matrix multiplication using MPI (Fox's algorithm)

Skipped

## Assignment 4. Parallel bubble sort using MPI 

Task: Implement odd-even parallel bubble sort using MPI and measure running times for different sizes of subarrays exchanged between processes (Lab4_metrics.xlsx)
