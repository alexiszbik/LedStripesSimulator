#include "MidiInputHandler.h"

#include <iostream>

int main()
{
    try
    {
        MidiInputHandler handler("LedStripes");
        handler.run();
    }
    catch (const std::exception& ex)
    {
        std::cerr << "Error: " << ex.what() << '\n';
        return 1;
    }

    return 0;
}
