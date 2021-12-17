#include "../include/Entity.hpp"

namespace mvecs
{
    Entity::Entity(std::size_t ID, std::size_t chunkID)
        : mID(ID)
        , mChunkID(chunkID)
    {
    }

    std::size_t Entity::getID() const
    {
        return mID;
    }

    std::size_t Entity::getChunkID() const
    {
        return mChunkID;
    }
}  // namespace mvecs