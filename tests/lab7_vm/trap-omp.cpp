/**********************************************************************
* Program:
*    Lab OpenMP
*    Brother Jones, CS 345
* Author:
*    Bryson Gibbons
* Summary:
*    Learn how to use OpenMP to run a program in parallel processing
*    environments and take advantage of parallel processing
* Conclusions:
*    -The basic tests all performed as described, and I noticed massive
*    variation in the answers provided by running the program with
*    2 threads after inserting the instruction to run in parallel.
*    -The race condition was on the variable insert, and that had to be
*    managed in some way which could include semaphores or some way of
*    summing up each thread separately, and then combining them. This
*    was the method provided by the instructions.
*    -The reason why the program completed in less time with 4 threads
*    than with 1 is because we have minimized the time it takes to
*    perform the operation through parallelization, while we have not
*    made it so highly parallel that the overhead of being parallel
*    starts to increase runtime. With threadcounts above 4, I did not
*    see any consistency on the changes in timing compared to a thread
*    count of 4, but 6 threads was usually as fast or faster than
*    4 threads, and the time flatlines with threadcounts above 16.
***********************************************************************/

#include <iostream>
#include <cmath>
#include <cstdlib>
using namespace std;

/* Demo program for OpenMP: computes trapezoidal approximation to an integral*/

const double pi = 3.141592653589793238462643383079;

int main(int argc, char** argv) {
   /* Variables */
   double a = 0.0;           /* limits of integration */
   double b = pi; 
   int n = 4194304;          /* number of subdivisions = 2^22 */
   double h = (b - a) / n;   /* width of subdivision */
   double integral;          /* accumulates answer */
   int threadct = 1;         /* number of threads to use */
   
   /* parse command-line arg for number of threads */
   if (argc > 1)
     threadct = atoi(argv[1]);
   
   double f(double x);
   
#ifdef _OPENMP
   cout << "OMP defined, threadct = " << threadct << endl;
#else
   cout << "OMP not defined" << endl;
#endif
   
   integral = (f(a) + f(b))/2.0;
   int i;
   
   #pragma omp parallel for num_threads(threadct) \
      shared (a, n, h) reduction(+: integral) private(i)
   for(i = 1; i < n; i++) {
     integral += f(a+i*h);
   }
   
   integral = integral * h;
   cout << "With n = " << n << " trapezoids, our estimate of the integral" <<
     " from " << a << " to " << b << " is " << integral << endl;
}

double f(double x) {
   return sin(x);
}

