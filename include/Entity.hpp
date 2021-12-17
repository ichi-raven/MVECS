#ifndef MVECS_ENTITY_HPP_
#define MVECS_ENTITY_HPP_

#include <cstdint>

namespace mvecs
{
    /**
     * @brief 実体を表す 振る舞いとしては旧来のGameObject
     * 
     */
    struct Entity
    {
        /**
         * @brief コンストラクタ
         * 
         * @param ID ChunkでのID
         * @param chunkID ChunkのID
         */
        Entity(std::size_t ID, std::size_t chunkID);

        /**
         * @brief ChunkでのIDを取得
         * 
         * @return std::size_t ID ChunkでのID
         */
        std::size_t getID() const;

        /**
         * @brief ChunkのIDを取得
         * 
         * @return std::size_t ChunkのID
         */
        std::size_t getChunkID() const;

    private:
        //! ChunkでのID
        std::size_t mID;
        //! ChunkのID
        std::size_t mChunkID;
    };

}  // namespace mvecs

#endif