#pragma once

#include "Precompiled.hpp"

//
// Window
//
//  Creates and manages an application window with OpenGL context.
//  Supports multiple windows and contexts.
//
//  Example usage:
//      System::Window window;
//      window.Initialize();
//      
//      while(window.IsOpen())
//      {
//          window.ProcessEvents();
//          
//          /* ... */
//          
//          window.Present();
//      }
//
//  Binding events:
//      void Class::OnKeyboardKey(const Window::Events::KeyboardKey& event) { /*...*/ }
//      
//      Class instance;
//      Receiver<void(const Window::Events::KeyboardKey&)> receiver;
//      receiver.Bind<InputState, &InputState::OnKeyboardKey>(&instance);
//      receiver.Subscribe(window.events.keyboardKey);
//

namespace System
{
    // Window initialization struct.
    struct WindowInfo
    {
        WindowInfo();

        std::string name;
        int width;
        int height;
        bool vsync;
    };

    // Window class.
    class Window : private NonCopyable
    {
    public:
        Window();
        ~Window();

        // Restores instance to its original state.
        void Cleanup();

        // Initializes the window instance.
        bool Initialize(const WindowInfo& info = WindowInfo());

        // Makes window's context current.
        void MakeContextCurrent();

        // Processes window events.
        void ProcessEvents();

        // Presents backbuffer content on the window.
        void Present();

        // Closes the window.
        void Close();

        // Checks if window is open.
        bool IsOpen() const;

        // Checks if window is focused.
        bool IsFocused() const;

        // Gets window's width.
        int GetWidth() const;

        // Gets window's height.
        int GetHeight() const;

        // Gets window's private data.
        GLFWwindow* GetPrivate();

    public:
        // Window events.
        struct Events
        {
            // Move event.
            struct Move
            {
                int x;
                int y;
            };

            Dispatcher<void(const Move&)> move;

            // Resize event.
            struct Resize
            {
                int width;
                int height;
            };

            Dispatcher<void(const Resize&)> resize;

            // Focus event.
            struct Focus
            {
                bool focused;
            };

            Dispatcher<void(const Focus&)> focus;

            // Close event.
            struct Close
            {
            };

            Dispatcher<void(const Close&)> close;

            // Keyboard key event.
            struct KeyboardKey
            {
                int key;
                int scancode;
                int action;
                int mods;
            };

            Dispatcher<void(const KeyboardKey&)> keyboardKey;

            // Text input event.
            struct TextInput
            {
                unsigned int character;
            };

            Dispatcher<void(const TextInput&)> textInput;

            // Mouse button event.
            struct MouseButton
            {
                int button;
                int action;
                int mods;
            };

            Dispatcher<void(const MouseButton&)> mouseButton;

            // Mouse scroll event.
            struct MouseScroll
            {
                double offset;
            };

            Dispatcher<void(const MouseScroll&)> mouseScroll;

            // Cursor position event.
            struct CursorPosition
            {
                double x;
                double y;
            };

            Dispatcher<void(const CursorPosition&)> cursorPosition;

            // Cursor enter event.
            struct CursorEnter
            {
                bool entered;
            };

            Dispatcher<void(const CursorEnter&)> cursorEnter;
        } events;

    private:
        // Window implementation.
        GLFWwindow* m_window;

        // Initialization state.
        bool m_initialized;
    };
}
