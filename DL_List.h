//Preprocessor directive to ensure that the contents of the header file are only included once 
//Defines the JDLLIST macro, which prevents the file from being included multiple times
#ifndef __JDLLIST__
#define __JDLLIST__

//Standard C++ Libraries
#include <stdio.h>

//Namespace Definition
namespace QuickTrade {
    //Template class def for DL_List, NODE is a template param
    template <typename NODE>
    class DLList {
        private:
            NODE *head_;    //Pointer for the first node of the list
            NODE *tail_;    //Pointer for the last node of the list
        
        public:
            //Constructor initializing head and tail to null
            DLList()
            : head_(0)
            , tail_(0)
            {}

            //Virtual Destructor for allowing inheritance
            virtual ~DLList() {}

            //Function to add a new node
            void addNode(NODE *input) {
                //If list is empty
                if (head_ == 0) {
                    head_ = input;
                    tail_ = head_;
                    return;
                }

                //If list is not empty
                head_->next_ = input;
                input->previous_ = head_;
                head_ = input;
            }

            //Function to remove a node from the list
            void removeNode(NODE *target) {
                //If node is in between the list
                if (target->previous_ != 0 && target->next_ != 0) {
                    target->previous_->next_ = target->next_;
                    target->next_->previous_ = target->previous_;
                } 
                //Delete tail node
                else if (target == tail_ && target->next_ != 0) {
                    tail_ = target->next_;
                    tail_->previous_ = 0;
                }
                //Delete head node
                else if (target == head_ && target->previous_ != 0) {
                    head_ = target->previous_;
                    head_->next_ = 0;
                }
                //Handling list with single element
                else {
                    head_ = 0;
                    tail_ = 0;
                    return;
                }
            }

            //Get Head and Tail pointers
            NODE *getHead() const { return head_; }
            NODE *getTail() const { return tail_; }

            //Debug function to print list
            #ifdef DEBUG
            void printList() {
                NODE *head = tail_->previous_;
                while (head != tail_) {
                    fprintf(stderr, "NODE: ");
                    head->printSelf();
                    head = head->previous_;
                }
                fprintf(stderr, "TAIL NODE: ");
                head->printSelf();
            }
            #endif
    };
}

#endif