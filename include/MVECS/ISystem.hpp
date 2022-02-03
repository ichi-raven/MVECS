#ifndef MVECS_MVECS_ISYSTEM_HPP_
#define MVECS_MVECS_ISYSTEM_HPP_

#include <functional>
#include <memory>

#include "IComponentData.hpp"
#include "Entity.hpp"
//#include "World.hpp"

namespace mvecs
{
    template <typename Key, typename Common>
    class World;

// System型定義用、コンストラクタが展開される
#define SYSTEM(SystemType, KeyType, CommonType) \
public:                                         \
    SystemType()  = delete;                     \
    ~SystemType() = delete;                     \
    SystemType(mvecs::World<KeyType, CommonType>* const pWorld, const int executionOrder = 0) : mvecs::ISystem<KeyType, CommonType>(pWorld, executionOrder) {}

    template <typename Key, typename Common>
    class ISystem
    {
    public:
        //! デフォルトコンストラクタはdelete
        ISystem() = delete;

        /**
         * @brief コンストラクタ
         * @warning World外での構築は危険
         * @param pWorld Worldへのポインタ
         */
        ISystem(World<Key, Common>* const pWorld, const int executionOrder = 0)
            : mpWorld(pWorld)
            , mExecutionOrder(executionOrder)
        {
        }

        /**
         * @brief World初期化時 or Systemが追加された時に呼ばれるインタフェース
         *
         */
        virtual void onInit() = 0;

        /**
         * @brief Worldが更新された時呼ばれるインタフェース
         *
         */
        virtual void onUpdate() = 0;

        /**
         * @brief Worldが削除された時呼ばれるインタフェース
         *
         */
        virtual void onEnd() = 0;

        /**
         * @brief 特定のComponentData型にforEachする
         *
         * @tparam T Component型
         * @tparam typename ComponentData判定用
         * @param func 実行する関数オブジェクト
         */
        template <typename T, typename = std::enable_if_t<IsComponentDataType<T>>>
        void forEach(std::function<void(T&)> func)
        {
            mpWorld->template forEach<T>(func);
        }

        /**
         * @brief forEachを並列実行する
         * @warning funcはthread-safeであること
         * @tparam T Component型
         * @tparam typename ComponentData判定用
         * @param func 実行する関数オブジェクト
         */
        template <typename T, typename = std::enable_if_t<IsComponentDataType<T>>>
        void forEachParallel(std::function<void(T&)> func, uint8_t threadNum = 4)
        {
            mpWorld->template forEachParallel<T>(func, threadNum);
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
            mpWorld->template createEntity<Args...>(reserveSizeIfCreatedNewChunk);
        }

        /**
         * @brief Entityを破棄する
         *
         * @param entity 破棄するEntity
         */
        void destroyEntity(const Entity& entity)
        {
            mpWorld->destroyEntity(entity);
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
            mpWorld->template setComponentData<T>(entity, value);
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
            return mpWorld->template getComponentData<T>(entity);
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
            mpWorld->template addSystem<T>();
        }
        /**
         * @brief World切り替えを通知する
         *
         * @param key 切り替え先
         * @param reset 初期化を行うかどうか
         */
        void change(const Key& key, bool reset = true)
        {
            mpWorld->change(key, reset);
        }

        /**
         * @brief Application終了を通知する
         *
         */
        void endAll()
        {
            mpWorld->dispatchEnd();
        }

        /**
         * @brief 共有領域を取得する
         *
         * @return Common&
         */
        Common& common()
        {
            return mpWorld->common();
        }

        /**
         * @brief 実行順序を取得する
         *
         * @return int 実行順序
         */
        int getExecutionOrder() const
        {
            return mExecutionOrder;
        }

    private:
        //! Worldへのポインタ(SystemをWorld外で作成しないで)
        World<Key, Common>* const mpWorld;

    protected:
        //! 実行する順序(小さい順に実行される)
        int mExecutionOrder;
    };

}  // namespace mvecs

#endif