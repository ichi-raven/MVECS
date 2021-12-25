#ifndef MVECS_MVECS_WORLD_HPP_
#define MVECS_MVECS_WORLD_HPP_

#include <algorithm>
#include <iostream>
#include <thread>
#include <vector>

#include "Chunk.hpp"
#include "ComponentArray.hpp"

namespace mvecs
{
    // 前方宣言(見なかったことにしてください)
    class ISystem;
    template <typename T>
    constexpr bool IsSystemType = std::is_base_of<ISystem, T>::value;

    /**
     * @brief Worldクラス
     * @details いわゆるScene
     *
     */
    class World
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
        Entity createEntity(const std::size_t reserveSizeIfNewChunk = 1)
        {
            constexpr Archetype archetype = Archetype::create<Args...>();
            for (auto& e : mChunks)
            {
                if (e.getArchetype() == archetype)
                    return e.allocate();
            }
            const auto&& chunkID = genChunkID();
            return insertChunk(Chunk::create(chunkID, archetype, reserveSizeIfNewChunk)).allocate();
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
            const auto index = findChunk(entity.getChunkID());
            return mChunks[index].setComponentData<T>(entity, value);
            // mChunks.find(entity.getChunkID())->second.setComponentData<T>(entity, value);
        }

        /**
         * @brief EntityのComponentDataを取得する
         *
         * @tparam T ComponentDataの型
         * @param entity 取得先Entity
         * @return 取得したComponentDataの値
         */
        template <typename T>
        T& getComponentData(const Entity& entity)
        {
            const auto index = findChunk(entity.getChunkID());
            return mChunks[index].getComponentData<T>(entity);
            // return mChunks.find(entity.getChunkID())->second.getComponentData<T>(entity);
        }

        /**
         * @brief Systemを追加する
         *
         * @tparam T 追加するSystemの型
         * @tparam typename System型判定用
         * @return std::size_t SystemのID
         */
        template <typename T, typename = std::is_base_of<ISystem, T>>
        std::size_t addSystem()
        {
            auto&& systemID = genSystemID();
            // startSystemImpl(mSystems.insert(std::move(std::make_pair(systemID, std::move(std::make_unique<T>(this))))).first->second);
            startSystemImpl(insertSystem(std::make_unique<T>(this)));
            return systemID;
        }

        /**
         * @brief Systemを取り除く
         *
         * @param systemID 取り除くSystemのID
         */
        // void removeSystem(std::size_t systemID);

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
        template <typename T, typename = std::enable_if_t<IsComponentDataType<T>>>
        void forEach(const std::function<void(T&)>& func)
        {
            // ComponentArray作成
            std::vector<ComponentArray<T>> componentArrays;
            componentArrays.reserve(mChunks.size() / 2);
            for (auto& chunk : mChunks)
                if (chunk.getArchetype().isIn<T>())
                    componentArrays.emplace_back(chunk.getComponentArray<T>());

            // 実行
            for (auto& componentArray : componentArrays)
                for (std::size_t j = 0; j < componentArray.size(); ++j)
                    func(componentArray[j]);
        }

        /**
         * @brief forEachを並列実行する
         * @warning funcはthread-safeであること
         * @tparam T
         * @param func
         */
        template <typename T, typename = std::enable_if_t<IsComponentDataType<T>>>
        void forEachParallel(const std::function<void(T&)>& func, const uint8_t threadNum = 4)
        {
            // ComponentArray作成
            std::vector<ComponentArray<T>> componentArrays;
            componentArrays.reserve(mChunks.size() / 2);
            for (auto& chunk : mChunks)
                if (chunk.getArchetype().isIn<T>())
                    componentArrays.emplace_back(chunk.getComponentArray<T>());

            // 実行
            std::vector<std::thread> threads;
            threads.resize(threadNum);
            for (uint8_t i = 0; i < threadNum; ++i)
            {
                threads[i] = std::thread(
                    [i, threadNum, &componentArrays, &func]()
                    {
                        std::size_t start = 0, end = 0;
                        for (auto& componentArray : componentArrays)
                        {
                            start = i * componentArray.size() / threadNum;
                            end   = (i + 1) * componentArray.size() / threadNum;
                            for (std::size_t j = start; j < end; ++j)
                                func(componentArray[j]);
                        }
                    });
            }

            for (auto& thread : threads)
                thread.join();
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
        void startSystemImpl(const std::unique_ptr<ISystem>& system);

        /**
         * @brief ChunkのIDを生成する
         *
         * @return std::size_t 生成されたID
         */
        static std::size_t genChunkID();

        /**
         * @brief SystemのIDを生成する
         *
         * @return std::size_t 生成されたID
         */
        static std::size_t genSystemID();

        /**
         * @brief Chunkをvectorに挿入する(常にソートする)
         *
         * @param chunk
         * @return Chunk&
         */
        Chunk& insertChunk(Chunk&& chunk);

        /**
         * @brief ChunkIDに対応するChunkを二分探索する
         *
         * @param chunkID
         * @return std::size_t 対応するChunkの添字
         */
        std::size_t findChunk(std::size_t chunkID);

        /**
         * @brief systemをvectorに挿入する(常にソートする)
         *
         * @param system
         * @return std::unique_ptr<ISystem>&
         */
        std::unique_ptr<ISystem>& insertSystem(std::unique_ptr<ISystem>&& system);

        //! Chunkたち
        std::vector<Chunk> mChunks;

        //! Systemたち
        std::vector<std::unique_ptr<ISystem>> mSystems;
    };
}  // namespace mvecs

#endif