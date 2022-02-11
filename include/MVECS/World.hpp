#ifndef MVECS_MVECS_WORLD_HPP_
#define MVECS_MVECS_WORLD_HPP_

#include <algorithm>
#include <optional>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include "Chunk.hpp"
#include "ComponentArray.hpp"

namespace mvecs
{
    template <typename Key, typename Common>
    class Application;

    template <typename Key, typename Common>
    class ISystem;

    template <typename T, typename Key, typename Common>
    constexpr bool IsSystemType = std::is_base_of<ISystem<Key, Common>, T>::value;

    /**
     * @brief Worldクラス EntityとSystemを保持する
     * @details いわゆるScene
     * @tparam Key Worldを一意に識別するキーの型
     */
    template <typename Key, typename Common>
    class World
    {
    public:
        /**
         * @brief コンストラクタ
         *
         */
        World(Application<Key, Common>* pApplication)
            : mIsInitialized(false)
            , mpApplication(pApplication)
        {
        }

        /**
         * @brief デストラクタ
         *
         */
        ~World()
        {
            mSystems.clear();
            mChunks.clear();
        }

        /**
         * @brief Entity構築
         *
         * @tparam Args Entityが持つComponentData
         * @param reserveSizeIfCreatedNewChunk Chunkが新しく構築される場合に確保する容量(デフォルトで1)
         * @return Entity 構築したEntity
         */
        template <typename... Args>
        Entity createEntity(const std::size_t reserveSizeIfCreatedNewChunk = 1)
        {
            constexpr Archetype archetype = Archetype::create<Args...>();
            for (auto& e : mChunks)
            {
                if (e.getArchetype() == archetype)
                    return e.allocate();
            }

            return insertChunk(Chunk::create(genChunkID(), archetype, reserveSizeIfCreatedNewChunk)).allocate();
        }

        /**
         * @brief Entityを破棄する
         *
         * @param entity 破棄するEntity
         */
        void destroyEntity(const Entity& entity)
        {
            mChunks[findChunk(entity.getChunkID())].deallocate(entity);
            // mChunks.find(entity.getChunkID())->second.deallocate(entity);
        }

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
            return mChunks[index].template setComponentData<T>(entity, value);
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
            return mChunks[index].template getComponentData<T>(entity);
            // return mChunks.find(entity.getChunkID())->second.getComponentData<T>(entity);
        }

        /**
         * @brief Systemを追加する
         *
         * @tparam T 追加するSystemの型
         * @tparam typename System型判定用
         */
        template <typename T, typename = std::is_base_of<ISystem<Key, Common>, T>>
        void addSystem()
        {
            // mSystems.insert(std::move(std::make_pair(systemID, std::move(std::make_unique<T>(this)))).first->second);
            insertSystem(std::make_unique<T>(this));
        }

        /**
         * @brief 指定された全てのComponentData型に対してfor_eachを行う
         *
         * @tparam T
         * @param func 実行する関数オブジェクト
         */
        template <typename T, typename = std::enable_if_t<IsComponentDataType<T>>>
        void forEach(const std::function<void(T&)>& func)
        {
            for (auto& chunk : mChunks)
                if (chunk.getArchetype().isIn<T>())
                    for (auto& component : chunk.getComponentArray<T>())
                        func(component);
        }

        /**
         * @brief 複数のComponentData型に対するforEach
         * @warning 指定したComponentData型をすべて含むChunk(Entity)しか巡回されない
         * @tparam Args 
         * @param func 
         */
        template <typename... Args>
        void forEach(const std::function<void(Args&...)>& func)
        {
            assert(sizeof...(Args) != 0 || !"empty type to forEach!");

            constexpr Archetype targetArchetype = Archetype::create<Args...>();

            for (auto& chunk : mChunks)
                if (chunk.getArchetype().isIn(targetArchetype))
                {
                    auto&& tuple = std::make_tuple(chunk.getComponentArray<Args>()...);
                    for (std::size_t i = 0; i < chunk.getEntityNum(); ++i)
                        func(std::get<ComponentArray<Args>>(tuple)[i]...);
                }
        }

        /**
         * @brief forEachを並列実行する
         * @warning funcはthread-safeであること
         * 
         * @tparam T 実行するComponentData型
         * @tparam ThreadNum スレッド数(デフォルトで4)
         * @tparam typename ComponentData型判定
         * @param func 
         */
        template <typename T, std::size_t ThreadNum = 4, typename = std::enable_if_t<IsComponentDataType<T>>>
        void forEachParallel(const std::function<void(T&)>& func)
        {
            // ComponentArray作成
            std::vector<ComponentArray<T>> componentArrays;
            const auto&& reserveSize = mChunks.size() / 4;
            componentArrays.reserve(reserveSize < 1 ? 1 : reserveSize);
            for (auto& chunk : mChunks)
                if (chunk.getArchetype().isIn<T>())
                    componentArrays.emplace_back(chunk.getComponentArray<T>());

            // 実行
            std::array<std::thread, ThreadNum> threads;
            for (uint8_t i = 0; i < ThreadNum; ++i)
            {
                threads[i] = std::thread(
                    [i, &componentArrays, &func]()
                    {
                        std::size_t start = 0, end = 0;
                        for (auto& componentArray : componentArrays)
                        {
                            start = i * componentArray.size() / ThreadNum;
                            end   = (i + 1) * componentArray.size() / ThreadNum;
                            for (std::size_t j = start; j < end; ++j)
                                func(componentArray[j]);
                        }
                    });
            }

            for (auto& thread : threads)
                thread.join();
        }

