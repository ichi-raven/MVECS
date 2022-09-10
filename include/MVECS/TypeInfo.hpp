#ifndef MVECS_MVECS_TYPEINFO_HPP_
#define MVECS_MVECS_TYPEINFO_HPP_

#include <cstdint>
#include <functional>
#include <string_view>
#include <type_traits>

//! ComponentDataには必ずこれらを定義すること
#define COMPONENT_DATA(T)                                            \
public:                                                              \
    static constexpr std::uint32_t getTypeHash()                     \
    {                                                                \
        std::string_view typeStr = #T;                               \
        return mvecs::CompileTimeHash::fnv1a_32(#T, typeStr.size()); \
    }

namespace mvecs
{
    /**
     * @brief SFINAEでgetTypeNameの実装を判定する
     * 
     */
    namespace TypeBinding
    {
        /**
         * @brief 実装
         * 
         */
        struct HasTypeInfoImpl
        {
            template <typename T>
            static auto check(T&& val) -> decltype(val.getTypeHash(), std::true_type{});

            template <typename T>
            static auto check(...) -> std::false_type;
        };

        /**
         * @brief USE_TYPEINFOマクロの内容を定義しているか判定
         * @details constexpr bool b = HasTypeInfo<T>(); のように使用する
         * @tparam T 
         */
        template <typename T>
        class HasTypeInfo : public decltype(HasTypeInfoImpl::check<T>(std::declval<T>()))
        {
        };

        /**
         * @brief 型にCOMPONENT_DATAマクロが定義されているかを判定し、値を取得する
         * 
         * @tparam T 判定する型
         */
        template <typename T>
        constexpr bool HasTypeInfoValue = HasTypeInfo<T>();
    };  // namespace TypeBinding

    /**
     * @brief コンパイル時ハッシュ関数の名前空間
     * 
     */
    namespace CompileTimeHash
    {
        /**
         * @brief コンパイル時に文字列をハッシュ化する(拾い物)
         * 
         * @param s 文字列
         * @param count 文字列のサイズ
         * @return constexpr std::uint32_t ハッシュ値
         */
        constexpr std::uint32_t fnv1a_32(char const* s, std::size_t count)
        {
            return ((count ? fnv1a_32(s, count - 1) : 2166136261u) ^ s[count]) * 16777619u;
        }

        /**
         * @brief checkSynonymの内部実装, 直接使う意味はありません
         * 
         * @tparam size 
         * @tparam Head 
         * @tparam Tails 
         * @param table 
         * @param now 
         * @return true 
         * @return false 
         */
        template <std::size_t size, typename Head, typename... Tails>
        bool checkSynonymImpl(std::array<std::uint32_t, size>& table, std::size_t now)
        {
            assert(TypeBinding::HasTypeInfoValue<Head> || !"this type does not have a type info!");
            if (now >= size - 1)
            {
                return false;
            }

            uint32_t&& hash = Head::getTypeHash();
            for (std::size_t i = 0; i < now; ++i)
            {
                if (table[i] == hash)
                {
                    return true;
                }
            }
            
            table[now] = hash;

            if (sizeof...(Tails) != 0)
            {
                return checkSynonymImpl<Tails...>(table, now + 1);
            }
            else
            {
                return false;
            }
        }
        
        /**
         * @brief TypeInfoを持つ型のハッシュ値同士が衝突していないかをチェックする(低確率だが一応)
         * 
         * @tparam Args 型たち
         * @return true 衝突している(注意)
         * @return false 衝突していない
         */
        template <typename... Args>
        bool checkSynonym()
        {
            std::array<std::uint32_t, sizeof...(Args)> table;
            
            return checkSynonymImpl<sizeof...(Args), Args...>(table, 0);
        }

    }  // namespace CompileTimeHash

    /**
     * @brief 型のIDとそのサイズを取得できる情報
     * 
     */
    struct TypeInfo
    {
    private:
        /**
         * @brief 使用する型がTypeInfo制約をクリアしているかどうか判定できないためprivate
         * 
         */
        constexpr TypeInfo(std::size_t size, std::uint32_t hash)
            : mSize(size)
            , mTypeHash(hash)
        {
        }

    public:
        /**
         * @brief デフォルトコンストラクタ
         * 
         */
        constexpr TypeInfo()
            : mSize(0)
            , mTypeHash(0)
        {
        }

        /**
         * @brief 等しい
         * 
         * @param other 
         * @return true 
         * @return false 
         */
        constexpr bool operator==(const TypeInfo& other) const
        {
            return (mTypeHash == other.getHash());
        }

        /**
         * @brief 等しくない
         * 
         * @param other 
         * @return true 
         * @return false 
         */
        constexpr bool operator!=(const TypeInfo& other) const
        {
            return !(*this == other);
        }

        /**
         * @brief TypeInfo制約をクリアした型から構築する
         * 
         * @tparam T 型
         * @tparam typename TypeInfoの条件を満たせるか判定
         * @return constexpr TypeInfo 構築したTypeInfo
         */
        template <typename T, typename = std::enable_if_t<TypeBinding::HasTypeInfoValue<T>>>
        static constexpr TypeInfo create()
        {
            return TypeInfo(sizeof(T), T::getTypeHash());
        }

        /**
         * @brief ハッシュ値を取得
         * 
         * @return constexpr std::uint32_t 
         */
        constexpr std::uint32_t getHash() const
        {
            return mTypeHash;
        }

        /**
         * @brief 型のサイズを取得
         * 
         * @return constexpr std::size_t 
         */
        constexpr std::size_t getSize() const
        {
            return mSize;
        }

    private:
        //! 型のサイズ
        std::size_t mSize;
        //! 型のハッシュ値
        std::uint32_t mTypeHash;
    };

}  // namespace mvecs

#endif