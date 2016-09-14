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
//      window.Initialize(/* ... */);
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
//      
//      window.events.keyboardKey.Subscribe(receiver);
//

namespace System
{
    // Window class.
    class Window
    {
    public:
        Window();
        ~Window();

        // Restores instance to it's original state.
        void Cleanup();

        // Initializes the window instance.
        bool Initialize();

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
        // Public event dispatchers.
        struct EventDispatchers;

        struct Events
        {
            Events(EventDispatchers& dispatchers);

            // Move event.
            struct Move
            {
                int x;
                int y;
            };

            DispatcherBase<void(const Move&)>& move;

            // Resize event.
            struct Resize
            {
                int width;
                int height;
            };

            DispatcherBase<void(const Resize&)>& resize;

            // Focus event.
            struct Focus
            {
                bool focused;
            };

            DispatcherBase<void(const Focus&)>& focus;

            // Close event.
            struct Close
            {
            };

            DispatcherBase<void(const Close&)>& close;

            // Keyboard key event.
            struct KeyboardKey
            {
                int key;
                int scancode;
                int action;
                int mods;
            };

            DispatcherBase<void(const KeyboardKey&)>& keyboardKey;

            // Text input event.
            struct TextInput
            {
                unsigned int character;
            };

            DispatcherBase<void(const TextInput&)>& textInput;

            // Mouse button event.
            struct MouseButton
            {
                int button;
                int action;
                int mods;
            };

            DispatcherBase<void(const MouseButton&)>& mouseButton;

            // Mouse scroll event.
            struct MouseScroll
            {
                double offset;
            };

            DispatcherBase<void(const MouseScroll&)>& mouseScroll;

            // Cursor position event.
            struct CursorPosition
            {
                double x;
                double y;
            };

            DispatcherBase<void(const CursorPosition&)>& cursorPosition;

            // Cursor enter event.
            struct CursorEnter
            {
                bool entered;
            };

            DispatcherBase<void(const CursorEnter&)>& cursorEnter;
        } events;

        // Private event dispatchers.
        struct EventDispatchers
        {
            Dispatcher<void(const Events::Move&)>           move;
            Dispatcher<void(const Events::Resize&)>         resize;
            Dispatcher<void(const Events::Focus&)>          focus;
            Dispatcher<void(const Events::Close&)>          close;
            Dispatcher<void(const Events::KeyboardKey&)>    keyboardKey;
            Dispatcher<void(const Events::TextInput&)>      textInput;
            Dispatcher<void(const Events::MouseButton&)>    mouseButton;
            Dispatcher<void(const Events::MouseScroll&)>    mouseScroll;
            Dispatcher<void(const Events::CursorPosition&)> cursorPosition;
            Dispatcher<void(const Events::CursorEnter&)>    cursorEnter;
        };

    private:
        // Window implementation.
        GLFWwindow* m_window;

        // Event dispatchers.
        EventDispatchers m_dispatchers;

        // Initialization state.
        bool m_initialized;
    };
}
