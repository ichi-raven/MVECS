#pragma once
#ifndef MVECS_MVECS_ICHUNK_HPP_
#define MVECS_MVECS_ICHUNK_HPP_

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <memory>
#include <vector>

#include "Archetype.hpp"
#include "ComponentArray.hpp"
#include "Entity.hpp"

/**
 * @brief mvecs
 *
 */
namespace mvecs
{
    /**
     * @brief Chunkクラスのインタフェース ComponentDataの
     *
     */
    class IChunk
    {
    public:

        /**
         * @brief コンストラクタ
         *
         * @param ID
         */
        IChunk(const std::size_t ID, const Archetype& archetype);

        /**
         * @brief デストラクタ
         *
         */
        virtual ~IChunk();

        /**
         * @brief コピーコンストラクタはdelete
         *
         * @param src
         */
        IChunk(IChunk& src) = delete;

        /**
         * @brief 代入によるコピーもdelete
         *
         * @param src
         * @return IChunk&
         */
        IChunk& operator=(const IChunk& src) = delete;

        /**
         * @brief ムーブコンストラクタ
         *
         * @param src ムーブ元
         */
        IChunk(IChunk&& src) noexcept;

        /**
         * @brief ムーブコンストラクタ 演算子オーバーロード
         *
         * @param src
         * @return IChunk&
         */
        IChunk& operator=(IChunk&& src) noexcept;

        /**
         * @brief ChunkのIDを取得する
         *
         * @return std::size_t ID
         */
        std::size_t getID() const;

        /**
         * @brief Archetypeを取得する
         *
         * @return Archetype
         */
        const Archetype& getArchetype() const;

        /**
         * @brief Entityの領域を確保する
         * @details 実際の値は何も書き込まれていないので注意
         * @return Entity
         */
        virtual Entity allocate() = 0;

        /**
         * @brief 指定したEntityを削除してメモリを詰める
         *
         * @param entity 削除するEntity
         */
        virtual void deallocate(const Entity& entity) = 0;

        /**
         * @brief メモリ全体をクリアし、size=1にする
         *
         */
        void clear();

        /**
         * @brief 完全にChunkを破棄する(メモリを全て解放する)
         *
         */
        virtual void destroy() = 0;

        /**
         * @brief entityをotherのChunkに移動する
         *
         * @param other 移動先Chunk
         */
        //Entity moveTo(const Entity& entity, Chunk& other);

        /**
         * @brief ComponentDataの値を書き込む
         *
         * @tparam T ComponentDataの型
         * @param entity 書き込み先Entity
         * @param value 書き込むComponentData
         */
        //template <typename T, typename = std::enable_if_t<IsComponentDataType<T>>>
        //void setComponentData(const Entity& entity, const T& value)
        //{
        //    assert(mArchetype.isIn<T>() || !"T is not in Archetype");
        //    assert(entity.getID() <= mEntityNum || !"invalid entity ID!");

        //    // 書き込む型までのオフセット
        //    const std::size_t offset = mArchetype.getTypeOffset(mArchetype.getTypeIndex<T>(), mMaxEntityNum);

        //    // 書き込み
        //    std::memcpy(mpMemory + offset + entity.getID() * sizeof(T), &value, sizeof(T));
        //}

        /**
         * @brief  ComponentDataの値を取得する
         *
         * @tparam T 取得するComponentDataの型
         * @tparam typename ComponentData型か判定する
         * @param entity 取得先のEntity
         * @return 取得した値
         */
        template <typename T, typename = std::enable_if_t<IsComponentDataType<T>>>
        T& getComponentData(const Entity& entity) const
        {
            assert(mArchetype.isIn<T>() || !"T is not in Archetype");

            // 書き込む型までのオフセット
            std::size_t offset = mArchetype.getTypeOffset(mArchetype.getTypeIndex<T>(), mMaxEntityNum);

            // 読み出し
            return *(reinterpret_cast<T*>(mpMemory + offset + entity.getID() * sizeof(T)));
        }

        /**
         * @brief Entityが増えたらメモリを割り当て直す
         *
         * @param maxEntityNum 新しい最大Entity数
         */
        virtual void reallocate(const std::size_t maxEntityNum) = 0;

        /**
         * @brief 指定した型のComponentArrayを取得する
         * @details 渡されたアドレスは無効になる可能性があるため操作には注意する
         * @tparam T ComponentDataの型
         * @tparam typename ComponentData型判定用
         * @return ComponentArray<T>
         */
        template <typename T, typename = std::enable_if_t<IsComponentDataType<T>>>
        ComponentArray<T> getComponentArray() const
        {
            assert(mArchetype.isIn<T>() || !"T is not in Archetype");

            // 使用する型までのオフセット
            std::size_t offset = mArchetype.getTypeOffset(mArchetype.getTypeIndex<T>(), mMaxEntityNum);

            return ComponentArray<T>(reinterpret_cast<T*>(mpMemory + offset), mEntityNum);
        }

        /**
         * @brief 現在のEntityの個数を取得する
         *
         * @return std::size_t 個数
         */
        std::size_t getEntityNum() const;

        /**
         * @brief メモリダンプ
         *
         */
        void dumpMemory() const;

        /**
         * @brief EntityのIDと実際のメモリを対応づけている部分をダンプする
         *
         */
        void dumpIndexMemory() const;

    protected:

        //void insertEntityIndex(std::size_t* pIndex);

        //! ChunkのID
        std::size_t mID;

        //! もととなるArchetype
        Archetype mArchetype;
        //! メモリアドレス
        std::byte* mpMemory;
        //! 割り当てられる最大のEntity数
        std::size_t mMaxEntityNum;
        //! 現在のEntity数
        std::size_t mEntityNum;

        //!  割り当てたEntityたちのIDアドレス(destroyに応じて書き換える)
        std::vector<std::size_t*> mpEntityIDs;
    };
}  // namespace mvecs

#endif