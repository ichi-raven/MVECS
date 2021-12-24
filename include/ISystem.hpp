#ifndef MVECS_ISYSTEM_HPP_
#define MVECS_ISYSTEM_HPP_

#include "World.hpp"

#include <functional>
#include <memory>


namespace mvecs
{
    class ISystem
    {
    public:
        /**
         * @brief コンストラクタ
         * @warning World以外での構築は危険
         * @param pWorld Worldへのポインタ
         */
        ISystem(World* const pWorld, const int executionOrder = 0);

        /**
         * @brief Systemが追加された時呼ばれるインタフェース
         * 
         */
        virtual void onStart() = 0;

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
        template<typename T, typename = std::enable_if_t<IsComponentDataType<T>>>
        void forEach(std::function<void(T&)> func)
        {
            mpWorld->forEach<T>(func);
        }

        /**
         * @brief forEachを並列実行する
         * @warning funcはthread-safeであること
         * @tparam T Component型
         * @tparam typename ComponentData判定用
         * @param func 実行する関数オブジェクト
         */
        template<typename T, typename = std::enable_if_t<IsComponentDataType<T>>>
        void forEachParallel(std::function<void(T&)> func, uint8_t threadNum = 4)
        {
            mpWorld->forEachParallel<T>(func, threadNum);
        }

        template<typename... Args>
        void forEach(std::function<void(const Args&...)>)
        {

        }
        
        /**
         * @brief 実行順序を取得する
         * 
         * @return int 実行順序
         */
        int getExecutionOrder() const;

    protected:
        //! Worldへのポインタ(SystemをWorld外で作成しないで)
        World* const mpWorld;
        //! 実行する順序(若い順に実行される)
        int mExecutionOrder;
    };

}

#endif