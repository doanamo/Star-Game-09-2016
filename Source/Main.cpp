#include "Precompiled.hpp"
#include "System/Config.hpp"
#include "System/Window.hpp"

int main(int argc, char* argv[])
{
    Build::Initialize();
    Debug::Initialize();
    Logger::Initialize();

    // Initialize the config.
    System::Config config;
    if(!config.Initialize())
        return -1;

    SCOPE_GUARD(config.Cleanup());

    // Initialize the window.
    System::WindowInfo windowInfo;
    windowInfo.width = config.GetVariable<int>("Window.Width", 1024);
    windowInfo.height = config.GetVariable<int>("Window.Height", 576);
    windowInfo.vsync = config.GetVariable<bool>("Window.Vsync", true);

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
