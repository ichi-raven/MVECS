#ifndef MVECS_ICOMPONENTDATA_HPP_
#define MVECS_ICOMPONENTDATA_HPP_

#include "TypeInfo.hpp"

namespace mvecs
{
    /**
     * @brief ComponentDataのインタフェース PODタイプ
     * 
     */
    struct IComponentData
    {
    };

    /**
     * @brief ComponentDataにふさわしい型かチェックする
     * @details IComponentDataを継承していて、trivial型であり、TypeInfo制約をクリアしている型
     * @tparam T チェックする型
     */
    template <typename T>
    constexpr bool IsComponentDataType = 
    std::is_base_of<IComponentData, T>::value && 
    std::is_trivial<T>::value && 
    std::is_trivially_destructible<T>::value && 
    TypeBinding::HasTypeInfoValue<T>;
}  // namespace mvecs

#endif