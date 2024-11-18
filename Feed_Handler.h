//Preprocessor directive to ensure that the contents of the header file are only included once 
//Defines the JFEEDHANDLER macro, which prevents the file from being included multiple times
#ifndef __JFEEDHANDLER__
#define __JFEEDHANDLER__

//Standard C++ Libraries
#include <string>
#include <vector>

//Include defined header file
#include "HR_Timer.h"
#include "Lag_Histogram.h"
#include "Quick_Trade.h"
#include "Message_Parser.h"

//If PROFILE is defined, macros for starting and stopping a timer are active, else these do nothing
#ifdef PROFILE
   #define STARTPROFILE() { timer_.start(); }
   #define STOPPROFILE(y) { y.add(timer_.stop()); }
#else
   #define STARTPROFILE()
   #define STOPPROFILE(y)
#endif

namespace QuickTrade {
    template<typename ORDERIDTYPE, typename ORDERTYPE>
    //Class Definition
    class FeedHandler {
        private:
            OrderBook<ORDERIDTYPE, ORDERTYPE> order_book_;
            MessageParser parser_;

            #ifdef PROFILE
                HRTimer timer_;
                LagHistogram add_;
                LagHistogram modify_;
                LagHistogram remove_;
                LagHistogram trade_;
                LagHistogram midquote_;
                LagHistogram book_print_;
            #endif

        public: 
            #ifdef PROFILE
                //Constructor
                FeedHandler()
                : order_book_()
                , parser_()
                , timer_()
                , add_("AddOrder")
                , modify_("ModifyOrder")
                , remove_("RemoveOrder")
                , trade_("Trade")
                , midquote_("MidQuote Print")
                , book_print_("Book Print")
                {}

                //Destructor
                ~FeedHandler() {
                    order_book_.getLoggerReference().stopLogger();

                    add_.print();
                    modify_.print();
                    remove_.print();
                    trade_.print();
                    midquote_.print();
                    book_print_.print();
                }
            #endif

            //Function to process a single msg received as a char array
            void processMessage(char *line) {
                //If DEBUG is defined, it creates a copy of the original msg for debugging purposes
                #ifdef DEBUG
                    uint32_t len = strlen(line);
                    char orig_msg[len];
                    strncpy(orig_msg, line, strlen(line));
                #endif

                //Determine msg type and check whether it is successfully processed or not
                MessageType mt = parser_.getMessageType(line);
                bool valid_message = false;

                //If the msg is a trade, parse it into a TradeMessage
                //If trade price != 0, it is handled and logged
                if (mt == eMT_Trade) {
                    STARTPROFILE();
                    TradeMessage tm;
                    parser_.parseTrade(line, tm);
                    if (tm.trade_price_ != 0) {
                        valid_message = true;
                        DB("%s", orig_msg);
                        order_book_.handleTrade(tm);
                    }
                    STOPPROFILE(trade_);
                } else {
                    //For other msf types, create an ORDERTYPE obj dynamically and parse it into this obj
                    STARTPROFILE();
                    ORDERTYPE *ole = new ORDERTYPE();
                    parser_.parseOrder(line, *ole);

                    //Add, Modify and Remove order. If msg type is unknown, order obj is deleted and trigger assertion failure
                    if (ole->order_side_ == eS_Unknown) {
                        delete ole;
                    } else {
                        switch (mt) {
                            case eMT_Add:       order_book_.addOrder(ole);
                                                STOPPROFILE(add_);
                                                valid_message = true;
                                                break;
                            case eMT_Modify:    order_book_.modifyOrder(ole);
                                                STOPPROFILE(modify_);
                                                valid_message = true;
                                                break;
                            case eMT_Remove:    order_book_.removeOrder(ole);
                                                STOPPROFILE(remove_);
                                                valid_message = true;
                                                break;
                            default:            delete ole;
                                                FAILASSERT();
                                                break;
                        }
                    }      
                }

                //If msg was valid, print debugging info and book's midpoint
                if (valid_message) {
                    #ifdef DEBUG
                        DB("%s", orig_msg);
                        order_book_.printBook();
                    #endif
                    STARTPROFILE();
                    order_book_.printMidpoint();
                    STOPPROFILE(midquote_);
                }
            }

            //Function to print the current state of the order book
            //Skip if DEBUG is enabled to avoid redundant print
            void printCurrentOrderBook() {
                #ifdef DEBUG
                    return;
                #endif
                STARTPROFILE()
                order_book_.printBook();
                STOPPROFILE(book_print_);
            }
    };
}

#endif