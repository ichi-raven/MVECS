#ifndef MVECS_MVECS_ICOMPONENTDATA_HPP_
#define MVECS_MVECS_ICOMPONENTDATA_HPP_

#include "TypeInfo.hpp"

namespace mvecs
{
    /**
     * @brief ComponentDataのインタフェース
     *
     */
    struct IComponentData
    {
    };

    /**
     * @brief ComponentDataにふさわしい型かチェックする
     * @details IComponentDataを継承していて、trivial・trivially_destructibleであり、TypeInfo制約をクリアしている型
     * @tparam T チェックする型
     */
    template <typename T>
    constexpr bool IsComponentDataType = TypeBinding::HasTypeInfoValue<T>;
        //std::is_base_of<IComponentData, T>::value&&
        //    std::is_trivial<T>::value&&
        //        std::is_trivially_destructible<T>::value&&
        //            TypeBinding::HasTypeInfoValue<T>;
}  // namespace mvecs

#endif