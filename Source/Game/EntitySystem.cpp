#include "Precompiled.hpp"
#include "EntitySystem.hpp"
using namespace Game;

namespace
{
    // Log message strings.
    #define LogInitializeError() "Failed to initialize the entity system! "

    // Constant variables.
    const int MaximumIdentifier   = std::numeric_limits<int>::max();
    const int InvalidIdentifier   = 0;
    const int InvalidNextFree     = -1;
    const int InvalidQueueElement = -1;
}

EntitySystem::EntitySystem() :
    m_entityCount(0),
    m_freeListDequeue(InvalidQueueElement),
    m_freeListEnqueue(InvalidQueueElement),
    m_freeListIsEmpty(true),
    m_initialized(false)
{
}

EntitySystem::~EntitySystem()
{
    this->Cleanup();
}

void EntitySystem::Cleanup()
{
    if(!m_initialized)
        return;

    // Destroy all remaining entities.
    this->DestroyAllEntities();

    // Cleanup event dispatchers.
    this->events.create.Cleanup();
    this->events.finalize.Cleanup();
    this->events.destroy.Cleanup();

    // Clear the command list.
    Assert(m_commands.empty(), "Destroying command list that is not empty!");
    Utility::ClearContainer(m_commands);

    // Clear the entity list.
    Assert(m_entityCount == 0, "Destroying handle list while there are active entities left!");
    Utility::ClearContainer(m_handles);

    // Reset the entity counter.
    m_entityCount = 0;

    // Reset the queue list of free handles.
    m_freeListDequeue = InvalidQueueElement;
    m_freeListEnqueue = InvalidQueueElement;
    m_freeListIsEmpty = true;

    // Reset initialization state.
    m_initialized = false;
}

bool EntitySystem::Initialize()
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

    // Preallocate entity handles.
    // TODO!

    // Success!
    return m_initialized = true;
}

EntityHandle EntitySystem::CreateEntity()
{
    if(!m_initialized)
        return EntityHandle();

    // Check if we reached the numerical limit.
    Verify(m_handles.size() < MaximumIdentifier, "Entity handle identifier reached its numerical limit!");

    // Create a new handle if the free list queue is empty.
    if(m_freeListIsEmpty)
    {
        Assert(m_freeListDequeue == InvalidQueueElement, "Free list queue is marked as empty but it's actually not!");
        Assert(m_freeListEnqueue == InvalidQueueElement, "Free list queue is marked as empty but it's actually not!");

        // Create an entity handle.
        EntityHandle handle;
        handle.m_identifier = m_handles.size() + 1;
        handle.m_version = 0;

        // Create a handle entry.
        HandleEntry entry;
        entry.handle = handle;
        entry.nextFree = InvalidNextFree;
        entry.flags = HandleFlags::Free;

        m_handles.push_back(entry);

        // Add new handle entry to the free list queue.
        int handleIndex = m_handles.size() - 1;

        m_freeListDequeue = handleIndex;
        m_freeListEnqueue = handleIndex;
        m_freeListIsEmpty = false;
    }

    // Retrieve an unused handle from the free list queue.
    int handleIndex = m_freeListDequeue;
    HandleEntry& handleEntry = m_handles[handleIndex];

    Assert(handleEntry.flags == HandleFlags::Free, "Retrieved handle is not marked as free!");

    // Remove the retrieved handle from the free list queue. 
    if(m_freeListDequeue == m_freeListEnqueue)
    {
        // Set the free list queue state to empty if it had only one element.
        m_freeListDequeue = InvalidQueueElement;
        m_freeListEnqueue = InvalidQueueElement;
        m_freeListIsEmpty = true;
    }
    else
    {
        // Set the beginning of the queue to the next free element if it had multiple elements.
        m_freeListDequeue = handleEntry.nextFree;
        handleEntry.nextFree = InvalidNextFree;
    }

    Assert(handleEntry.nextFree == InvalidNextFree, "Removed handle still refers to a next free handle!");

    // Mark handle as valid.
    handleEntry.flags |= HandleFlags::Valid;

    // Schedule entity to be created.
    EntityCommand command;
    command.type = EntityCommands::Create;
    command.handle = handleEntry.handle;

    m_commands.push(command);

    // Increment the counter of valid entities.
    m_entityCount += 1;

    // Returns a valid handle for an entity that will be created on the next ProcessCommands() call.
    return handleEntry.handle;
}

void EntitySystem::DestroyEntity(const EntityHandle& entity)
{
    if(!m_initialized)
        return;

    // Check if the handle is valid.
    if(!this->IsHandleValid(entity))
        return;

    // Retrieve the handle entry.
    int handleIndex = entity.m_identifier - 1;
    HandleEntry& handleEntry = m_handles[handleIndex];

    Assert(handleEntry.flags & HandleFlags::Valid, "Attempting to destroy an entity that is not valid!");
    Assert(!(handleEntry.flags & HandleFlags::Destroy), "Attempting to destroy an entity that is already being destroyed!");

    // Mark handle to be destroyed.
    handleEntry.flags |= HandleFlags::Destroy;

    // Schedule entity to be destroyed.
    EntityCommand command;
    command.type = EntityCommands::Destroy;
    command.handle = handleEntry.handle;

    m_commands.push(command);
}

