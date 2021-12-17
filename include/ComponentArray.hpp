#ifndef MVECS_COMPONENTARRAY_HPP_
#define MVECS_COMPONENTARRAY_HPP_

#include "IComponentData.hpp"
#include "TypeInfo.hpp"

#include <cassert>

namespace mvecs
{
    template<typename T>
    class ComponentArray
    {
    public:
        ComponentArray(T* address, std::size_t size)
         : mAddress(address)
         , mSize(size)
        {
            static_assert(IsComponentDataType<T>, "T is not ComponentData type");
            assert(address);
        }

        T& operator[](std::size_t index)
        {
            assert(index < mSize);
            return *(mAddress + index);
        }

        std::size_t size()
        {
            return mSize;
        }

    private:
        T* mAddress;
        std::size_t mSize;
    };
}

#endif