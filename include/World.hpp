#ifndef MVECS_WORLD_HPP_
#define MVECS_WORLD_HPP_

#include <algorithm>
#include <vector>
#include <iostream>

#include "Chunk.hpp"
#include "ComponentArray.hpp"

namespace mvecs
{
    class ISystem;
    template <typename T>
    constexpr bool IsSystemType = std::is_base_of<ISystem, T>::value;

    /**
     * @brief Worldクラス
     * @details いわゆるScene
     * 
     */
    class World : public std::enable_shared_from_this<World>
    {
    public:
        /**
         * @brief コンストラクタ
         * 
         */
        World();

        /**
         * @brief デストラクタ
         * 
         */
        ~World();

        /**
         * @brief Entity構築
         * 
         * @tparam Args Entityが持つComponentData
         * @return Entity 構築後のEntity
         */
        template <typename... Args>
        Entity createEntity()
        {
            constexpr Archetype archetype = Archetype::create<Args...>();
            for (auto& e : mChunks)
            {
                if (e.getArchetype() == archetype)
                    return e.allocate();
            }
            auto& chunk = mChunks.emplace_back(Chunk::create(genChunkID(), archetype));
            return chunk.allocate();
        }

        /**
         * @brief Entityを破棄する
         * 
         * @param entity 破棄するEntity
         */
        void destroyEntity(const Entity& entity);

        /**
         * @brief EntityのComponentDataを書き込む
         * 
         * @tparam T 書き込むComponentDataの型
         * @param entity 書き込み先Entity
         * @param value 書き込む値
         */
        template <typename T>
        void setComponentData(const Entity& entity, const T& value)
        {
            mChunks[searchChunk(entity.getChunkID())].setComponentData<T>(entity, value);
        }

        /**
         * @brief EntityのComponentDataを取得する
         * 
         * @tparam T ComponentDataの型
         * @param entity 取得先Entity
         * @return 取得したComponentDataの値
         */
        template <typename T>
        T getComponentData(const Entity& entity)
        {
            return mChunks[searchChunk(entity.getChunkID())].getComponentData<T>(entity);
        }

        /**
         * @brief Systemを追加する
         * 
         * @tparam T 追加するSystemの型
         * @tparam typename System型判定用
         */
        template <typename T, typename = std::enable_if_t<IsSystemType<T>>>
        void addSystem()
        {
            mSystems.emplace_back(std::make_unique<T>(shared_from_this()));
            startSystemImpl(mSystems.size() - 1);
        }

        /**
         * @brief 全てのSystemを削除する
         * 
         */
        void clearSystem();

        /**
         * @brief 指定された全てのComponentData型に対してforeachを行う
         * 
         * @tparam T 
         * @param func 実行する関数オブジェクト
         */
        template <typename T>
        void forEach(const std::function<void(T&)>& func)
        {
            std::vector<ComponentArray<T>> componentArrays;
            componentArrays.reserve(mChunks.size());
            for (std::size_t i = 0; i < mChunks.size(); ++i)
            {
                if (mChunks[i].getArchetype().isIn<T>())
                    componentArrays.emplace_back(mChunks[i].getComponentArray<T>());
            }

            // 実行
            for (auto& componentArray : componentArrays)
                for (std::size_t j = 0; j < componentArray.size(); ++j)
                    func(componentArray[j]);
        }

        /**
         * @brief 更新を行う
         * 
         */
        void update();

    private:
        /**
         * @brief ISystem->onStart()を実行する
         * @details 循環インクルード回避のため
         * @param index ISystemの添字
         */
        void startSystemImpl(std::size_t index);

        /**
         * @brief ChunkのIDを生成する
         * 
         * @return std::size_t 生成されたID
         */
        static std::size_t genChunkID();

        /**
         * @brief そのIDを持つChunkを検索する
         * 
         * @param ID ID
         * @return std::size_t Chunkの添字
         */
        inline std::size_t searchChunk(std::size_t ID)
        {
            for (std::size_t i = 0; i < mChunks.size(); ++i)
                if (mChunks[i].getID() == ID)
                    return i;

            return std::numeric_limits<std::size_t>::max();
        }

        //! Chunkたち
        std::vector<Chunk> mChunks;

        //! Systemたち
        std::vector<std::unique_ptr<ISystem>> mSystems;
    };
}  // namespace mvecs

#endif