void EntitySystem::DestroyAllEntities()
{
    if(!m_initialized)
        return;

    // Process remaining entity commands.
    this->ProcessCommands();

    // Check if there are any entities to destroy.
    if(m_handles.empty())
        return;

    // Inform about entities soon to be destroyed.
    for(auto it = m_handles.begin(); it != m_handles.end(); ++it)
    {
        HandleEntry& handleEntry = *it;

        if(handleEntry.flags & HandleFlags::Valid)
        {
            // Send event about soon to be destroyed entity.
            this->events.destroy({ handleEntry.handle });

            // Set handle free flags.
            handleEntry.flags = HandleFlags::Free;

            // Increment handle version to invalidate it.
            handleEntry.handle.m_version += 1;

            // Decrement the counter of active entities.
            //m_entityCount -= 1;
        }
    }

    // Chain handles into a free list queue.
    for(std::size_t i = 0; i < m_handles.size(); ++i)
    {
        HandleEntry& handleEntry = m_handles[i];
        handleEntry.nextFree = i + 1;
    }

    // Close the free list queue chain at the end.
    int lastHandleIndex = m_handles.size() - 1;
    m_handles[lastHandleIndex].nextFree = InvalidNextFree;

    // Set the free list queue variables.
    m_freeListDequeue = 0;
    m_freeListEnqueue = lastHandleIndex;
    m_freeListIsEmpty = false;
}

void EntitySystem::ProcessCommands()
{
    if(!m_initialized)
        return;

    // Process entity commands.
    while(!m_commands.empty())
    {
        // Get the command from the queue.
        EntityCommand& command = m_commands.front();

        // Process entity command.
        switch(command.type)
        {
        case EntityCommands::Create:
            {
                // Locate the handle entry.
                int handleIndex = command.handle.m_identifier - 1;
                HandleEntry& handleEntry = m_handles[handleIndex];

                Assert(command.handle == handleEntry.handle, "Attempting to create a non existing entity!");

                // Inform that we want this entity finalized.
                if(!this->events.finalize({ handleEntry.handle }))
                {
                    // Destroy if entity failed to finalize.
                    this->DestroyEntity(handleEntry.handle);
                    break;
                }

                // Mark handle as active.
                Assert(!(handleEntry.flags & HandleFlags::Active), "Created entity is already active!");

                handleEntry.flags |= HandleFlags::Active;

                // Inform about a created entity.
                this->events.create({ handleEntry.handle });
            }
            break;

        case EntityCommands::Destroy:
            {
                // Locate the handle entry.
                int handleIndex = command.handle.m_identifier - 1;
                HandleEntry& handleEntry = m_handles[handleIndex];

                Assert(command.handle == handleEntry.handle, "Attempting to destroy a non existing entity!");

                // Inform about a destroyed entity.
                this->events.destroy({ handleEntry.handle });

                // Free entity handle.
                this->FreeHandle(handleIndex, handleEntry);
            }
            break;
        }

        // Remove command from the queue.
        m_commands.pop();
    }
}

void EntitySystem::FreeHandle(const int handleIndex, HandleEntry& handleEntry)
{
    Assert(m_initialized, "Entity system is not initialized!");

    // Make sure we got the matching index.
    Assert(handleIndex >= 0 && (std::size_t)handleIndex < m_handles.size(), "Invalid handle index!");
    Assert(&m_handles[handleIndex] == &handleEntry, "Missmatch between passed handle index and entry!");

    // Mark handle as free.
    Assert(!(handleEntry.flags & HandleFlags::Free), "Attempting to free a handle that is already free!");
    Assert(handleEntry.flags & HandleFlags::Valid, "Attempting to free a handle that is not valid!");
    Assert(handleEntry.flags & HandleFlags::Destroy, "Attempting to free a handle that is not being destroyed!");

    handleEntry.flags = HandleFlags::Free;

    // Increment handle version to invalidate it.
    handleEntry.handle.m_version += 1;

    // Add handle entry to the free list queue.
    if(m_freeListIsEmpty)
    {
        // Add a free handle to empty free list queue.
        m_freeListDequeue = handleIndex;
        m_freeListEnqueue = handleIndex;
        m_freeListIsEmpty = false;
    }
    else
    {
        // Check if the end of the free list queue is valid.
        Assert(m_handles[m_freeListEnqueue].nextFree == InvalidNextFree, "Last element in the free list queue is pointing at a next free handle!");
        Assert(handleEntry.nextFree == InvalidNextFree, "Fried handle entry is poiting to a next free handle!");

        // Add a free handle to occupied free list queue.
        m_handles[m_freeListEnqueue].nextFree = handleIndex;
        m_freeListEnqueue = handleIndex;
    }

    // Decrement the counter of valid entities.
    m_entityCount -= 1;
}

bool EntitySystem::IsHandleValid(const EntityHandle& entity) const
{
    if(!m_initialized)
        return false;

    // Check if the handle identifier is valid.
    if(entity.m_identifier == InvalidIdentifier)
        return false;

    Assert(entity.m_identifier > InvalidIdentifier, "Corrupted entity handle identifier encountered!");
    Assert(entity.m_identifier <= (int)m_handles.size(), "Corrupted entity handle identifier encountered!")

    // Retrieve the handle entry.
    int handleIndex = entity.m_identifier - 1;
    const HandleEntry& handleEntry = m_handles[handleIndex];

    // Check if handle entry is valid.
    if(!(handleEntry.flags & HandleFlags::Valid))
        return false;

    // Check if handle is scheduled to be destroyed.
    if(handleEntry.flags & HandleFlags::Destroy)
        return false;

    // Check if handle versions match.
    if(handleEntry.handle.m_version != entity.m_version)
        return false;

    return true;
}

int EntitySystem::GetEntityCount() const
{
    return m_entityCount;
}
