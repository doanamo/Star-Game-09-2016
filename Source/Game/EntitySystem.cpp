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

    // Check the state before cleaning up.
    Assert(m_commands.empty(), "Cleaning up the entity system while there are unprocessed command left!");
    Assert(m_entityCount == 0, "Cleaning up the entity system while there are alive entities left!");

    // Cleanup event dispatchers.
    this->events.finalize.Cleanup();
    this->events.create.Cleanup();
    this->events.destroy.Cleanup();

    // Clear the command list.
    Utility::ClearContainer(m_commands);

    // Clear the entity list.
    Utility::ClearContainer(m_handles);

    // Reset the entity counter.
    m_entityCount = 0;

    // Reset the queue list of free handles.
    m_freeListDequeue = InvalidQueueElement;
    m_freeListEnqueue = InvalidQueueElement;
    m_freeListIsEmpty = true;

    // Reset the initialization state.
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
    // TODO: Implement me!

    // Success!
    return m_initialized = true;
}

EntityHandle EntitySystem::CreateEntity()
{
    if(!m_initialized)
        return EntityHandle();

    // Check if we reached the numerical limit.
    Verify(m_handles.size() < MaximumIdentifier, "Entity handle identifier reached its numerical limit!");

    // Retrieve a free handle.
    HandleEntry& handleEntry = this->RetrieveHandle();

    // Mark the retrieved handle as valid.
    handleEntry.flags |= HandleFlags::Valid;

    // Schedule an entity to be created.
    EntityCommand command;
    command.type = EntityCommands::Create;
    command.handle = handleEntry.handle;

    m_commands.push(command);

    // Return a valid handle.
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
    int handleIndex = this->CalculateHandleIndex(entity);
    HandleEntry& handleEntry = m_handles[handleIndex];

    Assert(handleEntry.flags & HandleFlags::Valid, "Attempting to destroy an entity that is not valid!");
    Assert(!(handleEntry.flags & HandleFlags::Destroy), "Attempting to destroy an entity that is already being destroyed!");

    // Mark the handle to be destroyed.
    handleEntry.flags |= HandleFlags::Destroy;

    // Schedule the entity to be destroyed.
    EntityCommand command;
    command.type = EntityCommands::Destroy;
    command.handle = handleEntry.handle;

    m_commands.push(command);
}

void EntitySystem::DestroyAllEntities()
{
    if(!m_initialized)
        return;

    // Process entity commands.
    this->ProcessCommands();

    // Destroy all remaining entities.
    for(auto it = m_handles.begin(); it != m_handles.end(); ++it)
    {
        // Get the handle entry.
        HandleEntry& handleEntry = *it;

        // Calculate the handle index.
        int handleIndex = this->CalculateHandleIndex(handleEntry.handle);

        // Destroy entities that are still valid.
        if(handleEntry.flags & HandleFlags::Valid)
        {
            this->DestroyHandle(handleIndex, handleEntry);
        }
    }
}

void EntitySystem::ProcessCommands()
{
    if(!m_initialized)
        return;

    // Process entity commands.
    while(!m_commands.empty())
    {
        // Get the first command in the queue.
        EntityCommand& command = m_commands.front();

        // Process the entity command.
        switch(command.type)
        {
        case EntityCommands::Create:
            {
                // Locate the handle entry.
                int handleIndex = this->CalculateHandleIndex(command.handle);
                HandleEntry& handleEntry = m_handles[handleIndex];

                // Check if the entity handle matches the handle entry.
                Assert(command.handle == handleEntry.handle, "Attempting to create a non existing entity!");

                // Create an entity.
                this->CreateHandle(handleIndex, handleEntry);
            }
            break;

        case EntityCommands::Destroy:
            {
                // Locate the handle entry.
                int handleIndex = this->CalculateHandleIndex(command.handle);
                HandleEntry& handleEntry = m_handles[handleIndex];

                // Check if the entity handle matches the handle entry.
                Assert(command.handle == handleEntry.handle, "Attempting to destroy a non existing entity!");

                // Destroy an entity.
                this->DestroyHandle(handleIndex, handleEntry);
            }
            break;
        }

        // Remove the processed command from the queue.
        m_commands.pop();
    }
}

