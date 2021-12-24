#ifndef MVECS_COMPONENTARRAY_HPP_
#define MVECS_COMPONENTARRAY_HPP_

#include <cassert>
#include <iostream>

#include "IComponentData.hpp"
#include "TypeInfo.hpp"

namespace mvecs
{
    /**
     * @brief Chunk上のComponentData型のアドレスを配列のように見せる
     * 
     * @tparam T ComponentData型
     * @tparam typename ComponentData型判定用
     */
    template <typename T, typename = std::enable_if_t<IsComponentDataType<T>>>
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
}  // namespace mvecs

#endif