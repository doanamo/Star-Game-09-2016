#include "Precompiled.hpp"
#include "Config.hpp"
using namespace System;

Config::Config() :
    m_initialized(false)
{
}

Config::~Config()
{
    this->Cleanup();
}

void Config::Cleanup()
{
    if(!m_initialized)
        return;

    // Clear the variable map.
    Utility::ClearContainer(m_variables);

    // Initialization state.
    m_initialized = false;
}

bool Config::Initialize(const std::string filename)
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

    // Parse config file.
    // Todo: Implement loading from a file.

    // Success!
    return m_initialized = true;
}
