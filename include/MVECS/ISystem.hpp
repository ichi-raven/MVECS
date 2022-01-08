#ifndef MVECS_MVECS_ISYSTEM_HPP_
#define MVECS_MVECS_ISYSTEM_HPP_

#include <functional>
#include <memory>

#include "IComponentData.hpp"
//#include "World.hpp"

namespace mvecs
{
    template <typename Key, typename Common>
    class World;

// System型定義用、コンストラクタが展開される
#define SYSTEM(SystemType, KeyType, CommonType)  \
    SystemType()  = delete; \
    ~SystemType() = delete; \
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
         * @brief Systemが追加された時呼ばれるインタフェース
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
        void end()
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
        //! 実行する順序(若い順に実行される)
        int mExecutionOrder;
    };

}  // namespace mvecs

#endif