        /**
         * @brief ISystem::onInit()を呼ぶ
         *
         */
        void init()
        {
            for (auto& system : mSystems)
                // system.second->onUpdate();
                system->onInit();
            mIsInitialized = true;
        }

        /**
         * @brief ISystem::onUpdate()を呼ぶ
         *
         */
        void update()
        {
            for (auto& system : mSystems)
                // system.second->onUpdate();
                system->onUpdate();
        }

        /**
         * @brief ISystem::onEnd()を呼ぶ
         *
         */
        void end()
        {
            for (auto& system : mSystems)
                // system.second->onUpdate();
                system->onEnd();

            for (auto& chunk : mChunks)
                chunk.clear();
        }

        /**
         * @brief World切り替えを通知する
         *
         * @param key 切り替え先
         * @param reset 初期化を行うかどうか
         */
        void change(const Key& key, bool reset = true)
        {
            mpApplication->change(key, reset);
        }

        /**
         * @brief Application終了を通知する
         *
         */
        void dispatchEnd()
        {
            mpApplication->dispatchEnd();
        }

        /**
         * @brief 共有領域を取得する
         *
         * @return Common&
         */
        Common& common()
        {
            return mpApplication->common();
        }

    private:

        /**
         * @brief ChunkのIDを生成する
         *
         * @return std::size_t 生成されたID
         */
        static std::size_t genChunkID()
        {
            static std::size_t chunkID = 0;
            return chunkID++;
        }

        /**
         * @brief Chunkをvectorに挿入する(常にソートする)
         *
         * @param chunk
         * @return Chunk&
         */
        Chunk& insertChunk(Chunk&& chunk)
        {
            // mChunks.emplace_back(std::move(chunk));
            // return mChunks.back();

            auto&& itr = std::lower_bound(mChunks.begin(), mChunks.end(), chunk, [](const Chunk& left, const Chunk& right)
                                        { return left.getID() < right.getID(); });
            if (itr == mChunks.end())
            {
                mChunks.emplace_back(std::move(chunk));
                itr = --mChunks.end();
            }
            else
                mChunks.insert(itr, std::move(chunk));
            return *itr;
        }

        /**
         * @brief ChunkIDに対応するChunkを二分探索する
         *
         * @param chunkID
         * @return std::size_t 対応するChunkの添字
         */
        std::size_t findChunk(std::size_t chunkID)
        {
            // for (std::size_t i = 0; i < mChunks.size(); ++i)
            //     if (mChunks[i].getID() == chunkID)
            //         return i;

            auto lower = mChunks.begin();
            auto upper = mChunks.end() - 1;
            while (lower <= upper)
            {
                auto mid = lower + (upper - lower) / 2;
                if (chunkID == mid->getID())
                    return std::distance(mChunks.begin(), mid);
                else if (chunkID < mid->getID())
                    upper = mid - 1;
                else
                    lower = mid + 1;
            }

            assert(!"chunk was not found!");
            return std::numeric_limits<std::size_t>::max();
        }

        /**
         * @brief systemをvectorに挿入する(常にソートする)
         *
         * @param system
         * @return std::unique_ptr<ISystem>&
         */
        std::unique_ptr<ISystem<Key, Common>>& insertSystem(std::unique_ptr<ISystem<Key, Common>>&& system)
        {
            auto itr = std::lower_bound(mSystems.begin(), mSystems.end(), system, [](const std::unique_ptr<ISystem<Key, Common>>& left, const std::unique_ptr<ISystem<Key, Common>>& right)
                                        { return left->getExecutionOrder() < right->getExecutionOrder(); });
            if (itr == mSystems.end())
            {
                mSystems.emplace_back(std::move(system));
                itr = --mSystems.end();
            }
            else
                mSystems.insert(itr, std::move(system));

            if (mIsInitialized)
                (*itr)->onInit();

            return *itr;
        }

        //! Applicationのポインタ
        Application<Key, Common>* mpApplication;

        //! Chunkたち
        std::vector<Chunk> mChunks;

        //! Systemたち
        std::vector<std::unique_ptr<ISystem<Key, Common>>> mSystems;

        //! すでにinitされたかどうか これによってSystem追加時にinitするかどうか決まる
        bool mIsInitialized;
    };

}  // namespace mvecs

#endif