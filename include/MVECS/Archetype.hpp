#ifndef MVECS_MVECS_ARCHETYPE_HPP_
#define MVECS_MVECS_ARCHETYPE_HPP_

#include <cassert>
#include <cstdint>
#include <limits>
#include <type_traits>

#include "IComponentData.hpp"
#include "TypeInfo.hpp"

namespace mvecs
{
    /**
     * @brief Chunkが持つ型(Entityが持つ型)の情報を定義するもの 基本的にコンパイル時処理
     *
     */
    class Archetype
    {
    public:
        //! Archetypeが持てる型情報の個数の限界
        static constexpr std::size_t MaxTypeNum = 16;

        /**
         * @brief Archetypeが同一かどうか判定する
         *
         * @param other 比較対象
         * @return true 同一
         * @return false 同一でない
         */
        constexpr bool operator==(const Archetype& other) const
        {
            if (mTypeCount != other.mTypeCount)
                return false;

            for (std::size_t i = 0; i < mTypeCount; ++i)
                if (mTypes[i] != other.mTypes[i])
                    return false;
            return true;
        }

        /**
         * @brief Archetypeが同一でないかどうか判定する
         *
         * @param other 比較対象
         * @return true 同一
         * @return false 同一でない
         */
        constexpr bool operator!=(const Archetype& other) const
        {
            return !(*this == other);
        }

        /**
         * @brief otherがthisの部分集合かどうか
         *
         * @param other 判定対象
         * @return true 部分集合
         * @return false 部分集合でない
         */
        constexpr bool isIn(const Archetype& other) const
        {
            if (mTypeCount < other.mTypeCount)
                return false;

            bool in = false;
            for (std::size_t i = 0; i < other.mTypeCount; ++i)
            {
                in = false;
                for (std::size_t j = 0; j < mTypeCount && other.mTypes[i].getHash() <= mTypes[j].getHash(); ++j)
                    if (other.mTypes[i] == mTypes[j])
                    {
                        in = true;
                        break;
                    }

                if (!in)
                    return false;
            }

            return true;
        }

        /**
         * @brief そのComponentData型を持つかどうか
         *
         * @tparam T ComponentData型
         * @tparam typename ComponentData型判定用
         * @return true 持つ
         * @return false 持たない
         */
        template <typename T, typename = std::enable_if_t<IsComponentDataType<T>>>
        constexpr bool isIn() const
        {
            std::uint32_t hash = T::getTypeHash();
            for (std::size_t i = 0; i < mTypeCount && hash <= mTypes[i].getHash(); ++i)
                if (mTypes[i].getHash() == hash)
                    return true;
            return false;
        }

        /**
         * @brief 指定したComponentData型の添字を取得する
         *
         * @tparam T ComponentData型
         * @tparam typename ComponentData型判定用
         * @return constexpr std::size_t その型のある添字
         */
        template <typename T, typename = std::enable_if_t<IsComponentDataType<T>>>
        constexpr std::size_t getTypeIndex() const
        {
            std::uint32_t hash = T::getTypeHash();
            for (std::size_t i = 0; i < mTypeCount && hash <= mTypes[i].getHash(); ++i)
                if (mTypes[i].getHash() == hash)
                    return i;
            assert(!"the type is not in this archetype!");
        }

        /**
         * @brief その型までのメモリ上のオフセットを取得する
         *
         * @param indexUntil 型の添字
         * @param coef 係数(単に各型のサイズに掛けられる)
         * @return constexpr std::size_t オフセット(バイト)
         */
        constexpr std::size_t getTypeOffset(std::size_t indexUntil, std::size_t coef = 1) const
        {
            assert(indexUntil < mTypeCount);
            std::size_t rtn = 0;
            for (std::size_t i = 0; i < indexUntil; ++i)
                rtn += coef * mTypes[i].getSize();
            return rtn;
        }

        /**
         * @brief 指定したComponentData型の添字を取得する(ハッシュ値版)
         *
         * @param hash ComponentData型のハッシュ
         * @return constexpr std::size_t その型のある添字
         */
        constexpr std::size_t getTypeIndex(std::size_t hash)
        {
            for (std::size_t i = 0; i < mTypeCount && hash <= mTypes[i].getHash(); ++i)
                if (mTypes[i].getHash() == hash)
                    return i;
            assert(!"the type is not in this archetype!");
            return std::numeric_limits<std::size_t>::max();
        }

        /**
         * @brief 指定したindexの型のハッシュ値を取得する
         *
         * @param index
         * @return constexpr std::size_t
         */
        constexpr std::size_t getTypeHash(std::size_t index)
        {
            assert(index < mTypeCount);
            return mTypes[index].getHash();
        }

        /**
         * @brief 型からArchetypeを構築する
         *
         * @tparam Args 構築する型、TypeInfo制約を満たすもの
         * @return constexpr Archetype 作成したインスタンス
         */
        template <typename... Args>
        static constexpr Archetype create()
        {
            Archetype rtn;
            rtn.createImpl<Args...>();

            // ソートする(降順)
            TypeInfo tmp;
            for (std::size_t i = 0; i < MaxTypeNum - 1; ++i)
                for (std::size_t j = i + 1; j < MaxTypeNum; ++j)
                    if (rtn.mTypes[i].getHash() < rtn.mTypes[j].getHash())
                    {
                        tmp           = rtn.mTypes[i];
                        rtn.mTypes[i] = rtn.mTypes[j];
                        rtn.mTypes[j] = tmp;
                    }

            return rtn;
        }

        /**
         * @brief [index]番目の型のサイズを取得する
         *
         * @param index 型の添字
         * @return constexpr std::size_t サイズ
         */
        constexpr std::size_t getTypeSize(std::size_t index) const
        {
            assert(index < mTypeCount);
            return mTypes[index].getSize();
        }

        /**
         * @brief 型の個数を取得する
         *
         * @return constexpr std::size_t 個数
         */
        constexpr std::size_t getTypeCount() const
        {
            return mTypeCount;
        }

        /**
         * @brief 全ての型のサイズの総和を取得する
         *
         * @return constexpr std::size_t サイズの総和
         */
        constexpr std::size_t getAllTypeSize() const
        {
            return mAllTypeSize;
        }

    private:
        /**
         * @brief create()の可変長テンプレート引数展開処理の実装部
         *
         * @tparam Head
         * @tparam Tails
         */
        template <typename Head, typename... Tails>
        constexpr void createImpl()
        {
            assert(IsComponentDataType<Head> || !"the type is not ComponentData!");

            mTypes[mTypeCount++] = TypeInfo::create<Head>();
            mAllTypeSize += sizeof(Head);
            assert(mTypeCount < MaxTypeNum || !"over max ComponentData type num!");

            if constexpr (sizeof...(Tails) != 0)
                createImpl<Tails...>();
        }

        //! 型情報(TypeInfo)の配列
        TypeInfo mTypes[MaxTypeNum];

        //! 現在の型の個数
        std::size_t mTypeCount = 0;
        //! 型サイズの総和を事前に計算しておく
        std::size_t mAllTypeSize = 0;
    };
}  // namespace mvecs

#endif