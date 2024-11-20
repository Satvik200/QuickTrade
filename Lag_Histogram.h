//Preprocessor directive to ensure that the contents of the header file are only included once 
//Defines the LAGHISTOGRAM macro, which prevents the file from being included multiple times
#ifndef __LAGHISTOGRAM__
#define __LAGHISTOGRAM__

//Standard C++ Libraries
#include <limits.h>
#include <stdint.h>
#include <string>
#include <algorithm>
#include <vector>

//Class Definition
class LagHistogram {
    private:
        std::string title_;                 //Histogram name
        std::vector<uint64_t> samples_;     //Vector for storing samples

    public:
        //Constructor Definition
        LagHistogram(std::string title, uint32_t sample_count = 0)
        : title_(title)
        , samples_()
        {
            if (sample_count > 0)                   //If sample_count provided, reserve capacity in the vector to avoid reallocation
                samples_.reserve(sample_count);
        }

        //Function to add a new measurement
        void add(uint64_t input) {
            samples_.push_back(input);
        }

        void print() {
            //check for empty samples
            if (samples_.size() == 0) {
                fprintf(stderr, "[%s] No valid samples for run.\n", title_.c_str());
                return;
            }

            //Sort samples in ascending order for percentile calculations
            std::sort(samples_.begin(), samples_.end());
            uint64_t sum = 0;

            fprintf(stderr, "\nPerformance results for [%s] (unit: milliseconds)\n", title_.c_str());

            //Calculate sum of all samples
            for(uint32_t i = 0; i < samples_.size(); ++i) {
                sum += samples_[i];
            }
            
            //Print basic statistics
            fprintf(stderr, "   %-10s %10lu\n", "Samples:", samples_.size());
            fprintf(stderr, "   %-10s %10llu\n", "Min:", samples_[0]);
            fprintf(stderr, "   %-10s %10llu\n", "Max:", samples_[samples_.size() - 1]);
            fprintf(stderr, "   %-10s %10llu\n", "Mean:", sum / samples_.size());
            fprintf(stderr, "   %-10s %10llu\n", "Median:", samples_[samples_.size() / 2]);

            //If samples>10, print common percentiles
            if (samples_.size() > 10) {
                fprintf(stderr, "\n   %-20s\n", "[Percentiles]");
                fprintf(stderr, "   %-10s %10llu\n", "10th:", samples_[(samples_.size() / 10)]);
                fprintf(stderr, "   %-10s %10llu\n", "20th:", samples_[(samples_.size() / 10.) * 2]);
                fprintf(stderr, "   %-10s %10llu\n", "50th:", samples_[(samples_.size() / 10.) * 5]);
                fprintf(stderr, "   %-10s %10llu\n", "70th:", samples_[(samples_.size() / 10.) * 7]);
                fprintf(stderr, "   %-10s %10llu\n", "90th:", samples_[(samples_.size() / 10.) * 9]);
            }

            //If samples>100, print high percentiles
            if (samples_.size() > 100) {
                fprintf(stderr, "   %-10s %10llu\n", "95th:", samples_[(samples_.size() / 100.) * 95]);
                fprintf(stderr, "   %-10s %10llu\n", "99th:", samples_[(samples_.size() / 100.) * 99]);
            }

            //If samples>10000, print very high percentiles
            if (samples_.size() >= 10000) {
                fprintf(stderr, "   %-10s %10llu\n", "99.99th:", samples_[(samples_.size() / 10000.) * 9999]);
            }
        }
};

#endif