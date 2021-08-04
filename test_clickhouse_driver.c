#include "use_clickhouse_driver.h"
#include <stdio.h>

#define MEASURE_TIME
#ifdef MEASURE_TIME
#include <sys/time.h>   // for gettimeofday()
#include <time.h>
#endif

#ifdef MEASURE_TIME
struct timespec diff(struct timespec start, struct timespec end);
#endif

/*
 * Notes: clickhouse-cpp is not thread safe yet, therefore I will not try to use a threads here
 */

void insertFakeData()
{
	struct timespec start, end;
	CHconstruct();
	int times;
	for(times = 0; times < 5; times ++)
	{
#ifdef MEASURE_TIME
		clock_gettime(CLOCK_MONOTONIC, &start); // using a monotonic clock prevents surprises introduced by clocks changing during measurements (such as NTP)
#endif
		int i;
		struct packet p;
		for(i = 0; i < 12000000; i++)
		{
			p.pkt_length = rand();
			CHappend(&p);
		}
		CHinsert();
#ifdef MEASURE_TIME
		clock_gettime(CLOCK_MONOTONIC, &end);
		struct timespec timediff = diff(start, end);
		printf("Time diff: %lu sec %lu nsec \n", timediff.tv_sec, timediff.tv_nsec);
#endif
		//CHshow();
	}
	CHdestruct();
}

int main()
{
	srand(time(NULL));   // Initialization, should only be called once.
	insertFakeData();
}

#ifdef MEASURE_TIME
struct timespec diff(struct timespec start, struct timespec end)
{
	struct timespec temp;
	if((end.tv_nsec - start.tv_nsec) < 0)
	{
		temp.tv_sec = end.tv_sec - start.tv_sec - 1;
		temp.tv_nsec = 1000000000 + end.tv_nsec - start.tv_nsec;
	}
	else
	{
		temp.tv_sec = end.tv_sec - start.tv_sec;
		temp.tv_nsec = end.tv_nsec - start.tv_nsec;
	}
	return temp;
}
#endif
