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
        //! iterator型の定義
        using iterator         = T*;
        //! const_iterator型の定義
        using const_iterator   = const T*;
        //! reverse_iterator型の定義
        using reverse_iterator = std::reverse_iterator<iterator>;

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

        /**
         * @brief 先頭イテレータを取得する
         * 
         * @return iterator 
         */
        iterator begin() noexcept
        {
            assert(mAddress);
            return mAddress;
        }

        /**
         * @brief 終端イテレータを取得する
         * 
         * @return iterator 
         */
        iterator end() noexcept
        {
            assert(mAddress);
            return mAddress + mSize;
        }

        /**
         * @brief const先頭イテレータを取得する
         * 
         * @return const_iterator 
         */
        const_iterator cbegin() const noexcept
        {
            assert(mAddress);
            return mAddress;
        }

        /**
         * @brief const終端イテレータを取得する
         * 
         * @return const_iterator 
         */
        const_iterator cend() const noexcept
        {
            assert(mAddress);
            return mAddress + mSize;
        }

        /**
         * @brief 逆先頭イテレータを取得する
         * 
         * @return reverse_iterator 
         */
        reverse_iterator rbegin() noexcept
        {
            assert(mAddress);
            return reverse_iterator{mAddress + mSize};
        }

        /**
         * @brief 逆終端イテレータを取得する
         * 
         * @return reverse_iterator 
         */
        reverse_iterator rend() noexcept
        {
            assert(mAddress);
            return reverse_iterator{mAddress};
        }

    private:
        //! 開始アドレス
        T* mAddress;
        //! 要素数
        std::size_t mSize;
    };
}  // namespace mvecs

#endif