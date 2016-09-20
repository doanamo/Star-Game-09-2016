#include "Precompiled.hpp"
#include "System/Window.hpp"

System::Window Window;

int main(int argc, char* argv[])
{
    Build::Initialize();
    Debug::Initialize();
    Logger::Initialize();

    // Initialize the window.
    if(!Window.Initialize())
        return -1;

    SCOPE_GUARD(Window.Cleanup());

    // Main loop.
    while(Window.IsOpen())
    {
        Window.ProcessEvents();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        Window.Present();
    }

    return 0;
}
