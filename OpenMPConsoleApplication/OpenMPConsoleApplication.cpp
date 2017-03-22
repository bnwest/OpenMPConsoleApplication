//
// OpenMPConsoleApplication.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

// Enable OpenMP in Visual Studio:
// 1. Projects > OpenMPConsoleApplication Properties ...
// 2. OpenMPConsoleApplication Property Pages modal dbox is thrown
// 3. in left pane select C/C++ > Language
// 4. Set Open MP Support to true (which appears to add the /openmp compiler switch for building)


#include <omp.h>  
//#include <stdio.h>  
//#include <stdlib.h>  
//#include <windows.h>  

// hello world program of OpenMP
// calculate the integral 0 to 1 of 4.0 / (1 + X**2) = pi

static const long num_steps = 1000000;
static int NUM_THREADS;

void calculatePi_v1()
{
	double step;
	double pi = 0.0;
	double sum = 0.0;

	step = 1.0 / (double) num_steps;

	for ( int i = 0; i<num_steps; i++ )
	{
		double x = (i + 0.5) * step;
		sum = sum + 4.0 / (1.0 + x*x);
	}

	pi = step * sum;

	printf( "pi=%0.10f\n", pi );
}

// single program multiple data pattern

void calculatePi_v2()
{
	double step;

	double pi = 0.0;
	double* sum = new double[NUM_THREADS];
	int numThreads = 0;

	step = 1.0 / (double) num_steps;
	#pragma omp parallel
	// creates a team of threads, each thread will execute the code block below
	{
		int threadId = omp_get_thread_num(); // 0 ... n-1
		int privateNumThreads = omp_get_num_threads();
		
		#pragma opm single
		{
			numThreads = privateNumThreads;
		}

		sum[threadId] = 0.0;
		for ( int i = threadId; i < num_steps; i += privateNumThreads )
		{
			double x = (i + 0.5) * step;
			sum[threadId] += (4.0 / (1.0 + x*x)); // false sharing, each thread keeps updating the same cahce line
		}
	}

	for ( int i = 0; i < numThreads; i++ )
	{
		pi += step * sum[i];
	}

	printf( "pi=%0.10f\n", pi );
}

void calculatePi_v3()
{
	double step;

	double pi = 0.0;
	double sum;

	step = 1.0 / (double) num_steps;
	sum = 0.0;

	#pragma omp parallel
	// creates a team of threads, each thread will execute the code block below
	{
		double privateSum;
		int threadId = omp_get_thread_num(); // 0 ... n-1
		int privateNumThreads = omp_get_num_threads();

		privateSum = 0.0;
		for ( int i = threadId; i < num_steps; i += privateNumThreads )
		{
			double x = (i + 0.5) * step;
			privateSum += (4.0 / (1.0 + x*x)); // no false sharing
		}

		#pragma omp critical
		// or #pragma omp atomic
		// creates a critical section
		{
			sum += privateSum;
		}
	}

	pi = step * sum;

	printf( "pi=%0.10f\n", pi );
}

void calculatePi_v4()
{
	double pi = 0.0;
	double sum = 0.0;

	double step = 1.0 / (double) num_steps;

	#pragma omp parallel for reduction(+:sum)
	for ( int i = 0; i < num_steps; i++ )
	{
		double x = (i + 0.5) * step;
		sum = sum + 4.0 / (1.0 + x*x);
	}

	pi = step * sum;

	printf( "pi=%0.10f\n", pi );
}

int main()
{
	NUM_THREADS = omp_get_num_procs();
	omp_set_num_threads( omp_get_num_procs() );  // one thread per each processor

	calculatePi_v1();

	calculatePi_v2();

	calculatePi_v3();

	calculatePi_v4();

	return 0;
}
