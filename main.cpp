//Standard C++ Libraries
#include <sched.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <fstream>

//Include defined header file
#include "Feed_Handler.h"
#include "FH_Error_Tracker.h"
#include "HR_Timer.h"
#include "Lag_Histogram.h"
#include "Utilities.h"

using namespace QuickTrade;

/*--------------------------------------------------------------------------------*/
// MAIN FUNCTION
/*--------------------------------------------------------------------------------*/

int main(int argc, char **argv) {
    //If the program is run without arguments, display usage instructions & exit with error -1
    if (argc == 1) {
      std::cout << "Usage: OrderBookProcessor <filename>" << std::endl;
      return -1;
   }

    //Initialize error tracking
    FHErrorTracker::instance()->init();

    //Boost priority of the process using Round-Robin Scheduling
    //Disabled because performace is better without it    
//    struct sched_param sp;
//    sp.sched_priority = sched_get_priority_max(SCHED_RR);
//    if (sched_setscheduler(0, SCHED_RR, &sp) == -1) {
//         std::cout << "Error setting scheduler to SCHED_RR.  Please run with " <<
//             "permissions for realtime scheduling.  Aborting." << std::endl;
//         return -1;
//     }

    //Instantiate Feed Handler and extract filename
    FeedHandler<uint32_t, OrderLevelEntry> feed;
    const std::string filename(argv[1]);

    //Open file in Read Mode
    FILE *pFile;
    try {
        pFile = fopen(filename.c_str(), "r");
        if (pFile == NULL) {
            throw;
        }
    } catch (...) {
        std::cout << "Error occured opening file: " << filename << ".  Please check the file and try again." << std::endl;
        return -1;
    }

    //Setting up processing logs
    //after every 10 msgs call feed.printCurrentOrderBook to display the current state of the order book
    uint32_t counter = 0;
    size_t len;
    char *buffer = NULL;
    while (!feof (pFile)) {
        while (1) {
            ssize_t read = getline(&buffer, &len, pFile);
            if (read == -1) break;

            feed.processMessage(buffer);

            if (buffer) free(buffer);
            buffer = 0;

            ++counter;
            if (counter % 10 == 0) {
                feed.printCurrentOrderBook();
            }
        }
    }

    //Close the file
    fclose(pFile);

    //Log Error Statistics
    FHErrorTracker::instance()->printStatistics();
    
    //Exit the program
    return 0;
}