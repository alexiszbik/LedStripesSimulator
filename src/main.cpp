#include "Application.h"

#include <exception>
#include <iostream>

int main()
{
    try
    {
        Application app("LedStripes");
        return app.run();
    }
    catch (const std::exception& ex)
    {
        std::cerr << "Error: " << ex.what() << '\n';
        return 1;
    }
}
