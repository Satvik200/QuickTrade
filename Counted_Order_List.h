//Preprocessor directive to ensure that the contents of the header file are only included once 
//Defines the JCOUNTEDORDERLIST macro, which prevents the file from being included multiple times
#ifndef __JCOUNTEDORDERLIST__
#define __JCOUNTEDORDERLIST__

//Standard C++ Libraries
#include <stdint.h>

//Include defined header file
#include "DL_List.h"
#include "Logger.h"

namespace QuickTrade {
    template <typename NODE>
    //Class Definition
    class CountedOrderList : public DLList<NODE> {
        private:
            uint32_t level_quantity_;

        public:
            //Constructor Definition
            CountedOrderList()
            : level_quantity_(0)
            {}

            //Virtual destructor for proper inheritance cleanup
            virtual ~CountedOrderList() {}

            //Function to add new node
            //Updates total quantity and calls parent class's addNode Function
            void addNode(NODE *input) {
                level_quantity_ += input->order_qty_;
                DLList<NODE>::addNode(input);
            }

            //Function to remove node
            //Subtracts quantity from total and calls parent removeNode Function
            void removeNode(NODE *input) {
                level_quantity_ -= input->order_qty_;
                DLList<NODE>::removeNode(input);
            }

            //Function to update quantity of existing node
            void changeNodeQuantity(NODE *input, uint32_t new_quantity) {
                level_quantity_ -= input->order_qty_;
                level_quantity_ += new_quantity;
                input->order_qty_ = new_quantity;
            }

            //Clears entire price level and removes all nodes from list
            void clearLevel() {
                NODE *head = DLList<NODE>::getHead();
                while (head != DLList<NODE>::getTail()) {
                    removeNode(head);
                    head = head->previous_;
                }
                removeNode(head);
            }

           
            //Prints all orders at this price level
            void printLevel(char tag, char *&buffer, int &index, int &max_buffer) {
                NODE *tail = DLList<NODE>::getTail();
                while (tail != DLList<NODE>::getHead()) {
                    if (index + 50 > max_buffer) growBuffer(buffer, max_buffer);
                    index += snprintf(&buffer[index], sizeof(buffer) - index, "%c %u ", tag, tail->order_qty_);
                    tail = tail->next_;
                }
                if (index + 50 > max_buffer) growBuffer(buffer, max_buffer);
                index += snprintf(&buffer[index], sizeof(buffer) - index, "%c %u ", tag, tail->order_qty_);
            }

            //Returns total quantity at this price level
            uint32_t getQuantity() const { 
                return level_quantity_; 
            }

    };
}

#endif
