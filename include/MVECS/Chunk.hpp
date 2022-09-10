/*****************************************************************//**
 * @file   Chunk.hpp
 * @brief  IChunkの可変長テンプレート引数を持つためのクラス定義・実装
 *
 * @author ichi-raven
 * @date   September 2022
 *********************************************************************/
#ifndef MVECS_MVECS_CHUNK_HPP_
#define MVECS_MVECS_CHUNK_HPP_

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <memory>
#include <vector>
#include <tuple>
#include <utility>

#include "Archetype.hpp"
#include "ComponentArray.hpp"
#include "Entity.hpp"
#include "IChunk.hpp"

 /**
  * @brief mvecs
  *
  */
namespace mvecs
{
	/**
	 * @brief Chunkクラス Archetypeから作られる実際のメモリ上のデータ(テンプレート引数を取れる実装)
	 *
	 */
	template<typename... Args>
	class Chunk : public IChunk
	{
	public:
		/**
		 * @brief 与えられたArchetypeからChunkを構築する
		 *
		 * @param entitySize Chunkが持つEntityの最大数
		 * @return Chunk 構築したChunk
		 */
		static Chunk<Args...>&& create(const std::size_t ID, const Archetype& archetype, const std::size_t maxEntityNum = 1)
		{
			Chunk<Args...> rtn(ID, archetype);

			std::size_t memSize = archetype.getAllTypeSize() * maxEntityNum;

			// 割り当て
			rtn.mpMemory = new std::byte[memSize]();

			// メモリクリア(やらなくてもいいかも)
			std::memset(rtn.mpMemory, 0, memSize);
			rtn.mMaxEntityNum = maxEntityNum;

			assert(rtn.mMaxEntityNum != 0);

			return std::move(rtn);
		}

		/**
		 * @brief コンストラクタ
		 *
		 * @param ID
		 */
		Chunk(const std::size_t ID, const Archetype& archetype)
			: IChunk(ID, archetype)
		{
		}

		/**
		 * @brief デストラクタ
		 *
		 */
		virtual ~Chunk()
		{
			destroy();
		}

		/**
		 * @brief コピーコンストラクタはdelete
		 *
		 * @param src
		 */
		Chunk(Chunk& src) = delete;

		/**
		 * @brief 代入によるコピーもdelete
		 *
		 * @param src
		 * @return Chunk&
		 */
		Chunk& operator=(const Chunk& src) = delete;

		/**
		 * @brief ムーブコンストラクタ
		 *
		 * @param chunk ムーブ元
		 */
		Chunk(Chunk&& src) noexcept
			: IChunk(std::forward<Chunk&&>(src))
		{
		}

		/**
		 * @brief ムーブコンストラクタ 演算子オーバーロード
		 *
		 * @param src
		 * @return Chunk&
		 */
		Chunk& operator=(Chunk&& src) noexcept
		{
			mID = src.mID;
			mArchetype = src.mArchetype;
			mpMemory = src.mpMemory;
			mMaxEntityNum = src.mMaxEntityNum;
			mEntityNum = src.mEntityNum;

			src.destroy();

			return *this;
		}

		/**
		 * @brief Entityの領域を確保する
		 * @details 実際の値は何も書き込まれていないので注意
		 * @return Entity
		 */
		virtual Entity allocate() override
		{
			std::size_t* const pIndex = new std::size_t(mEntityNum);
			Entity entity(pIndex, mID);

			// 構築
			for (std::size_t i = 0; i < sizeof...(Args); ++i)
			{
				// 書き込む型までのオフセット
				const std::size_t offset = mArchetype.getTypeOffset(i, mMaxEntityNum);

				std::byte* ptr = (mpMemory + offset + entity.getID() * mArchetype.getTypeSize(i));

				construct<Args...>(mArchetype.getReverseTypeIndex(i), ptr);
			}

			// 新しいindexを挿入
			mpEntityIDs.emplace_back(pIndex);

			if (mEntityNum + 1 >= mMaxEntityNum)
			{
				// DEBUG!!!!!!!!
				//std::cerr << "plus realloc : " << mMaxEntityNum * 2 << "\n";
				// メモリを再割り当てする
				reallocate(mMaxEntityNum * 2);  // std::vectorの真似
			}

			// 更新
			++mEntityNum;

			return entity;
		}

