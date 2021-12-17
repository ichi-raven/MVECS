#ifndef MVECS_ISYSTEM_HPP_
#define MVECS_ISYSTEM_HPP_

#include "World.hpp"

#include <functional>
#include <memory>


namespace mvecs
{
    class ISystem
    {
    public:
        ISystem(const std::shared_ptr<World>& pWorld);
        virtual void onStart() = 0;
        virtual void onUpdate() = 0;
        virtual void onEnd() = 0;

        template<typename T>
        void forEach(std::function<void(T&)> func)
        {
            mpWorld->forEach<T>(func);
        }

        template<typename... Args>
        void forEach(std::function<void(const Args&...)>)
        {

        }

    protected:
        std::shared_ptr<World> mpWorld;
    };

    //template<typename T>
    //constexpr bool IsSystemType = std::is_base_of<ISystem, T>::value;
}

#endif