int EntitySystem::GetEntityCount() const
{
    return m_entityCount;
}

int EntitySystem::CalculateHandleIndex(const EntityHandle& handle) const
{
    // Return the index of the handle entry that corresponds to this entity handle.
    return handle.m_identifier - 1;
}

void EntitySystem::AllocateHandle()
{
    Assert(m_initialized, "Entity system is not initialized!");
    
    // Create an entity handle.
    EntityHandle handle;
    handle.m_identifier = m_handles.size() + 1;
    handle.m_version = 0;

    // Create a handle entry.
    HandleEntry entry;
    entry.handle = handle;
    entry.flags = HandleFlags::Free;
    entry.nextFree = InvalidNextFree;

    m_handles.push_back(entry);

    // Add the created handle entry to the free list queue.
    int handleIndex = this->CalculateHandleIndex(handle);

    if(m_freeListIsEmpty)
    {
        // Add the handle as the only element of the queue.
        m_freeListDequeue = handleIndex;
        m_freeListEnqueue = handleIndex;
        m_freeListIsEmpty = false;
    }
    else
    {
        // Add the handle at the end of the queue.
        m_handles[m_freeListEnqueue].nextFree = handleIndex;
        m_freeListEnqueue = handleIndex;
    }
}

EntitySystem::HandleEntry& EntitySystem::RetrieveHandle()
{
    Assert(m_initialized, "Entity system is not initialized!");

    // Allocate a handle if the free list queue is empty.
    // TODO: Add a stable minimum amount of free list queue elements.
    if(m_freeListIsEmpty)
    {
        Assert(m_freeListDequeue == InvalidQueueElement, "Free list dequeue is not invalid while the list is not empty!");
        Assert(m_freeListEnqueue == InvalidQueueElement, "Free list enqueue is not invalid while the list is not empty!");

        this->AllocateHandle();

        Assert(m_freeListDequeue != InvalidQueueElement, "Free list dequeue is invalid after allocating a handle!");
        Assert(m_freeListEnqueue != InvalidQueueElement, "Free list enqueue is invalid after allocating a handle!");
        Assert(!m_freeListIsEmpty, "Free list queue is empty after allocating a handle!");
    }

    // Retrieve an unused handle from the free list queue.
    int handleIndex = m_freeListDequeue;
    HandleEntry& handleEntry = m_handles[handleIndex];

    Assert(handleEntry.flags == HandleFlags::Free, "Retrieved handle is not marked as free!");

    // Remove the retrieved handle from the free list queue. 
    if(m_freeListDequeue == m_freeListEnqueue)
    {
        // Remove the handle as the only element of the queue.
        m_freeListDequeue = InvalidQueueElement;
        m_freeListEnqueue = InvalidQueueElement;
        m_freeListIsEmpty = true;
    }
    else
    {
        // Remove the handle from the beginning of the queue.
        m_freeListDequeue = handleEntry.nextFree;
        handleEntry.nextFree = InvalidNextFree;
    }

    return handleEntry;
}