		/**
		 * @brief 指定したEntityを削除してメモリを詰める
		 *
		 * @param entity 削除するEntity
		 */
		virtual void deallocate(const Entity& entity) override
		{
			std::size_t deallocatedIndex = entity.getID();

			// 破棄
			for (std::size_t i = 0; i < sizeof...(Args); ++i)
			{
				// 書き込む型までのオフセット
				const std::size_t offset = mArchetype.getTypeOffset(i, mMaxEntityNum);

				std::byte* ptr = (mpMemory + offset + entity.getID() * mArchetype.getTypeSize(i));

				destruct<Args...>(mArchetype.getReverseTypeIndex(i), ptr);
			}

			// indexを削除
			auto&& itr = std::remove_if(
				mpEntityIDs.begin(),
				mpEntityIDs.end(),
				[&](std::size_t* pIndex)
				{
					if (*pIndex == deallocatedIndex)
					{
						delete pIndex;
						return true;
					}
					else if (*pIndex > deallocatedIndex)
					{
						--(*pIndex);
					}

					return false;
				});
			mpEntityIDs.erase(itr, mpEntityIDs.end());

			// 実際のメモリ領域を移動
			if (mEntityNum > deallocatedIndex + 1)
			{
				std::byte* dst = nullptr, * src = nullptr;
				std::size_t offset = 0;
				for (std::size_t i = 0; i < mArchetype.getTypeCount(); ++i)
				{
					const auto&& typeSize = mArchetype.getTypeSize(i);
					dst = mpMemory + offset + deallocatedIndex * typeSize;
					src = dst + typeSize;  // 後ろに1つずらす
					std::memmove(dst, src, (mEntityNum - deallocatedIndex - 1) * typeSize);
					offset += typeSize * mMaxEntityNum;
				}
			}

			// Entity数更新
			--mEntityNum;

			// 1/3を切ってる場合はメモリを切り詰める
			if (mEntityNum && mEntityNum < (mMaxEntityNum / 3) && mMaxEntityNum > 16)
			{
				// DEBUG!!!!!!!!!
				//std::cerr << "minus realloc : " << mMaxEntityNum / 2 << "\n";

				reallocate(mMaxEntityNum / 2);
			}
		}

		/**
		 * @brief Entityが増えたらメモリを割り当て直す
		 *
		 * @param maxEntityNum 新しい最大Entity数
		 */
		virtual void reallocate(const std::size_t newMaxEntityNum) override
		{
			assert(newMaxEntityNum != mMaxEntityNum);
			assert(newMaxEntityNum >= mEntityNum);

			const auto oldMaxEntityNum = mMaxEntityNum;

			// 新メモリ割当て
			auto&& newMemSize = mArchetype.getAllTypeSize() * newMaxEntityNum;
			std::byte* newMem = new std::byte[mArchetype.getAllTypeSize() * newMaxEntityNum]();
			std::memset(newMem, 0, newMemSize);

			{  // データ移行
				std::size_t newOffset = 0, oldOffset = 0;
				const auto&& typeCount = mArchetype.getTypeCount();
				for (std::size_t i = 0; i < typeCount; ++i)
				{
					const auto&& typeSize = mArchetype.getTypeSize(i);

					// TODO: ここで全Entityに対してコピーコンストラクタを呼ぶ
					if (isTriviallyCopyable<Args...>(mArchetype.getReverseTypeIndex(i)))
					{
						memcpy(newMem + newOffset, mpMemory + oldOffset, typeSize * mEntityNum);
					}
					else
					{
						for (const auto pEntity : mpEntityIDs)
						{
							std::byte* pSrc = mpMemory + oldOffset + *pEntity * typeSize;
							std::byte* pDst = newMem + newOffset + *pEntity * typeSize;

							copyConstruct<Args...>(mArchetype.getReverseTypeIndex(i), pSrc, pDst);
						}
					}

					newOffset += typeSize * newMaxEntityNum;
					oldOffset += typeSize * oldMaxEntityNum;
				}
			}

			// アドレス移行
			delete[] mpMemory;
			mpMemory = newMem;

			mMaxEntityNum = newMaxEntityNum;
		}

		/**
		 * @brief 完全にChunkを破棄する(メモリを全て解放する)
		 *
		 */
		virtual void destroy()
		{
			if (!mpMemory || mpEntityIDs.empty())
			{
				return;
			}

			// ここで全ComponentDataに対してデストラクタを呼ぶ
			for (std::size_t i = 0; i < sizeof...(Args); ++i)
			{
				// 書き込む型までのオフセット
				const std::size_t offset = mArchetype.getTypeOffset(i, mMaxEntityNum);

				for (const auto pEntity : mpEntityIDs)
				{
					std::byte* ptr = mpMemory + offset + *pEntity * mArchetype.getTypeSize(i);
					destruct<Args...>(mArchetype.getReverseTypeIndex(i), ptr);
				}
			}

			delete[] mpMemory;
			mpMemory = nullptr;

			for (auto p : mpEntityIDs)
			{
				delete p;
			}

			mpEntityIDs.clear();
		}

