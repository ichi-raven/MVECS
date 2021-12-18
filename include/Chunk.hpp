#ifndef MVECS_CHUNK_HPP_
#define MVECS_CHUNK_HPP_

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <memory>

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
     * @brief Chunkクラス Archetypeから作られる実際のメモリ上のデータ
     *
     */
    class Chunk
    {
    public:
        /**
         * @brief 与えられたArchetypeからChunkを構築する
         *
         * @param entitySize Chunkが持つEntityの最大数
         * @return Chunk 構築したChunk
         */
        static Chunk create(std::size_t ID, const Archetype& archetype, const std::size_t maxEntityNum = 1)
        {
            Chunk rtn(ID, archetype);
            rtn.mMemory       = std::unique_ptr<std::byte>(new std::byte[archetype.getAllTypeSize() * maxEntityNum + sizeof(std::size_t) * maxEntityNum]);
            rtn.mMaxEntityNum = maxEntityNum;
            assert(rtn.mMaxEntityNum != 0);

            // ID-index部のメモリクリア
            std::memset(rtn.mMemory.get() + archetype.getAllTypeSize() * maxEntityNum, static_cast<int>(std::numeric_limits<std::size_t>::max()), maxEntityNum);

            return rtn;
        }

        /**
         * @brief コンストラクタ
         *
         * @param ID
         */
        Chunk(std::size_t ID, Archetype archetype);

        /**
         * @brief ChunkのIDを取得する
         *
         * @return std::size_t
         */
        std::size_t getID() const;

        /**
         * @brief Archetypeを取得する
         *
         * @return constexpr Archetype
         */
        const Archetype& getArchetype() const;

        /**
         * @brief Entityの領域を確保する
         * @details 実際の値は何も書き込まれていないので注意
         * @return Entity
         */
        Entity allocate();

        /**
         * @brief 指定したEntityを削除してメモリを詰める
         *
         * @param entity 削除するEntity
         */
        void deallocate(const Entity& entity);

        /**
         * @brief entityをotherのChunkに移動する
         *
         * @param other 移動先Chunk
         */
        Entity moveTo(const Entity& entity, Chunk& other);

        /**
         * @brief ComponentDataの値を書き込む
         *
         * @tparam T ComponentDataの型
         * @param entity 書き込み先Entity
         * @param value 書き込むComponentData
         */
        template <typename T, typename = std::enable_if_t<IsComponentDataType<T>>>
        void setComponentData(const Entity& entity, const T& value)
        {
            assert(mArchetype.isIn<T>() || !"T is not in Archetype");
            // ID-indexテーブル
            const std::size_t* memToIndexPart = reinterpret_cast<std::size_t*>(mMemory.get() + mArchetype.getAllTypeSize() * mMaxEntityNum);
            const std::size_t index           = memToIndexPart[entity.getID()];

            assert(index != std::numeric_limits<size_t>::max() || !"this entity was not allocated!");

            // 書き込む型までのオフセット
            std::size_t offset = mArchetype.getTypeOffset(mArchetype.getTypeIndex<T>(), mMaxEntityNum);

            // 書き込み
            std::memcpy(mMemory.get() + offset + index * sizeof(T), &value, sizeof(T));
        }

        /**
         * @brief  ComponentDataの値を取得する
         *
         * @tparam T 取得するComponentDataの型
         * @tparam typename ComponentData型か判定する
         * @param entity 取得先のEntity
         * @return 取得した値
         */
        template <typename T, typename = std::enable_if_t<IsComponentDataType<T>>>
        T getComponentData(const Entity& entity) const
        {
            assert(mArchetype.isIn<T>() || !"T is not in Archetype");
            const std::size_t* memToIndexPart = reinterpret_cast<std::size_t*>(mMemory.get() + mArchetype.getAllTypeSize() * mMaxEntityNum);
            const std::size_t index           = memToIndexPart[entity.getID()];
            assert(index != std::numeric_limits<size_t>::max() || !"this entity was not allocated!");

            // 書き込む型までのオフセット
            std::size_t offset = mArchetype.getTypeOffset(mArchetype.getTypeIndex<T>(), mMaxEntityNum);

            // 読み出し
            return *reinterpret_cast<T*>(mMemory.get() + offset + index * sizeof(T));
        }

        /**
         * @brief Entityが増えたらメモリを割り当て直す
         *
         * @param maxEntityNum 新しい最大Entity数
         */
        void reallocate(std::size_t maxEntityNum);

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

            return ComponentArray<T>(reinterpret_cast<T*>(mMemory.get() + offset), mEntityNum);
        }

    private:
        //! ChunkのID
        const std::size_t mID;

        //! もととなるArchetype
        Archetype mArchetype;
        //! メモリアドレス
        std::unique_ptr<std::byte> mMemory;
        //! 割り当てられる最大のEntity数
        std::size_t mMaxEntityNum;
        //! 現在のEntity数
        std::size_t mEntityNum;
        //! Entityに割り当てるテーブルの添字(ID)
        std::size_t mNextEntityIndex;
    };
}  // namespace mvecs

#endif