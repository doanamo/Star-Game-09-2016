#pragma once

#include "Precompiled.hpp"

//
// Config
//
//  Stores application's configuration which can be
//  read from a file and then accessed in runtime.
//
//  Example usage:
//      System::Config config;
//      config.Initialize("Game.cfg");
//      
//      width = config.GetVariable<int>("Window.Width", 1024);
//      height = config.GetVariable<int>("Window.Height", 576);
//      vsync = config.GetVariable<bool>("Window.Vsync", true);
//

namespace System
{
    // Config class.
    class Config : private NonCopyable
    {
    public:
        Config();
        ~Config();

        // Restores instance to its original state.
        void Cleanup();

        // Initializes the config.
        bool Initialize(const std::string filename = "");

        // Sets a config variable.
        template<typename Type>
        void SetVariable(const std::string name, const Type& value);

        // Gets a config variable.
        template<typename Type>
        Type GetVariable(const std::string name, const Type& default);

    private:
        // Type declarations.
        typedef std::map<std::string, std::string> VariableMap;

    private:
        // Map of variables.
        VariableMap m_variables;

        // Initialization state.
        bool m_initialized;
    };

    // Template implementations.
    template<typename Type>
    void Config::SetVariable(const std::string name, const Type& value)
    {
        if(!m_initialized)
            return;

        // Set the variable value.
        m_variables.insert_or_assign(name, std::to_string(value));
    }

    template<typename Type>
    Type Config::GetVariable(const std::string name, const Type& default)
    {
        if(!m_initialized)
            return Type();

        // Find the variable by name.
        auto it = m_variables.find(name);

        // Set a new variable if it does not exist.
        if(it == m_variables.end())
        {
            auto result = m_variables.insert(std::make_pair(name, std::to_string(default)));
            Assert(result.second, "Failed to insert a config variable!");

            it = result.first;
        }

        // Convert variable value from a string and return it.
        std::istringstream convert(it->second);

        Type value;
        convert >> value;
        return value;
    }
}
