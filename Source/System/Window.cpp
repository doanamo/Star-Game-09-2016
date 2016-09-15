#include "Precompiled.hpp"
#include "Window.hpp"
using namespace System;

namespace
{
    // Log message strings.
    #define LogInitializeError() "Failed to initialize a window! "

    // Instance counter for GLFW library.
    bool LibraryInitialized = false;
    int InstanceCount = 0;

    // Window callbacks.
    void ErrorCallback(int error, const char* description)
    {
        Log() << "GLFW Error: " << description;
    }

    void MoveCallback(GLFWwindow* window, int x, int y)
    {
        Assert(window != nullptr);

        // Get the window instance.
        auto instance = reinterpret_cast<System::Window*>(glfwGetWindowUserPointer(window));
        Assert(instance != nullptr);

        // Send an event.
        Window::Events::Move eventData;
        eventData.x = x;
        eventData.y = y;

        instance->events.move(eventData);
    }

    void ResizeCallback(GLFWwindow* window, int width, int height)
    {
        Assert(window != nullptr);

        // Get the window instance.
        auto instance = reinterpret_cast<System::Window*>(glfwGetWindowUserPointer(window));
        Assert(instance != nullptr);

        // Send an event.
        Window::Events::Resize eventData;
        eventData.width = width;
        eventData.height = height;

        instance->events.resize(eventData);
    }

    void FocusCallback(GLFWwindow* window, int focused)
    {
        Assert(window != nullptr);

        // Get the window instance.
        auto instance = reinterpret_cast<System::Window*>(glfwGetWindowUserPointer(window));
        Assert(instance != nullptr);

        // Send an event.
        Window::Events::Focus eventData;
        eventData.focused = focused > 0;

        instance->events.focus(eventData);
    }

    void CloseCallback(GLFWwindow* window)
    {
        Assert(window != nullptr);

        // Get the window instance.
        auto instance = reinterpret_cast<System::Window*>(glfwGetWindowUserPointer(window));
        Assert(instance != nullptr);

        // Send an event.
        Window::Events::Close eventData;

        instance->events.close(eventData);
    }

    void KeyboardKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        Assert(window != nullptr);

        // Get the window instance.
        auto instance = reinterpret_cast<System::Window*>(glfwGetWindowUserPointer(window));
        Assert(instance != nullptr);

        // Send an event.
        Window::Events::KeyboardKey eventData;
        eventData.key = key;
        eventData.scancode = scancode;
        eventData.action = action;
        eventData.mods = mods;

        instance->events.keyboardKey(eventData);
    }

    void TextInputCallback(GLFWwindow* window, unsigned int character)
    {
        Assert(window != nullptr);

        // Get the window instance.
        auto instance = reinterpret_cast<System::Window*>(glfwGetWindowUserPointer(window));
        Assert(instance != nullptr);

        // Send an event.
        Window::Events::TextInput eventData;
        eventData.character = character;

        instance->events.textInput(eventData);
    }

    void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
    {
        Assert(window != nullptr);

        // Get the window instance.
        auto instance = reinterpret_cast<System::Window*>(glfwGetWindowUserPointer(window));
        Assert(instance != nullptr);

        // Send an event.
        Window::Events::MouseButton eventData;
        eventData.button = button;
        eventData.action = action;
        eventData.mods = mods;

        instance->events.mouseButton(eventData);
    }

    void MouseScrollCallback(GLFWwindow* window, double offsetx, double offsety)
    {
        Assert(window != nullptr);

        // Get the window instance.
        auto instance = reinterpret_cast<System::Window*>(glfwGetWindowUserPointer(window));
        Assert(instance != nullptr);

        // Send an event.
        Window::Events::MouseScroll eventData;
        eventData.offset = offsety;

        instance->events.mouseScroll(eventData);
    }

    void CursorPositionCallback(GLFWwindow* window, double x, double y)
    {
        Assert(window != nullptr);

        // Get the window instance.
        auto instance = reinterpret_cast<System::Window*>(glfwGetWindowUserPointer(window));
        Assert(instance != nullptr);

        // Send an event.
        Window::Events::CursorPosition eventData;
        eventData.x = x;
        eventData.y = y;

        instance->events.cursorPosition(eventData);
    }

    void CursorEnterCallback(GLFWwindow* window, int entered)
    {
        Assert(window != nullptr);

        // Get the window instance.
        auto instance = reinterpret_cast<System::Window*>(glfwGetWindowUserPointer(window));
        Assert(instance != nullptr);

        // Send an event.
        Window::Events::CursorEnter eventData;
        eventData.entered = entered != 0;

        instance->events.cursorEnter(eventData);
    }
}

Window::Window() :
    m_window(nullptr),
    m_initialized(false)
{
    // Increase instance count.
    ++InstanceCount;
}

Window::~Window()
{
    // Cleanup instance.
    this->Cleanup();

    // Decrease instance count.
    --InstanceCount;

    // Shutdown GLFW library.
    if(InstanceCount == 0)
    {
        if(LibraryInitialized)
        {
            glfwTerminate();
            LibraryInitialized = false;
        }
    }
}

