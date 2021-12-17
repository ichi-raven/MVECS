#include "../include/World.hpp"

#include <limits>

#include "../include/ISystem.hpp"

namespace mvecs
{
    World::World()
    {
    }

    World::~World()
    {
        for (auto& system : mSystems)
            system->onEnd();
    }

    void World::startSystemImpl(std::size_t index)
    {
        mSystems[index]->onStart();
    }

    void World::clearSystem()
    {
        for (auto& system : mSystems)
            system->onEnd();
        mSystems.clear();
    }

    void World::update()
    {
        for (auto& system : mSystems)
            system->onUpdate();
    }

    std::size_t World::genChunkID()
    {
        static std::size_t chunkID = 0;
        return chunkID++;
    }

    void World::destroyEntity(const Entity& entity)
    {
        mChunks[searchChunk(entity.getChunkID())].deallocate(entity);
    }

    // inline std::size_t World::searchChunk(std::size_t ID)
    // {
    //     for (std::size_t i = 0; i < mChunks.size(); ++i)
    //         if (mChunks[i].getID() == ID)
    //             return i;

    //     return std::numeric_limits<std::size_t>::max();
    // }
}  // namespace mvecs