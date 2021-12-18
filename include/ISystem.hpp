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
        ISystem(World* const pWorld);

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
         * @param func 
         */
        template<typename T, typename = std::enable_if_t<IsComponentDataType<T>>>
        void forEach(std::function<void(T&)> func)
        {
            mpWorld->forEach<T>(func);
        }

        template<typename... Args>
        void forEach(std::function<void(const Args&...)>)
        {

        }

    protected:
        World* const mpWorld;
    };

}

#endif