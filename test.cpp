//Standard C++ Libraries
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>

//Include defined header file
#include "DL_List.h"
#include "Feed_Handler.h"
#include "HR_Timer.h"
#include "Lag_Histogram.h"
#include "Logger.h"
#include "Quick_Trade.h"
#include "Utilities.h"

using namespace QuickTrade;

//Define TestFunction as a pointer to a function that returns a bool
typedef bool (*TestFunction)();

//Define a structure for pairing a test name with a test pointer
struct TestFunctionPair {
    TestFunctionPair(std::string name, TestFunction function)
    : test_name_(name), 
    function_(function)
    {}
    std::string test_name_;
    TestFunction function_;
};

//Global vector to store all test function pairs
std::vector<TestFunctionPair> test_functions_;

/*------------------------------------------------------------------------------------------*/
//   SECTION -> DL List
/*------------------------------------------------------------------------------------------*/

//Test the basic functionality of DL List class by adding and removing a single node
bool testDLListCompilation() {
    DLList<OrderLevelEntry> order_list_;
    OrderLevelEntry ole;
    ole.order_id_ = 1;
    ole.order_qty_ = 10;
    ole.order_side_ = eS_Buy;
    DB("Adding order: ");
    ole.printSelf();
    try {
        order_list_.addNode(&ole);
        order_list_.removeNode(&ole);
        return true;
    } catch (...) {
        return false;
    }
}

//Test addition of multiple nodes to the DL List and their subsequent removal
bool testDLListAddMultiple() {
    std::vector<OrderLevelEntry*> orders_;
    DLList<OrderLevelEntry> order_list_;
    for (uint32_t i = 0; i < 100; ++i) {
        orders_.push_back(new OrderLevelEntry());

        OrderLevelEntry &ole = *orders_[orders_.size() - 1];
        ole.order_id_ = i;
        ole.order_qty_ = i * 10;
        ole.order_side_ = eS_Buy;
        try {
            order_list_.addNode(&ole);
        } catch (...) {
            return false;
        }
    }
    for (uint32_t i = 0; i < orders_.size(); ++i) {
        while (order_list_.getTail() != 0) {
            OrderLevelEntry *ole = order_list_.getTail();
            order_list_.removeNode(ole);
            delete ole;
        }
    }
    return true;
}

//Test selective removal of nodes from the DL List
bool testDLListRemoveNodes() {
    std::vector<OrderLevelEntry*> orders_;
    DLList<OrderLevelEntry> order_list_;

    std::vector<OrderLevelEntry*> nodes_to_remove;

    for (uint32_t i = 0; i < 100; ++i) {
        orders_.push_back(new OrderLevelEntry());

        OrderLevelEntry &ole = *orders_[orders_.size() - 1];
        ole.order_id_ = i;
        ole.order_qty_ = i * 10;
        ole.order_side_ = eS_Buy;
        try {
            if (i % 3 == 0) {
                nodes_to_remove.push_back(&ole);
            }
            order_list_.addNode(&ole);
        } catch (...) {
            return false;
        }
    }

    try {
        for (uint32_t i = 0; i < nodes_to_remove.size(); ++i) {
            order_list_.removeNode(nodes_to_remove[i]);
            nodes_to_remove[i] = 0;
        }
        nodes_to_remove.clear();
    } catch (...) {
        return false;
    }

    for (uint32_t i = 0; i < orders_.size(); ++i) {
        order_list_.removeNode(orders_[i]);
        delete orders_[i];
    }
    return true;
}

/*------------------------------------------------------------------------------------------*/
//   SECTION -> Logger
/*------------------------------------------------------------------------------------------*/

//Test the performance of a logger class by repeatedly logging msgs and measure the time taken
bool testLogger() {
    HRTimer timer;
    LagHistogram lh = LagHistogram("testLogger()");
    Logger l;
    for (uint32_t i = 0; i < 1000; ++i) {
        timer.start();
        char msg[50];
        snprintf(msg, sizeof(msg), "Testing %d\n", i);
        l.print(msg);
        lh.add(timer.stop());
    }
    lh.print();
    return true;
}

/*------------------------------------------------------------------------------------------*/
//   SECTION -> Testing Framework
/*------------------------------------------------------------------------------------------*/

//Add a test function and its name to the testFunctions vector
void addTest(std::string name, TestFunction func) {
    test_functions_.push_back(TestFunctionPair(name, func));
}

//Initialize the test framework and register all available tests
void initTests() {
    addTest("DLList: addition && single dtor", &testDLListCompilation);
    addTest("DLList: add multiple && multi dtor", &testDLListAddMultiple);
    addTest("DLList: remove node", &testDLListRemoveNodes);
    addTest("Logger: test logger performance", &testLogger);
}

/*------------------------------------------------------------------------------------------*/
int main(int argc, char** argv) {
    initTests();

    int tests_failed = 0;
    int tests_passed = 0;

    while (1) {
        std::cout << "--------------------------------------" << std::endl;
        std::cout << "Please select from the following tests: " << std::endl;
        std::cout << "A: Run All Tests" << std::endl;
        for (uint32_t i = 0; i < test_functions_.size(); ++i) {
            std::cout << i << ": " << test_functions_[i].test_name_ << std::endl;
        }
        std::cout << "x: Exit" << std::endl;

        char input[2] = { '0', '0' };
        std::cin >> input;
        int input_int = atoi(input);
        if (input[0] == 'x' || input[0] == 'X') {
            break;
        } else if (input[0] == 'a' || input[0] == 'A') {
            for (uint32_t i = 0; i < test_functions_.size(); ++i) {
                test_functions_[i].function_() == false ?
                ++tests_failed :
                ++tests_passed;
            }
            break;
        }

        bool passed = test_functions_[input_int].function_();

        if (passed) {
            std::cout << "TEST PASSED.\n";
            ++tests_passed;
        } else {
            std::cout << "TEST FAILED.\n";
            ++tests_failed;
        }
    }

    if (tests_failed != 0) {
        std::cout << "TESTS FAILED. See output for details." << std::endl;
        std::cout << "PASSED: " << tests_passed << std::endl;
        std::cout << "FAILED: " << tests_failed << std::endl;
    } else {
        std::cout << "All Tests Passed." << std::endl;
    }
}