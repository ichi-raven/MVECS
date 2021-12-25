#ifndef MVECS_MVECS_COMPONENTARRAY_HPP_
#define MVECS_MVECS_COMPONENTARRAY_HPP_

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
        /**
         * @brief コンストラクタ
         *
         * @param address ComponentArrayを構築するアドレス
         * @param size 配列の要素数
         */
        ComponentArray(T* address, std::size_t size)
            : mAddress(address)
            , mSize(size)
        {
            static_assert(IsComponentDataType<T>, "T is not ComponentData type");
            assert(address);
        }

        /**
         * @brief []のオーバーロード(添字アクセスできるようにする)
         *
         * @param index 添字
         * @return T&
         */
        T& operator[](const std::size_t index)
        {
            assert(index < mSize);
            return *(mAddress + index);
        }

        /**
         * @brief 要素数を取得する
         *
         * @return std::size_t 要素数
         */
        std::size_t size()
        {
            return mSize;
        }

    private:
        //! 開始アドレス
        T* mAddress;
        //! 要素数
        std::size_t mSize;
    };
}  // namespace mvecs

#endif