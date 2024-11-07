//Preprocessor directive to ensure that the contents of the header file are only included once 
//Defines the JLOGGER macro, which prevents the file from being included multiple times
#ifndef JLOGGER
#define JLOGGER

//Standard C++ Libraries
#include <atomic>
#include <chrono>
#include <functional>
#include <iostream>
#include <mutex>
#include <queue>
#include <string>
#include <thread>

//Define preprocessor macro TRUE
#define TRUE 1

/*----------------------------------------------------------------------------------------------------*/
// LOGGER CLASS
/*----------------------------------------------------------------------------------------------------*/
class Logger {
    private:
        //Move msgs from queue to print queue.
        //Acquire lock on mutex, loop on msgs and push each to print queue and remove them from queue
        //Also set msg available flag to FALSE
        void copyMessages() {
            std::lock_guard<std::mutex> lock(mutex_);
            while (messages_.size() != 0) {
                messages_to_print_.push(messages_.front());
                messages_.pop();
            }
            messages_available_ = false;
        }

        //Called when first print() call is made and it creates and starts the logging thread
        void init() {
            thread_ = new std::thread(std::bind(&Logger::runLogger, this));
        }

        //Print msgs from print queue to stderr
        void printMessages() {
            while (messages_to_print_.size() != 0) {
                fprintf(stderr, "%s", messages_to_print_.front().c_str());
                messages_to_print_.pop();
            }
        }

        std::atomic<bool> exit_;                        //Flag to indicate if the logging thread should exit
        std::atomic<bool> messages_available_;          //Flag to indicate if there are msgs available to be logged
        std::thread *thread_;                           //Pointer to the logging thread
        std::queue<std::string> messages_;              //Queue for holding msgs to be logged
        std::queue<std::string> messages_to_print_;     //Queue for holding msgs to be printed
        std::mutex mutex_;                              //To protect messages_ queue

    public:
        //Constructor
        Logger()
        : exit_(false)                  
        , messages_available_(false)    
        , thread_(0)                    
        , messages_()                   
        , mutex_()                      
        {}

        //Destructor
        ~Logger() {
            if (thread_ == 0) return;
            exit_ = true;
            thread_->join();
            delete thread_;
            thread_ = 0;
        }

        //Manual option to stop the logging thread and flush remaining logs
        void stopLogger() {
            exit_ = true;
            thread_->join();
            delete thread_;
            thread_ = 0;
        }

        //Add msg to the messages_ queue. 
        //If not yet started init() creates and start the thread
        //Acquire lock on mutex to ensure thread safe access to the queue, push the msg and set flag to true
        void print(std::string msg) {
            if (thread_ == 0) {
                init();
            }
            //Mutex protects against messing with messages_ while logging thread is copying out to it's own queue
            std::lock_guard<std::mutex> lock(mutex_);
            messages_.push(msg);
            messages_available_ = true;
        }

        //Main loop that runs the logging thread. It checks if there are any msgs available
        //If YES, call func to move the msgs to print queue and then print to stderr
        //If exit flag is TRUE, do this again to print any remaining msgs and then terminate the logging thread.
        void runLogger() {
            while (TRUE) {
                if (messages_available_ == true) {
                    copyMessages();
                    printMessages();
                }
                if (exit_ == true) {
                    copyMessages();
                    printMessages();
                    return;
                }
            }
        }
};

#endif