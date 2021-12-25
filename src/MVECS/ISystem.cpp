#include "../include/MVECS/ISystem.hpp"

namespace mvecs
{
    ISystem::ISystem(World* const pWorld, const int executionOrder)
        : mpWorld(pWorld)
        , mExecutionOrder(executionOrder)
    {
        
    }

    int ISystem::getExecutionOrder() const
    {
        return mExecutionOrder;
    }
}  // namespace mvecs