#ifndef PELICANTIMER_H_
#define PELICANTIMER_H_

#include <ctime>
#include <cstdio>

/**
 * @brief
 * Preprocessor macros used for timing.
 *
 * @details
 * This header defines two macros to measure the time interval
 * taken between their calls.
 *
 * Define TIMER_ENABLE to use them.
 *
 * Call TIMER_START to start the timer, and TIMER_STOP(...) to print the time
 * taken (with a message prefix) to standard output (STDOUT).
 */
#ifdef TIMER_ENABLE
#define TIMER_START {clock_t _pelican_start_time = clock();
#define TIMER_STOP(...) clock_t _pelican_timer_interval = clock() - _pelican_start_time; \
    float _pelican_time_sec = (float)(_pelican_timer_interval) / CLOCKS_PER_SEC; \
    fprintf(stdout, "\n"); \
    fprintf(stdout, __VA_ARGS__); \
    fprintf(stdout, ": %.2f sec.\n", _pelican_time_sec);}
#else
#define TIMER_START ;
#define TIMER_STOP(...) ;
#endif

#endif /* PELICANTIMER_H_ */
