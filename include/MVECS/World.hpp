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
            : mIsRunning(false)
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
            mpChunks.clear();
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

            for (auto& e : mpChunks)
            {
                if (e->getArchetype() == archetype)
                {
                    return e->allocate();
                }
            }

            auto* p = new Chunk<Args...>(Chunk<Args...>::create(genChunkID(), archetype, reserveSizeIfCreatedNewChunk));

            return insertChunk(p)->allocate();
        }

        /**
         * @brief Entityを破棄する
         *
         * @param entity 破棄するEntity
         */
        void destroyEntity(const Entity& entity)
        {
            mpChunks[findChunk(entity.getChunkID())]->deallocate(entity);
            //auto& chunk = mpChunks[findChunk(entity.getChunkID())];
            //chunk->deallocate(entity);
            // mpChunks.find(entity.getChunkID())->second.deallocate(entity);
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
            //const auto index = findChunk(entity.getChunkID());
            return mpChunks[findChunk(entity.getChunkID())]->template getComponentData<T>(entity) = value;
            // mpChunks.find(entity.getChunkID())->second.setComponentData<T>(entity, value);
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
            //const auto index = findChunk(entity.getChunkID());
            return mpChunks[findChunk(entity.getChunkID())].template getComponentData<T>(entity);
            // return mpChunks.find(entity.getChunkID())->second.getComponentData<T>(entity);
        }

        /**
         * @brief Systemを追加する
         *
         * @tparam T 追加するSystemの型
         * @tparam typename System型判定用
         */
        template <typename T, typename = std::is_base_of<ISystem<Key, Common>, T>>
        void addSystem(int executionOrder = 0)
        {
            // mSystems.insert(std::move(std::make_pair(systemID, std::move(std::make_unique<T>(this)))).first->second);
            insertSystem(std::make_unique<T>(this, executionOrder));
        }

        /**
         * @brief 複数System型をまとめて追加する
         *
         * @tparam Args System型たち
         */
        template <typename... Args>
        void addSystems()
        {
            addSystemsImpl<Args...>();
        }

        /**
         * @brief 指定された全てのComponentData型に対してfor_eachを行う
         *
         * @tparam T
         * @param func 実行する関数オブジェクト
         */
        //template <typename T, typename = std::enable_if<IsComponentDataType<T>>>
        //void forEach(const std::function<void(T&)>& func)
        //{
        //    for (auto& chunk : mpChunks)
        //        if (chunk.getArchetype().isIn<T>())
        //            for (auto& componentData : chunk.getComponentArray<T>())
        //                func(componentData);
        //}

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

            for (auto& pChunk : mpChunks)
            {
                auto entityNum = pChunk->getEntityNum();
                if (pChunk->getArchetype().isIn(targetArchetype) && entityNum > 0)
                {
                    assert(entityNum);
                    auto&& tuple = std::make_tuple(pChunk->getComponentArray<Args>()...);
                    for (std::size_t i = 0; i < entityNum; ++i)
                    {
                        func(std::get<ComponentArray<Args>>(tuple)[i]...);
                    }
                }
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
            const auto&& reserveSize = mpChunks.size() / 4;
            componentArrays.reserve(reserveSize < 1 ? 1 : reserveSize);
            for (auto& pChunk : mpChunks)
            {
                if (pChunk->getArchetype().isIn<T>())
                {
                    componentArrays.emplace_back(pChunk->getComponentArray<T>());
                }
            }

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
            {
                thread.join();
            }
        }

        /**
         * @brief ISystem::onInit()を呼ぶ
         *
         */
        void init()
        {
            mIsRunning = true;
            for (auto& system : mSystems)
            {
                // system.second->onInit();
                system->onInit();
            }
        }

        /**
         * @brief ISystem::onUpdate()を呼ぶ
         *
         */
        void update()
        {
            for (auto itr = mSystems.begin(); itr != mSystems.end(); ++itr)
            {
                // system.second->onUpdate();
                (*itr)->onUpdate();

                if ((*itr)->removeThis())
                {
                    mSystems.erase(itr);
                    continue;
                }
            }

            // std::cout << "debug--------\n";
            // for (const auto& chunk : mpChunks)
            // {
            //     std::cout << "chunk size : " << chunk.getEntityNum() << "\n";
            //     chunk.dumpIndexMemory();
            //     std::cout << "\n\n";
            // }
            // std::cout << "-------------\n";
        }

        /**
         * @brief ISystem::onEnd()を呼ぶ
         *
         */
        void end()
        {
            for (auto& system : mSystems)
            {
                // system.second->onUpdate();
                system->onEnd();
            }

            for (auto& pChunk : mpChunks)
            {
                pChunk->destroy();
            }
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
        std::unique_ptr<IChunk>& insertChunk(IChunk* pChunk)
        {
            // mpChunks.emplace_back(std::move(chunk));
            // return mpChunks.back();

            auto&& uniquePtr = std::unique_ptr<IChunk>(pChunk);

            auto&& iter = std::lower_bound(mpChunks.begin(), mpChunks.end(), uniquePtr, [](const std::unique_ptr<IChunk>& left, const std::unique_ptr<IChunk>& right)
                                          { return left->getID() < right->getID(); });
            if (iter == mpChunks.end())
            {
                mpChunks.emplace_back(std::move(uniquePtr));
                iter = mpChunks.end() - 1;
            }
            else
            {
                mpChunks.insert(iter, std::move(uniquePtr));
            }

            return *iter;
        }

        /**
         * @brief ChunkIDに対応するChunkを二分探索する
         *
         * @param chunkID
         * @return std::size_t 対応するChunkの添字
         */
        std::size_t findChunk(std::size_t chunkID)
        {
            // for (std::size_t i = 0; i < mpChunks.size(); ++i)
            //     if (mpChunks[i].getID() == chunkID)
            //         return i;

            auto lower = mpChunks.begin();
            auto upper = mpChunks.end() - 1;
            while (lower <= upper)
            {
                auto mid = lower + (upper - lower) / 2;
                if (chunkID == (*mid)->getID())
                {
                    return std::distance(mpChunks.begin(), mid);
                }
                else if (chunkID < (*mid)->getID())
                {
                    upper = mid - 1;
                }
                else
                {
                    lower = mid + 1;
                }
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
            auto iter = std::lower_bound(mSystems.begin(), mSystems.end(), system, [](const std::unique_ptr<ISystem<Key, Common>>& left, const std::unique_ptr<ISystem<Key, Common>>& right)
                                        { return left->getExecutionOrder() < right->getExecutionOrder(); });
            if (iter == mSystems.end())
            {
                mSystems.emplace_back(std::move(system));
                iter = mSystems.end();
                --iter;
            }
            else
            {
                iter = mSystems.insert(iter, std::move(system));
            }

            if (mIsRunning)
            {
                (*iter)->onInit();
            }

            return *iter;
        }

        /**
         * @brief 複数System追加の実装部
         *
         * @tparam Head 先頭System型
         * @tparam Tail 残りのSystem型
         * @tparam typename System型判定用
         */
        template <typename Head, typename... Tail, typename = std::is_base_of<ISystem<Key, Common>, Head>>
        void addSystemsImpl()
        {
            addSystem<Head>();

            if constexpr (sizeof...(Tail) != 0)
            {
                addSystemsImpl<Tail...>();
            }
        }

        //! Applicationのポインタ
        Application<Key, Common>* mpApplication;

        //! Chunkたち
        std::vector<std::unique_ptr<IChunk>> mpChunks;

        //! Systemたち
        std::list<std::unique_ptr<ISystem<Key, Common>>> mSystems;

        //! すでにinitされたかどうか これによってSystem追加時にinitするかどうか決まる
        bool mIsRunning;
    };

}  // namespace mvecs

#endif