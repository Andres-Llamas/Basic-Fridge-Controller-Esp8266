#ifndef TimeManager_h
#define TimeManager_h
#include "CustomStructs.h"

class TimeManager
{
private:

public:
    static clockTime currentTime;
    static void Initialization();
    static void UpdateTime();
    /// @brief Displays an specific date and time
    /// @param timeToShow
    static void ShowDateAndTime(clockTime timeToShow);

    /// @brief Displays the current date and time obtained from ntpc
    static void ShowDateAndTime();    
};
#endif