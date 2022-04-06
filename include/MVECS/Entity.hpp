#ifndef MVECS_MVECS_ENTITY_HPP_
#define MVECS_MVECS_ENTITY_HPP_

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
        Entity(std::size_t* pID, std::size_t chunkID);

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
        //! ChunkでのIDへのアドレス
        std::size_t* mpID;
        //! ChunkのID
        std::size_t mChunkID;
    };

}  // namespace mvecs

#endif