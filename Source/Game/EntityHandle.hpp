#pragma once

#include "Precompiled.hpp"

//
// Entity Handle
//
//  References an unique entity in the world. Consists of two integers -
//  an identifier and a version. The version counter is increased everytime
//  an unique identifier is reused.
//

namespace Game
{
    // Entity handle class.
    class EntityHandle
    {
    public:
        // Friend declarations.
        friend class EntitySystem;

    public:
        // Constructor.
        EntityHandle() :
            m_identifier(0),
            m_version(0)
        {
        }

        // Copy constructor.
        EntityHandle(const EntityHandle& other) :
            m_identifier(other.m_identifier),
            m_version(other.m_version)
        {
        }

        // Comparison operators.
        bool operator==(const EntityHandle& other) const
        {
            return m_identifier == other.m_identifier && m_version == other.m_version;
        }

        bool operator!=(const EntityHandle& other) const
        {
            return m_identifier != other.m_identifier || m_version != other.m_version;
        }

        // Sorting operator.
        bool operator<(const EntityHandle& other) const
        {
            return m_identifier < other.m_identifier;
        }

        // Gets the identifier.
        int GetIdentifier() const
        {
            return m_identifier;
        }

        // Gets the version.
        int GetVersion() const
        {
            return m_version;
        }

    private:
        // Handle data.
        int m_identifier;
        int m_version;
    };
}

// Hashing functors.
namespace std
{
    // Container element.
    template<>
    struct hash<Game::EntityHandle>
    {
        std::size_t operator()(const Game::EntityHandle& handle) const
        {
            // Use the identifier as a hash.
            return handle.GetIdentifier();
        }
    };

    // Pair element.
    template<>
    struct hash<std::pair<Game::EntityHandle, Game::EntityHandle>>
    {
        std::size_t operator()(const std::pair<Game::EntityHandle, Game::EntityHandle>& pair) const
        {
            // Use combined identifiers as a hash.
            // This turns two 32bit integers into 64bit one.
            // We assume std::size_t is 64bit, but it's fine if it's not.
            return (std::size_t)pair.first.GetIdentifier() * std::numeric_limits<int>::max() + pair.second.GetIdentifier();
        }
    };
}