		/**
		 * @brief entityをotherのChunkに移動する
		 *
		 * @param other 移動先Chunk
		 */
		Entity moveTo(const Entity& entity, IChunk& other)
		{
			const std::size_t srcIndex = entity.getID();

			// 移動先Entityをallocate
			Entity rtn = other.allocate();
			const std::size_t dstIndex = rtn.getID();

			std::byte* dst = nullptr, * src = nullptr;
			std::size_t dstOffset = 0, srcOffset = 0;
			std::size_t size = 0;
			if (mArchetype.getAllTypeSize() < other.mArchetype.getAllTypeSize())  // 移動元 < 移動先
			{
				for (std::size_t i = 0; i < sizeof...(Args); ++i)
				{
					srcOffset = mArchetype.getTypeOffset(i, mMaxEntityNum);
					dstOffset = other.mArchetype.getTypeOffset(other.mArchetype.getTypeIndex(mArchetype.getTypeHash(i)), other.mMaxEntityNum);

					size = mArchetype.getTypeSize(i);
					dst = other.mpMemory + dstOffset + dstIndex * size;
					src = mpMemory + srcOffset + srcIndex * size;

					if (isTriviallyCopyable(i))
					{
						std::memcpy(dst, src, size * mEntityNum);
					}
					else
					{
						// 書き込む型までのオフセット
						for (const auto pEntity : mpEntityIDs)
						{
							std::byte* pSrc = src + *pEntity * size;
							std::byte* pDst = dst + *pEntity * size;

							copyConstruct<Args...>(i, pSrc, pDst);
						}
					}


				}
			}
			else  // 移動元 >= 移動先
			{
				for (std::size_t i = 0; i < other.mArchetype.getTypeCount(); ++i)
				{
					srcOffset = other.mArchetype.getTypeOffset(other.mArchetype.getTypeIndex(mArchetype.getTypeHash(i)), other.mMaxEntityNum);
					dstOffset = mArchetype.getTypeOffset(i, mMaxEntityNum);

					size = mArchetype.getTypeSize(i);
					dst = mpMemory + srcOffset + srcIndex * size;
					src = other.mpMemory + dstOffset + dstIndex * size;

					if (isTriviallyCopyable(i))
					{
						std::memcpy(dst, src, size * mEntityNum);
					}
					else
					{
						// 書き込む型までのオフセット
						for (const auto pEntity : mpEntityIDs)
						{
							std::byte* pSrc = src + *pEntity * size;
							std::byte* pDst = dst + *pEntity * size;

							copyConstruct<Args...>(i, pSrc, pDst);
						}
					}
				}
			}

			return rtn;
		}

	private:

		/**
		 * @brief このアドレスに指定した型を配置newする
		 *
		 * @param typeIndex Args...の何番目の型か(Archetype::getReverseTypeIndex()を用いる)
		 * @param ptr 配置newされるアドレス
		 */
		template<typename Head, typename... Tail>
		constexpr void construct(std::size_t typeIndex, std::byte* ptr)
		{

			if (typeIndex == sizeof...(Args) - sizeof...(Tail) - 1)
			{
				if constexpr (!std::is_trivially_constructible_v<Head>)
				{
					new(ptr) Head();
				}

				return;
			}

			if constexpr (sizeof...(Tail) > 0)
			{
				construct<Tail...>(typeIndex, ptr);
			}
		}

		/**
		 * @brief このアドレスで指定した型のデストラクタを呼ぶ
		 * 
		 * @param typeIndex Args...の何番目の型か(Archetype::getReverseTypeIndex()を用いる)
		 * @param ptr
		 */
		template<typename Head, typename... Tail>
		constexpr void destruct(std::size_t typeIndex, std::byte* ptr)
		{

			if (typeIndex == sizeof...(Args) - sizeof...(Tail) - 1)
			{
				if constexpr (!std::is_trivially_destructible_v<Head>)
				{
					reinterpret_cast<Head*>(ptr)->~Head();
				}

				return;
			}

			if constexpr (sizeof...(Tail) > 0)
			{
				destruct<Tail...>(typeIndex, ptr);
			}
		}

		/**
		 * @brief srcからdstへ指定した型でのコピーを行う
		 * 
		 * @param typeIndex Args...の何番目の型か(Archetype::getReverseTypeIndex()を用いる)
		 * @param src コピー元アドレス
		 * @param dst コピー先アドレス
		 */
		template<typename Head, typename... Tail>
		constexpr void copyConstruct(std::size_t typeIndex, std::byte* src, std::byte* dst)
		{
			if (typeIndex == sizeof...(Args) - sizeof...(Tail) - 1)
			{

				new(dst) Head(*reinterpret_cast<Head*>(src));

				return;
			}

			if constexpr (sizeof...(Tail) > 0)
			{
				copyConstruct<Tail...>(typeIndex, src, dst);
			}
		}

		/**
		 * @brief その型がtrivially_copyableかどうか判定する
		 * 
		 * @param typeIndex Args...の何番目の型か(Archetype::getReverseTypeIndex()を用いる)
		 * @return trivially_copyableかどうか
		 */
		template<typename Head, typename... Tail>
		constexpr bool isTriviallyCopyable(std::size_t typeIndex)
		{
			if (typeIndex == sizeof...(Args) - sizeof...(Tail) - 1)
			{
				return std::is_trivially_copyable_v<Head>;
			}

			if constexpr (sizeof...(Tail) > 0)
			{
				isTriviallyCopyable<Tail...>(typeIndex);
			}

			return false;
		}

	};
}  // namespace mvecs

#endif