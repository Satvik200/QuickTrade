//Preprocessor directive to ensure that the contents of the header file are only included once 
//Defines the JFHERRORTRACKER macro, which prevents the file from being included multiple times
#ifndef __JFHERRORTRACKER__
#define __JFHERRORTRACKER__

//Standard C++ Libraries
#include <stdint.h>
#include <stdio.h>

namespace QuickTrade {
    /*----------------------------------------------------------------------------------------------------*/
    // FILE HANDLING ERROR TRACKING CLASS
    /*----------------------------------------------------------------------------------------------------*/
    //This class is not thread-safe and instance() should be called in the main() before starting 
    //any thread to avoid the need for locking
    class FHErrorTracker {
        private:
            //Constructor
            FHErrorTracker() {}
            //Static pointer to the singleton instance of the class
            static FHErrorTracker *instance_;

            //Logical error counters
            uint32_t duplicate_add_ = 0;
            uint32_t trade_missing_orders_ = 0;
            uint32_t bad_cancels_ = 0;
            uint32_t bad_modifies_ = 0;
            uint32_t crossed_book_ = 0;

            //Parsing error counters
            uint32_t corrupt_messages_ = 0;
            uint32_t invalid_qtys_ = 0;
            uint32_t invalid_prices_ = 0;
            uint32_t invalid_ids_ = 0;
            uint32_t good_messages_ = 0;

        public:
            //Returns a singleton instance of the class
            //If 0, create a new instance and return it
            static FHErrorTracker *instance() {
                if (instance_ == 0) {
                    instance_ = new FHErrorTracker();
                }
                return instance_;
            }

            void init(){}

            //Member funcs of the class which are used to update error counters
            //Simply increment the member variables.
            void duplicateAdd() { ++duplicate_add_; }
            void tradeMissingOrders() { ++ trade_missing_orders_; }
            void badCancel() { ++bad_cancels_; }
            void crossedBook() { ++crossed_book_; }
            void corruptMessage() { ++corrupt_messages_; }
            void invalidQuantity() { ++invalid_qtys_; }
            void invalidPrice() { ++invalid_prices_; }
            void invalidID() { ++invalid_ids_; }
            void invalidModify() { ++bad_modifies_; }
            void goodMessage() { ++good_messages_; }

            //Print the current values of all error counters to the stderr stream
            void printStatistics() {
                fprintf(stderr, "\n[Feed Handler Statistics]\n");
                fprintf(stderr, "   %-30s %10u\n", "Corrupt Messages", corrupt_messages_);
                fprintf(stderr, "   %-30s %10u\n", "Good Messages:", good_messages_);
                fprintf(stderr, "   %-30s %10u\n", "Duplicate Adds:", duplicate_add_);
                fprintf(stderr, "   %-30s %10u\n", "Trades Missing Orders:", trade_missing_orders_);
                fprintf(stderr, "   %-30s %10u\n", "Cancels for Missing ID's:", bad_cancels_);
                fprintf(stderr, "   %-30s %10u\n", "Modifies for Missing ID's:", bad_modifies_);
                fprintf(stderr, "   %-30s %10u\n", "Crossed Book:", crossed_book_);
                fprintf(stderr, "   %-30s %10u\n", "Invalid Quantities:", invalid_qtys_);
                fprintf(stderr, "   %-30s %10u\n", "Invalid Prices:", invalid_prices_);
                fprintf(stderr, "   %-30s %10u\n", "Invalid IDs:", invalid_ids_);
            }

            //Destructor to delete the singleton instance of the class
            ~FHErrorTracker() {
                delete instance_;
            }
    };
}

#endif