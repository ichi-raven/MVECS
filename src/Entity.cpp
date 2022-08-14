#include "../include/MVECS/Entity.hpp"

namespace mvecs
{
    Entity::Entity(std::size_t* pID, std::size_t chunkID)
        : mpID(pID)
        , mChunkID(chunkID)
    {
    }

    std::size_t Entity::getID() const
    {
        return *mpID;
    }

    std::size_t Entity::getChunkID() const
    {
        return mChunkID;
    }
}  // namespace mvecs