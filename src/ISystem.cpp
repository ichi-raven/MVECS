#include "../include/ISystem.hpp"

namespace mvecs
{
    ISystem::ISystem(const std::shared_ptr<World>& pWorld)
    {
        mpWorld = pWorld;
    }
}