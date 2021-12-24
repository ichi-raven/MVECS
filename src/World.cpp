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
            // system.second->onEnd();
            system->onEnd();
        mSystems.clear();
        mChunks.clear();
    }

    void World::startSystemImpl(const std::unique_ptr<ISystem>& system)
    {
        system->onStart();
    }

    void World::clearSystem()
    {
        for (auto& system : mSystems)
            // system.second->onEnd();
            system->onEnd();
        mSystems.clear();
    }

    // void World::removeSystem(std::size_t systemID)
    // {
    //     mSystems.erase(systemID);
    // }

    void World::update()
    {
        for (auto& system : mSystems)
            // system.second->onUpdate();
            system->onUpdate();
    }

    std::size_t World::genChunkID()
    {
        static std::size_t chunkID = 0;
        return chunkID++;
    }

    std::size_t World::genSystemID()
    {
        static std::size_t systemID = 0;
        return systemID++;
    }

    void World::destroyEntity(const Entity& entity)
    {
        mChunks[findChunk(entity.getChunkID())].deallocate(entity);
        // mChunks.find(entity.getChunkID())->second.deallocate(entity);
    }

    Chunk& World::insertChunk(Chunk&& chunk)
    {
        //mChunks.emplace_back(std::move(chunk));
        //return mChunks.back();

        auto itr = std::lower_bound(mChunks.begin(), mChunks.end(), chunk, [](const Chunk& left, const Chunk& right)
                                    { return left.getID() < right.getID(); });
        if (itr == mChunks.end())
        {
            mChunks.emplace_back(std::move(chunk));
            itr = --mChunks.end();
        }
        else
            mChunks.insert(itr, std::move(chunk));
        return *itr;
    }

    std::size_t World::findChunk(std::size_t chunkID)
    {
        // for (std::size_t i = 0; i < mChunks.size(); ++i)
        //     if (mChunks[i].getID() == chunkID)
        //         return i;

        auto lower = mChunks.begin();
        auto upper = mChunks.end() - 1;
        while (lower <= upper)
        {
            auto mid = lower + (upper - lower) / 2;
            if (chunkID == mid->getID())
                return std::distance(mChunks.begin(), mid);
            else if (chunkID < mid->getID())
                upper = mid - 1;
            else
                lower = mid + 1;
        }

        assert(!"chunk was not found!");
        return std::numeric_limits<std::size_t>::max();
    }

    std::unique_ptr<ISystem>& World::insertSystem(std::unique_ptr<ISystem>&& system)
    {
         auto itr = std::lower_bound(mSystems.begin(), mSystems.end(), system, [](const std::unique_ptr<ISystem>& left, const std::unique_ptr<ISystem>& right)
                                    { return left->getExecutionOrder() < right->getExecutionOrder(); });
        if (itr == mSystems.end())
        {
            mSystems.emplace_back(std::move(system));
            itr = --mSystems.end();
        }
        else
            mSystems.insert(itr, std::move(system));
        return *itr;
    }


    // inline std::size_t World::searchChunk(std::size_t ID)
    // {
    //     for (std::size_t i = 0; i < mChunks.size(); ++i)
    //         if (mChunks[i].getID() == ID)
    //             return i;

    //     return std::numeric_limits<std::size_t>::max();
    // }
}  // namespace mvecs