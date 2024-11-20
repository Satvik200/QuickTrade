//Preprocessor directive to ensure that the contents of the header file are only included once 
//Defines the JTYPES macro, which prevents the file from being included multiple times
#ifndef __JTYPES__
#define __JTYPES__

//Standard C++ Libraries
#include <iostream>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

//Include defined header file
#include "Utilities.h"

//Constants for min and max msg length and max price
#define MESSAGE_LEN_MIN 5
#define MESSAGE_LEN_MAX 36
#define MAXPRICE 100000 * 100

//DEBUG macro: If debug is defined -> DB will print to stderr else DB do nothing
//Used variadic macros (...) to accept any number of arguments
#if DEBUG
   #define DB(...) { fprintf(stderr, __VA_ARGS__); }
#else
   #define DB(...)
#endif

//Platform-specifc pointer type definition
//ILP32 is a data model where int, long and pointer are 32-bits
#ifdef __ILP32__
   #define PTR unsigned long
#else
   #define PTR uint64_t
#endif

//Macro that forces an assertion failure
#define FAILASSERT() { assert(0==1); }

//Optimization hints for the compiler about branch prediction 
//It has been used to optimize if-else statements
#define LIKELY(expr) (__builtin_expect(!!(expr), 1))
#define UNLIKELY(expr) (__builtin_expect(!!(expr), 0))

//Function to double the size of a char buffer
//It stores old size, doubles the buffer size, reallocates memory, null-terminates the new portion
void growBuffer(char *&buf, int &max_buffer) {
    int old_max = max_buffer;
    max_buffer *= 2;
    buf = (char *)realloc(buf, max_buffer * sizeof(char));
    memset(&buf[old_max], '\0', max_buffer - old_max);
}

//Fucntion for safe string copy
//Checks source length, grows buffer(if needed), copies string and updates index
void safeCopyToBuffer(char *&dest_root, const char *source, int &index, int &max_buffer) {
    int len = strlen(source);
    if (len == 0) return;
    while (index + len >= max_buffer) {
        int old_max = max_buffer;
        max_buffer *= 2;
        dest_root = (char *)realloc(dest_root, max_buffer * sizeof(char));
        memset(&dest_root[old_max], '\0', max_buffer - old_max);
    }
    strncpy(&dest_root[index], source, len);
    index += len;
}

//Enum for different types of trading msgs
enum MessageType {
    eMT_Unknown,
    eMT_Add,
    eMT_Remove,
    eMT_Modify,
    eMT_Trade
};

//Enum for trade sides-buy/sell
enum Side {
    eS_Unknown,
    eS_Buy,
    eS_Sell
};

//Struct representing an order entry in a doubly-linked list
struct OrderLevelEntry {

    //Constructor for initializing all members to 0/unknown 
    OrderLevelEntry()
    : order_id_(0)
    , order_price_(0)
    , order_qty_(0)
    , order_side_(eS_Unknown)
    , next_(0)
    , previous_(0)
    {}

    //Debug function to print order details
    void printSelf() {
        DB("[ORDER] ID: %u.  QTY: %u.  SIDE: %d.  NEXT: %llu. PREV: %llu\n",
            order_id_,
            order_qty_,
            order_side_,
            (PTR)next_,
            (PTR)previous_);
    }

    uint32_t order_id_;                 //Unique Identifier (32-bit)
    unsigned long long order_price_;    //Order Price (64-bit)
    uint32_t order_qty_;                //Order Quantity (32-bit)
    Side order_side_;                   //Order Side - Buy/Sell

    //Pointers for doubly-linked list
    OrderLevelEntry *next_;
    OrderLevelEntry *previous_;
};

//Struct for trade msgs with quantity & price info, plus a debug print method
struct TradeMessage {
    uint32_t trade_qty_;
    unsigned long long trade_price_;
    void printSelf() {
        DB("[TRADE] QTY: %u. PRICE: %llu.\n",
            trade_qty_,
            trade_price_);
    }
};

#endif