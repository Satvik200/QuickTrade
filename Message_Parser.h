//Preprocessor directive to ensure that the contents of the header file are only included once 
//Defines the JMESSAGEPARSER macro, which prevents the file from being included multiple times
#ifndef __JMESSAGEPARSER__
#define __JMESSAGEPARSER__

//Standard C++ Libraries
#include <stdint.h>
#include <cmath>

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_uint.hpp>
#include <boost/spirit/include/qi_real.hpp>

//Include defined header file
#include "FH_Error_Tracker.h"
#include "Utilities.h"

// Define MESSAGELENMAX
#define MESSAGELENMAX 1024

namespace QuickTrade {
    //Enum for different parsing status results
    enum ParseStatus {
        ePS_Good,
        ePS_CorruptMessage,
        ePS_BadQuantity,
        ePS_BadPrice,
        ePS_BadID,
        ePS_GenericBadValue
    };

    class MessageParser {
        private:
            /* These functions have been marked INLINE for performance */
            //Helper functions to parse tokens into numbers
            inline ParseStatus tokenizeAndConvertToUint(char *tk_msg, uint32_t &dest);
            inline ParseStatus tokenizeAndConvertToDouble(char *tk_msg, double &dest);

            //Functions to handle and report parsing errors
            inline void reportStatus(ParseStatus status);
            inline void failOrderParse(OrderLevelEntry &ole, ParseStatus status);
            inline void failTradeParse(TradeMessage &tm, ParseStatus status);

        public:
            //Class constructor and destructor
            MessageParser() {}
            ~MessageParser() {}

            /* These functions have been marked INLINE for performance */
            //Function to get msg type from input
            inline MessageType getMessageType(char *message);
            //Function to parse order msgs
            inline void parseOrder(char *tk_msg, OrderLevelEntry &ole);
            //Function for trade msgs
            inline void parseTrade(char *tk_msg, TradeMessage &tm);
    };

    //Identify msg type from first char
    //Validate msg length, map single chars to msg types and return unknown for invalid types
    inline MessageType getMessageType(char *tk_msg) {
        uint32_t len = strlen(tk_msg);
        if (len == 0 || len > MESSAGELENMAX) {
            FHErrorTracker::instance()->corruptMessage();
            return eMT_Unknown;
        }

        tk_msg = strtok(tk_msg, ",");
        switch(tk_msg[0]) {
            case 'A': return eMT_Add;
            case 'M': return eMT_Modify;
            case 'X': return eMT_Remove;
            case 'T': return eMT_Trade;
            default: return eMT_Unknown;
        }
    }

    //Convert next token to unsigned int
    inline ParseStatus tokenizeAndConvertToUint(char *tk_msg, uint32_t &dest) {
        tk_msg = strtok(NULL, ",");
        if (tk_msg == NULL) {
            return ePS_CorruptMessage;
        } else if (tk_msg[0] == '-') {
            return ePS_GenericBadValue;
        } else if (!boost::spirit::qi::parse(tk_msg, &tk_msg[strlen(tk_msg)], boost::spirit::qi::uint_, dest)) {
            return ePS_GenericBadValue;
        }
        return ePS_Good;
    }

    //Convert next token to floating point
    inline ParseStatus tokenizeAndConvertToDouble(char *tk_msg, double &dest) {
        tk_msg = strtok(NULL, ",");
        if (tk_msg == NULL) {
            return ePS_CorruptMessage;
        } else if (tk_msg[0] == '-') {
            return ePS_GenericBadValue;
        } else if (!boost::spirit::qi::parse(tk_msg, &tk_msg[strlen(tk_msg)], boost::spirit::qi::double_, dest)) {
            return ePS_GenericBadValue;
        }
        return ePS_Good;
    }

    //Centralized error reporting
    inline void reportStatus(ParseStatus status) {
        switch (status) {
            case ePS_Good:              FHErrorTracker::instance()->goodMessage();
                                        break;
            case ePS_CorruptMessage:    FHErrorTracker::instance()->corruptMessage();
                                        break;
            case ePS_BadQuantity:       FHErrorTracker::instance()->invalidQuantity();
                                        break;
            case ePS_BadPrice:          FHErrorTracker::instance()->invalidPrice();
                                        break;
            case ePS_BadID:             FHErrorTracker::instance()->invalidID();
                                        break;
            default:                    fprintf(stderr, "Unknown parsing error occurred.  Skipping report of error.\n");
                                        FAILASSERT();
                                        break;
        }
    }

