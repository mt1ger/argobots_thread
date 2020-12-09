#include <iostream>
#include <cstdlib>
#include <stdio.h>
#include <unistd.h>
#include <chrono>
#include<thread>
// #include<ostream>

#include"thread.h"

using namespace std;

typedef struct {
    int n;
    int ret;
} fibonacci_arg_t;

void fibonacci(void *arg)
{
    int n = ((fibonacci_arg_t *)arg)->n;
    int *p_ret = &((fibonacci_arg_t *)arg)->ret;

    if (n <= 1) {
        *p_ret = 1;
    } 
	else {
        fibonacci_arg_t child1_arg = {n - 1, 0};
        fibonacci_arg_t child2_arg = {n - 2, 0};

		stdx::thread threads (fibonacci, &child1_arg);

        /* Calculate fib(n - 2).  We do not create another ULT. */
        fibonacci(&child2_arg);

		threads.join();

        *p_ret = child1_arg.ret + child2_arg.ret;
    }
}

int fibonacci_seq(int n)
{
    if (n <= 1) {
        return 1;
    } else {
        int i;
        int fib_i1 = 1; /* Value of fib(i - 1) */
        int fib_i2 = 1; /* Value of fib(i - 2) */
        for (i = 3; i <= n; i++) {
            int tmp = fib_i1;
            fib_i1 = fib_i1 + fib_i2;
            fib_i2 = tmp;
        }
        return fib_i1 + fib_i2;
    }
}


void as (void * args) 
{
	((fibonacci_arg_t*)args)->n *= 3;
}

double test (int a, double b)  
{
	return a + b;
}

template<class Fn, class ...Args>
struct t_wrapper_args_ 
{
	Fn func;
	std::tuple<Args...> tuple_;
	ABT_eventual* ev_ptr;
};

int main (int argc, char * argv[])
{

	int n = 13;

	/* Read arguments. */
	while (1) {
		int opt = getopt(argc, argv, "he:n:");
		if (opt == -1)
			break;
		switch (opt) {
			case 'e':
				// num_xstreams = atoi(optarg);
				// cout << num_xstreams << " are needed." << endl;
				break;
			case 'n':
				n = atoi(optarg);
				break;
			case 'h':
			default:
				printf("Usage: ./stdThread [-e NUM_XSTREAMS]\n");
				return -1;
		}
	}

	chrono::steady_clock::time_point start = chrono::steady_clock::now();

	fibonacci_arg_t arg = {n, 0};
    // fibonacci(&arg);
	stdx::thread t1 (as, &arg);
	t1.join ();
    int ret = arg.ret;
	int n__ = arg.n;
    int ans = fibonacci_seq(n);
	cout << "The n is " << n__ << endl;
	cout << "The returned value is " << ret << "; The verification is " << ans << endl;

	chrono::steady_clock::time_point end = chrono::steady_clock::now();
	chrono::duration<double> time_span = chrono::duration_cast<chrono::duration<double> >(end-start);
	cout << "Execution time: " << time_span.count() << endl;


        // fibonacci_arg_t child2_arg = {10, 0};
		// ABT_eventual ev;	
		// t_wrapper_args_<decltype(&fibonacci), decltype(&child2_arg)> twa;
		// twa.func = fibonacci;
		// twa.tuple_ = std::make_tuple (&child2_arg);
		// twa.ev_ptr = &ev;
		// stdx::thread_d threads (fibonacci, &child2_arg);
		// stdx::t_wrapper<decltype(&fibonacci), decltype(&child2_arg)> (&twa);
		// threads.wait();
		// cout << child2_arg.ret << endl;
		
		// void (*func1)(void *);
		// void (*func2)(void *);
		// func2 = &fibonacci;
		// cout << is_same<decltype(func1), decltype(&fibonacci)>::value << endl;
		// cout << is_same<decltype(func1), decltype(func2)>::value << endl;
		// fibonacci(&child2_arg);
		// cout << child2_arg.ret  << endl;
        // child2_arg = {10, 0};
		// func2(&child2_arg);
		// cout << child2_arg.ret  << endl;

	return 1;
}

