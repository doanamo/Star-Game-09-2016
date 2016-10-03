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
    struct EntityHandle
    {
        // Handle data.
        int identifier;
        int version;

        // Constructor.
        EntityHandle() :
            identifier(0),
            version(0)
        {
        }

        // Comparison operators.
        bool operator==(const EntityHandle& other) const
        {
            return identifier == other.identifier && version == other.version;
        }

        bool operator!=(const EntityHandle& other) const
        {
            return identifier != other.identifier || version != other.version;
        }

        // Sorting operator.
        bool operator<(const EntityHandle& other) const
        {
            return identifier < other.identifier;
        }
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
            return handle.identifier;
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
            return (std::size_t)pair.first.identifier * std::numeric_limits<int>::max() + pair.second.identifier;
        }
    };
}