void Window::Cleanup()
{
    if(!m_initialized)
        return;

    // Cleanup event dispatchers.
    events.move.Cleanup();
    events.resize.Cleanup();
    events.focus.Cleanup();
    events.close.Cleanup();
    events.keyboardKey.Cleanup();
    events.textInput.Cleanup();
    events.mouseButton.Cleanup();
    events.mouseScroll.Cleanup();
    events.cursorPosition.Cleanup();
    events.cursorEnter.Cleanup();

    // Destroy the window.
    if(m_window != nullptr)
    {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
    }

    // Reset initialization state.
    m_initialized = false;
}

bool Window::Initialize()
{
    // Cleanup this instance.
    this->Cleanup();

    // Setup a cleanup guard.
    SCOPE_GUARD
    (
        if(!m_initialized)
        {
            m_initialized = true;
            this->Cleanup();
        }
    );

    // Initialize GLFW library.
    if(!LibraryInitialized)
    {
        glfwSetErrorCallback(ErrorCallback);

        if(!glfwInit())
        {
            Log() << LogInitializeError() << "Couldn't initialize GLFW library.";
            return false;
        }

        LibraryInitialized = true;
    }

    // Create the window.
    glfwWindowHint(GLFW_RED_BITS, 8);
    glfwWindowHint(GLFW_GREEN_BITS, 8);
    glfwWindowHint(GLFW_BLUE_BITS, 8);
    glfwWindowHint(GLFW_ALPHA_BITS, 8);
    glfwWindowHint(GLFW_DEPTH_BITS, 24);
    glfwWindowHint(GLFW_STENCIL_BITS, 8);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    m_window = glfwCreateWindow(1024, 576, "Game", nullptr, nullptr);

    if(m_window == nullptr)
    {
        Log() << LogInitializeError() << "Couldn't create the window.";
        return false;
    }

    // Set window user data.
    glfwSetWindowUserPointer(m_window, this);

    // Add event callbacks.
    glfwSetWindowPosCallback(m_window, MoveCallback);
    glfwSetFramebufferSizeCallback(m_window, ResizeCallback);
    glfwSetWindowFocusCallback(m_window, FocusCallback);
    glfwSetWindowCloseCallback(m_window, CloseCallback);
    glfwSetKeyCallback(m_window, KeyboardKeyCallback);
    glfwSetCharCallback(m_window, TextInputCallback);
    glfwSetMouseButtonCallback(m_window, MouseButtonCallback);
    glfwSetScrollCallback(m_window, MouseScrollCallback);
    glfwSetCursorPosCallback(m_window, CursorPositionCallback);
    glfwSetCursorEnterCallback(m_window, CursorEnterCallback);

    // Make window context current.
    glfwMakeContextCurrent(m_window);

    // Set the swap interval.
    glfwSwapInterval((int)true);

    /*
    // Initialize GLEW library.
    glewExperimental = GL_TRUE;
    GLenum error = glewInit();

    if(error != GLEW_OK)
    {
        Log() << "GLEW Error: " << glewGetErrorString(error);
        Log() << LogInitializeError() << "Couldn't initialize GLEW library.";
        return false;
    }

    // Check created OpenGL context.
    int glMajor = glfwGetWindowAttrib(m_window, GLFW_CONTEXT_VERSION_MAJOR);
    int glMinor = glfwGetWindowAttrib(m_window, GLFW_CONTEXT_VERSION_MINOR);

    Log() << "Created OpenGL " << glMajor << "." << glMinor << " context.";
    */

    // Success!
    int windowWidth, windowHeight;
    glfwGetFramebufferSize(m_window, &windowWidth, &windowHeight);
    Log() << "Create a window (" << windowWidth << "x" << windowHeight << ").";

    return m_initialized = true;
}

void Window::MakeContextCurrent()
{
    if(!m_initialized)
        return;

    glfwMakeContextCurrent(m_window);
}

void Window::ProcessEvents()
{
    if(!m_initialized)
        return;

    glfwPollEvents();
}

void Window::Present()
{
    if(!m_initialized)
        return;

    glfwSwapBuffers(m_window);
}

void Window::Close()
{
    if(!m_initialized)
        return;

    glfwSetWindowShouldClose(m_window, GL_TRUE);
}

bool Window::IsOpen() const
{
    if(!m_initialized)
        return false;

    return glfwWindowShouldClose(m_window) == 0;
}

bool Window::IsFocused() const
{
    if(!m_initialized)
        return false;

    return glfwGetWindowAttrib(m_window, GLFW_FOCUSED) > 0;
}

int Window::GetWidth() const
{
    if(!m_initialized)
        return 0;

    int width = 0;
    glfwGetFramebufferSize(m_window, &width, nullptr);
    return width;
}

int Window::GetHeight() const
{
    if(!m_initialized)
        return 0;

    int height = 0;
    glfwGetFramebufferSize(m_window, nullptr, &height);
    return height;
}

GLFWwindow* Window::GetPrivate()
{
    return m_window;
}
