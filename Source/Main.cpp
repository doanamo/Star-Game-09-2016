#include "Precompiled.hpp"
#include "System/Window.hpp"

int main(int argc, char* argv[])
{
    Build::Initialize();
    Debug::Initialize();
    Logger::Initialize();

    // Initialize the window.
    System::WindowInfo windowInfo;
    windowInfo.width = 1024;
    windowInfo.height = 576;
    windowInfo.vsync = true;

    System::Window window;
    if(!window.Initialize(windowInfo))
        return -1;

    SCOPE_GUARD(window.Cleanup());

    // Main loop.
    while(window.IsOpen())
    {
        window.ProcessEvents();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        window.Present();
    }

    return 0;
}