void EntitySystem::CreateHandle(const int handleIndex, HandleEntry& handleEntry)
{
    Assert(m_initialized, "Entity system is not initialized!");

    // Make sure we got the matching index.
    Assert(handleIndex >= 0 && (std::size_t)handleIndex < m_handles.size(), "Invalid handle index!");
    Assert(&m_handles[handleIndex] == &handleEntry, "Missmatch between passed handle index and entry!");

    // Check handle flags.
    Assert(!(handleEntry.flags & HandleFlags::Active), "Attempting to create a handle that is already active!");
    Assert(handleEntry.flags & HandleFlags::Valid, "Attemping to create a handle that is not valid!");

    // Increment the counter of active entities.
    m_entityCount += 1;

    // Inform that we want this entity finalized.
    if(this->events.finalize.HasSubscribers())
    {
        if(!this->events.finalize({ handleEntry.handle }))
        {
            // Destroy the entity handle if finalization fails.
            this->DestroyHandle(handleIndex, handleEntry);
            return;
        }
    }

    // Mark the handle as active.
    handleEntry.flags |= HandleFlags::Active;

    // Inform about a created entity.
    this->events.create({ handleEntry.handle });
}

void EntitySystem::DestroyHandle(const int handleIndex, HandleEntry& handleEntry)
{
    Assert(m_initialized, "Entity system is not initialized!");

    // Make sure we got the matching index.
    Assert(handleIndex >= 0 && (std::size_t)handleIndex < m_handles.size(), "Invalid handle index!");
    Assert(&m_handles[handleIndex] == &handleEntry, "Missmatch between passed handle index and entry!");

    // Inform about a destroyed entity.
    this->events.destroy({ handleEntry.handle });

    // Free entity handle.
    this->FreeHandle(handleIndex, handleEntry);

    // Decrement the counter of active entities.
    m_entityCount -= 1;
}

void EntitySystem::FreeHandle(const int handleIndex, HandleEntry& handleEntry)
{
    Assert(m_initialized, "Entity system is not initialized!");

    // Make sure we got the matching index.
    Assert(handleIndex >= 0 && (std::size_t)handleIndex < m_handles.size(), "Invalid handle index!");
    Assert(&m_handles[handleIndex] == &handleEntry, "Missmatch between passed handle index and entry!");

    // Mark the handle as free.
    Assert(!(handleEntry.flags & HandleFlags::Free), "Attempting to free a handle that is already free!");
    Assert(handleEntry.flags & HandleFlags::Valid, "Attempting to free a handle that is not valid!");

    handleEntry.flags = HandleFlags::Free;

    // Increment the handle version to invalidate it.
    handleEntry.handle.m_version += 1;

    // Add the handle entry to the free list queue.
    if(m_freeListIsEmpty)
    {
        // Add the handle as the only element of the queue.
        m_freeListDequeue = handleIndex;
        m_freeListEnqueue = handleIndex;
        m_freeListIsEmpty = false;
    }
    else
    {
        // Check if the end of the free list queue is valid.
        Assert(m_handles[m_freeListEnqueue].nextFree == InvalidNextFree, "Last element in the free list queue is pointing at a next free handle!");
        Assert(handleEntry.nextFree == InvalidNextFree, "Fried handle entry is poiting to a next free handle!");

        // Add the handle at the end of the queue.
        m_handles[m_freeListEnqueue].nextFree = handleIndex;
        m_freeListEnqueue = handleIndex;
    }
}

bool EntitySystem::IsHandleValid(const EntityHandle& entity) const
{
    if(!m_initialized)
        return false;

    // Check if the handle identifier is valid.
    if(entity.m_identifier == InvalidIdentifier)
        return false;

    Assert(entity.m_identifier > InvalidIdentifier, "Corrupted entity handle identifier encountered!");
    Assert(entity.m_identifier <= (int)m_handles.size(), "Corrupted entity handle identifier encountered!");

    // Retrieve the handle entry.
    int handleIndex = this->CalculateHandleIndex(entity);
    const HandleEntry& handleEntry = m_handles[handleIndex];

    // Check if the handle entry is valid.
    if(!(handleEntry.flags & HandleFlags::Valid))
        return false;

    // Check if the handle is scheduled to be destroyed.
    if(handleEntry.flags & HandleFlags::Destroy)
        return false;

    // Check if handle versions match.
    if(handleEntry.handle.m_version != entity.m_version)
        return false;

    return true;
}