    inline void MessageParser::failOrderParse(OrderLevelEntry &ole, ParseStatus status) {
        // Flag as failed parse
        ole.order_side_ = eS_Unknown;
        return reportStatus(status);
    }

    //Parses Complete trade msgs
    /* Msg format assumption:
        1. Comma-separated values
        2. Specific order of fields
        3. No whitespace
        
       Price Hanldng:
        1. Stored as int 
        2. Max 2 decimal place
        Upper limit checks */
    inline void MessageParser::parseOrder(char *tk_msg, OrderLevelEntry &ole) {
        ParseStatus result = tokenizeAndConvertToUint(tk_msg, ole.order_id_);
        if (result != ePS_Good) {
            if (result == ePS_CorruptMessage)
                return failOrderParse(ole, result);
            return failOrderParse(ole, ePS_BadID);
        }

        tk_msg = strtok(NULL, ",");
        if (tk_msg == NULL) {
            return failOrderParse(ole, ePS_CorruptMessage);
        } else {
            switch(tk_msg[0]) {
                case 'B':   ole.order_side_ = eS_Buy;
                            break;
                case 'S':   ole.order_side_ = eS_Sell;
                            break;
                default:    return failOrderParse(ole, ePS_CorruptMessage);
                            break;
            }
        }

        result = tokenizeAndConvertToUint(tk_msg, ole.order_qty_);
        if (result != ePS_Good) {
            if (result == ePS_CorruptMessage)
                return failOrderParse(ole, result);
            return failOrderParse(ole, ePS_BadQuantity);
        }

        if (ole.order_qty_ == 0) return failOrderParse(ole, ePS_BadQuantity);

        double price = 0;
        result = tokenizeAndConvertToDouble(tk_msg, price);
        if (result != ePS_Good) {
            if (result == ePS_CorruptMessage)
                return failOrderParse(ole, result);
            return failOrderParse(ole, ePS_BadPrice);
        }

        if (static_cast<unsigned long long>(price * 100) == 0) return failOrderParse(ole, ePS_BadPrice);

        if (price > ULLONG_MAX) {
            return failOrderParse(ole, ePS_BadPrice);
        }

        // Bail if not 0-2 decimals of precision
        if (std::fmod(price * 100, 1) != 0) {
            return failOrderParse(ole, ePS_BadPrice);
        }
        ole.order_price_ = static_cast<unsigned long long>(price * 100);
        FHErrorTracker::instance()->goodMessage();
    }

    inline void MessageParser::failTradeParse(TradeMessage &tm, ParseStatus status) {
        tm.trade_qty_ = 0;
        tm.trade_price_ = 0;
        reportStatus(status);
    }

    // Parses trade messages in format: T,quantity,price
    /* Message Requirements:
        1. Comma-delimited without spaces
        2. Quantity: must be > 0
        3. Price: must be > 0, max 2 decimals
        
       Price Storage:
        1. Internally stored as integers (price * 100)
        2. Range check against MAXPRICE
        3. Decimal precision validated */
    inline void MessageParser::parseTrade(char *tk_msg, TradeMessage &tm) {
        ParseStatus result = tokenizeAndConvertToUint(tk_msg, tm.trade_qty_);
        if (result != ePS_Good) {
            if (result == ePS_CorruptMessage)
                return failTradeParse(tm, result);;
            return failTradeParse(tm, ePS_BadQuantity);
        }

        if (tm.trade_qty_ == 0) return failTradeParse(tm, ePS_BadPrice);

        double price;
        result = tokenizeAndConvertToDouble(tk_msg, price);
        if (result != ePS_Good) {
            if (result == ePS_CorruptMessage)
                return failTradeParse(tm, result);
            return failTradeParse(tm, ePS_BadPrice);
        }

        if (price > ULLONG_MAX) {
            return failTradeParse(tm, ePS_BadPrice);
        }

        // Bail if not 0-2 decimals of precision
        if (std::fmod(price * 100, 1) != 0) {
            return failTradeParse(tm, ePS_BadPrice);
        }

        if (static_cast<unsigned long long>(price * 100) == 0) return failTradeParse(tm, ePS_BadPrice);

        tm.trade_price_ = static_cast<unsigned long long>(price * 100);
        if (tm.trade_price_ > MAXPRICE - 1) {
            DB("Price: %llu.  Max: %d\n", tm.trade_price_, MAXPRICE - 1);
            return failTradeParse(tm, ePS_BadPrice);
        }

        FHErrorTracker::instance()->goodMessage();
    }
}

#endif