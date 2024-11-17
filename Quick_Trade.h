//Preprocessor directive to ensure that the contents of the header file are only included once 
//Defines the JQUICKTRADE macro, which prevents the file from being included multiple times
#ifndef __JQUICKTRADE__
#define __JQUICKTRADE__

//Standard C++ Libraries
#include <assert.h>
#include <stdio.h>
#include <map>
#include <unordered_map>

//Include defined header file
#include "Counted_Order_List.h"
#include "FH_Error_Tracker.h"
#include "Logger.h"
#include "Message_Parser.h"
#include "Utilities.h"

namespace QuickTrade {
    template<typename ORDERIDTYPE, typename ORDERTYPE>
    //Class Definition
    class QuickTrade {
        private:
            Logger logger_;                             //Logger instance for logging msgs
            OrderListMap buy_book_map_;                 //Map of buy orders sorted by price
            OrderListMap sell_book_map_;                //Map of sell orders sorted by price
            OrderHash orders_;                          // Hash map of all active orders by ID
            unsigned long long recent_trade_price_;     //Most recent trade price
            uint32_t recent_trade_qty_;                 //Most recent trade quantity

        public: 
            //Constructor Definition
            OrderBook()
            : logger_()
            , recent_trade_price_(0)
            , recent_trade_qty_(0)
            {}

            //Destructor Definition
            ~OrderBook() {
                for (auto it = buy_book_map_.begin(); it != buy_book_map_.end(); ++it) {
                    it->second.clearLevel();
                }
                for (auto it = sell_book_map_.begin(); it != sell_book_map_.end(); ++it) {
                    it->second.clearLevel();
                }
                buy_book_map_.clear();
                sell_book_map_.clear();
                while (!orders_.empty()) {
                    auto it = orders_.begin();
                    delete it->second;
                    orders_.erase(it);
                }
            }

            //Typedefs for mapping order prices to order lists and order IDs to orders
            typedef std::map<unsigned long long, CountedOrderList<ORDERTYPE>> OrderListMap;
            typedef std::unordered_map<ORDERIDTYPE, ORDERTYPE*> OrderHash;

            //Function to return a reference to the logger_ member variable
            //Returning by reference allows external code to interact directly with the logger instance
            Logger& getLoggerReference() {
                return logger_;
            }

            //Function to find midpoint price between the lowest selling price & highest buy price
            void printMidpoint() {
                //Check if either sell or buy order book is empty
                if (sell_book_map_.empty() || buy_book_map_.empty()) {
                    logger_.print("NAN\n");
                    return;
                }

                //Find the min sell price
                unsigned long long sell_min = sell_book_map_.begin()->first;
                //Find the max buy price
                unsigned long long buy_max = buy_book_map_.rbegin()->first;

                // Calculate the midpoint price as the average
                double midpoint = (sell_min + buy_max) / 200.;
                // Format the midpoint value to 2 decimal places
                char buffer[20];
                sprintf(buffer, "%.2f\n", midpoint);
                // Log the formatted midpoint value
                logger_.print(buffer);
            }

            //Function to print the current state of the order book
            void printBook() {
                //Allocate buffer
                int max_buffer = 500;
                char *buffer = (char*)calloc(max_buffer, sizeof(char));
                int index = 0;

                //Print sell order levels in descending order of price
                for (auto it = sell_book_map_.rbegin(); it != sell_book_map_.rend(); ++it) {
                    if (it->second.getQuantity() != 0) {
                        if (index + 100 > max_buffer) growBuffer(buffer, max_buffer); //Expand buffer if needed
                        index += sprintf(&buffer[index], "%.2f ", it->first / 100.);
                        it->second.printLevel('S', buffer, index, max_buffer);
                        index += sprintf(&buffer[index], "\n");
                    }
                }

                //Print buy order levels in descending order of price
                safeCopyToBuffer(buffer, "\n", index, max_buffer);
                for (auto it = buy_book_map_.rbegin(); it != buy_book_map_.rend(); ++it) {
                    if (it->second.getQuantity() != 0) {
                        if (index + 100 > max_buffer) growBuffer(buffer, max_buffer);
                        index += sprintf(&buffer[index], "%.2f ", it->first / 100.);
                        it->second.printLevel('B', buffer, index, max_buffer);
                        index += sprintf(&buffer[index], "\n");
                    }
                }

                index += sprintf(&buffer[index], "\n");

                //Log the order book state and free the buffer
                logger_.print(buffer);
                free(buffer);
            }

            //Function to check if the buy and sell orders overlap in price, indicating a crossed book
            void checkCross() const {
                if (sell_book_map_.empty() || buy_book_map_.empty()) {
                    return;
                }
                auto sit = sell_book_map_.begin();
                auto bit = buy_book_map_.end();
                --bit;
                if (sit->first <= bit->first) {
                    FHErrorTracker::instance()->crossedBook();
                }
            }

            //Function to add a new order to the appropriate side of the order book
            void addOrder(ORDERTYPE* ole) {
                checkCross();

                auto ooit = orders_.find(ole->order_id_);
                if (ooit != orders_.end()) {
                    FHErrorTracker::instance()->duplicateAdd();
                    delete ole;
                    return;
                }

                OrderListMap &map = ole->order_side_ == eS_Buy ? buy_book_map_ : sell_book_map_;
                auto ret = map.insert({ole->order_price_, CountedOrderList<ORDERTYPE>()});
                ret.first->second.addNode(ole);
                orders_[ole->order_id_] = ole; //Pointer to the new order
            }
    };
}

#endif