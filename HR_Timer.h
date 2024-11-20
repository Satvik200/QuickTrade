//Preprocessor directive to ensure that the contents of the header file are only included once 
//Defines the JHTIMER macro, which prevents the file from being included multiple times
#ifndef __JHRTIMER__
#define __JHRTIMER__

//Standard C++ Libraries
#include <stdint.h>
#include <unistd.h>

//Include defined header file
#include "Utilities.h"

namespace QuickTrade {
    
    class HRTimer {
        private:
            //Function to read CPU timestamp counter
            //Uses inline assembly with rdtsc instruction
            inline uint64_t RDTSC() {
                #if defined(__i386__) || defined(__x86_64__)
                    uint32_t lo, hi;                                        //Reads 64-bit timestamp into two 32-bit registers
                    __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));    //asm volatile ensures the compiler doesn't optimize away/reorder this operation
                    return ((uint64_t)hi << 32) | lo;
                #elif defined(__aarch64__)  // ARM 64-bit
                    uint64_t virtual_timer_value;
                    asm volatile("mrs %0, cntvct_el0" : "=r"(virtual_timer_value));
                    return virtual_timer_value;
                #else
                    // Fallback: Portable timing method
                    return std::chrono::high_resolution_clock::now().time_since_epoch().count();
                #endif
            }

            uint64_t start_;    //Stores start timestamp
            uint64_t hz_;       //Stores CPU ticks per second from calibration

        public: 
            //Constructor to initialize start_ to 0 using initialization list
            HRTimer()
            : start_(0) {
                int64_t start = RDTSC();            //Gets initial CPU timestamp
                usleep(250000);                     //Sleeps for 2,50,000 microseconds (0.25s)
                int64_t end = RDTSC();              //Gets ending CPU timestamp

                int64_t duration = end - start;     //Find duration in ticks for 0.25s
                uint32_t ticks_sec = duration * 4;  //Calculate ticks per second

                fprintf(stderr, "Ticks/sec: %u.  MHz: %f\n", ticks_sec, ticks_sec / 1000000.0f);
                hz_ = ticks_sec;
            }

            //Start timer by storing current CPU timestamp (marked inline for performance)
            inline void start() {
                start_ = RDTSC();
            }

            //Stop timer and return elaped time
            inline uint64_t stop() {
                //UNLIKELY is an optimization hint from the prev file
                if (UNLIKELY(start_ == 0)) return 0;

                uint64_t nano = ((RDTSC() - start_) * 1000000000) / hz_;    //Gets duration in CPU ticks
                start_ = 0;                                                 //Convert to nanoseconds using hz_ calibration
                return nano;                                               
            }
    };
}    

#endif