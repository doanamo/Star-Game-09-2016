#pragma once

#include "Precompiled.hpp"
#include "EntityHandle.hpp"

//
// Entity System
//
//  Manages lifetime of entities and gives means for their identification.
//  
//  Example usage:
//      Game::EntitySystem entitySystem;
//      entitySystem.Initialize();
//      
//      EntityHandle entity = entitySystem.CreateEntity();
//      /*
//          Add components here!
//          Entity remains inactive until the next ProcessCommands() call.
//      */
//      entitySystem.ProcessCommands();
//      
//      entitySystem.DestroyEntity(entity);
//      /*
//          Entity remains active until the next ProcessCommands() call.
//      */
//      entitySystem.ProcessCommands();
//

namespace Game
{
    // Entity system class.
    class EntitySystem : private NonCopyable
    {
    public:
        EntitySystem();
        ~EntitySystem();

        // Restores class instance to its original state.
        void Cleanup();

        // Initializes the entity system.
        bool Initialize();

        // Creates an entity.
        EntityHandle CreateEntity();

        // Destroys an entity.
        void DestroyEntity(const EntityHandle& handle);

        // Destroys all entities.
        void DestroyAllEntities();

        // Processes entity commands.
        void ProcessCommands();

        // Checks if an entity handle is valid.
        bool IsHandleValid(const EntityHandle& entity) const;

        // Returns the number of active entities.
        int GetEntityCount() const;

    public:
        // Entity events.
        struct Events
        {
            // Finalize event.
            struct Finalize
            {
                const EntityHandle handle;
            };

            Dispatcher<bool(Finalize), CollectWhileTrue<bool>> finalize;

            // Create event.
            struct Create
            {
                const EntityHandle handle;
            };

            Dispatcher<void(Create)> create;

            // Destroy event.
            struct Destroy
            {
                const EntityHandle handle;
            };

            Dispatcher<void(Destroy)> destroy;
        } events;

    private:
        // Handle flags.
        struct HandleFlags
        {
            typedef int Type;

            enum Flag : Type
            {
                None    = 0,      // Handle has been allocated but is not being used.
                Valid   = 1 << 0, // Handle has been created but not finalized.
                Active  = 1 << 1, // Handle has been finalized and can be processed.
                Destroy = 1 << 2, // Handle has been scheduled to be destroyed.
            };

            static const Type Free = None;
        };

        // Handle entry structure.
        struct HandleEntry
        {
            EntityHandle handle;
            HandleFlags::Type flags;
            int nextFree;
        };

        // Entity command types.
        struct EntityCommands
        {
            enum Type
            {
                Invalid,
                Create,
                Destroy,
            };
        };

        // Entity command structure.
        struct EntityCommand
        {
            EntityCommands::Type type;
            EntityHandle handle;
        };

        // Type declarations.
        typedef std::queue<EntityCommand> CommandList;
        typedef std::vector<HandleEntry> HandleList;

    private:
        // Calculates handle index.
        int CalculateHandleIndex(const EntityHandle& handle) const;

        // Allocate an entity handle.
        void AllocateHandle();

        // Retrieves a free entity handle.
        HandleEntry& RetrieveHandle();

        // Creates an entity handle.
        void CreateHandle(const int handleIndex, HandleEntry& handEntry);

        // Destroys an entity handle.
        void DestroyHandle(const int handleIndex, HandleEntry& handEntry);

        // Frees an entity handle.
        void FreeHandle(const int handleIndex, HandleEntry& handEntry);

    private:
        // List of commands.
        CommandList m_commands;

        // List of entity handles.
        HandleList m_handles;

        // Number of active entities.
        int m_entityCount;

        // List of free handles.
        int  m_freeListDequeue;
        int  m_freeListEnqueue;
        bool m_freeListIsEmpty;

        // Initialization state.
        bool m_initialized;
    };
}
