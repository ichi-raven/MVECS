#ifndef MVECS_TYPEINFO_HPP_
#define MVECS_TYPEINFO_HPP_

#include <cstdint>
#include <functional>
#include <string_view>
#include <type_traits>

// ComponentDataには必ずこれらを定義すること
#define USE_TYPEINFO(T)                                                    \
public:                                                                    \
    static constexpr std::string_view getTypeName()                        \
    {                                                                      \
        return #T;                                                         \
    }                                                                      \
                                                                           \
    static constexpr std::uint32_t getTypeHash()                           \
    {                                                                      \
        return mvecs::CompileTimeHash::fnv1a_32(#T, getTypeName().size()); \
    }

namespace mvecs
{
    // SFINAEでgetTypeNameの実装を判定する
    namespace TypeBinding
    {
        // 実装
        struct HasTypeInfoImpl
        {
            template <typename T>
            static auto check(T&& val) -> decltype(val.getTypeName(), val.getTypeHash(), std::true_type{});

            template <typename T>
            static auto check(...) -> std::false_type;
        };

        // USE_TYPEINFOマクロの内容を定義しているか判定
        // constexpr bool b = HasTypeInfo<T>(); のように使用する
        template <typename T>
        class HasTypeInfo : public decltype(HasTypeInfoImpl::check<T>(std::declval<T>()))
        {
        };

        template <typename T>
        constexpr bool HasTypeInfoValue = HasTypeInfo<T>();
    };  // namespace TypeBinding

    namespace CompileTimeHash
    {
        // 拾い物
        constexpr std::uint32_t fnv1a_32(char const* s, std::size_t count)
        {
            return ((count ? fnv1a_32(s, count - 1) : 2166136261u) ^ s[count]) * 16777619u;
        }
    }  // namespace CompileTimeHash

    struct TypeInfo
    {
    private:
        constexpr TypeInfo(std::size_t size, std::uint32_t hash)
            : mSize(size), mTypeHash(hash)
        {
        }

    public:
        constexpr TypeInfo()
            : mSize(0), mTypeHash(0)
        {
        }

        constexpr bool operator==(const TypeInfo& other) const
        {
            return (mTypeHash == other.getHash());
        }

        constexpr bool operator!=(const TypeInfo& other) const
        {
            return !(*this == other);
        }

        template <typename T, typename = std::enable_if_t<TypeBinding::HasTypeInfoValue<T>>>
        static constexpr TypeInfo create()
        {
            return TypeInfo(sizeof(T), T::getTypeHash());
        }

        constexpr std::size_t getHash() const
        {
            return mTypeHash;
        }

        constexpr std::size_t getSize() const
        {
            return mSize;
        }

    private:
        std::size_t mSize;
        std::uint32_t mTypeHash;
    };

}  // namespace mvecs

#endif