#include "../include/MVECS/IChunk.hpp"

#include <cstring>
#include <iostream>

namespace mvecs
{
	IChunk::IChunk(const std::size_t ID, const Archetype& archetype)
		: mID(ID)
		, mArchetype(archetype)
		, mpMemory(nullptr)
		, mMaxEntityNum(1)
		, mEntityNum(0)
	{
	}

	IChunk::~IChunk()
	{
	}

	IChunk::IChunk(IChunk&& src) noexcept
		: mID(src.mID)
		, mArchetype(src.mArchetype)
		, mpMemory(std::move(src.mpMemory))
		, mMaxEntityNum(src.mMaxEntityNum)
		, mEntityNum(src.mEntityNum)
	{
	}

	IChunk& IChunk::operator=(IChunk&& src) noexcept

	{
		mID = src.mID;
		mArchetype = src.mArchetype;
		mpMemory = src.mpMemory;
		mMaxEntityNum = src.mMaxEntityNum;
		mEntityNum = src.mEntityNum;

		src.destroy();

		return *this;
	}

	std::size_t IChunk::getID() const
	{
		return mID;
	}

	const Archetype& IChunk::getArchetype() const
	{
		return mArchetype;
	}

	//Entity Chunk::allocate()
	//{
	//	if (mEntityNum + 1 >= mMaxEntityNum)
	//	{
	//		// DEBUG!!!!!!!!
	//		//std::cerr << "plus realloc : " << mMaxEntityNum * 2 << "\n";
	//		// メモリを再割り当てする
	//		reallocate(mMaxEntityNum * 2);  // std::vectorの真似
	//	}

	//	std::size_t* pIndex = new std::size_t;
	//	*pIndex = mEntityNum;
	//	Entity entity(pIndex, mID);

	//	// 構築
	//	{
	//	//    using T = // ここで型取得
	//	//    // 書き込む型までのオフセット
	//	//    const std::size_t offset = mArchetype.getTypeOffset(mArchetype.getTypeIndex<T>(), mMaxEntityNum);

	//	//    T* p = reinterpret_cast<T*>(mpMemory + offset + entity.getID() * sizeof(T));
	//	//    new(p) T;
	//	}

	//	// 新しいindexを挿入
	//	mpEntityIDs.emplace_back(pIndex);

	//	// 更新
	//	++mEntityNum;

	//	return entity;
	//}

	//void Chunk::deallocate(const Entity& entity)
	//{
	//	std::size_t deallocatedIndex = entity.getID();

	//	// 破棄
	//	{
	//		//using T = // ここで型取得
	//		// 書き込む型までのオフセット
	//		//const std::size_t offset = mArchetype.getTypeOffset(mArchetype.getTypeIndex<T>(), mMaxEntityNum);

	//		//T* p = reinterpret_cast<T*>(mpMemory + offset + entity.getID() * sizeof(T));
	//		//p->~T();
	//	}

	//	// indexを削除
	//	auto&& itr = std::remove_if(
	//		mpEntityIDs.begin(),
	//		mpEntityIDs.end(),
	//		[&](std::size_t* pIndex)
	//		{
	//			if (*pIndex == deallocatedIndex)
	//			{
	//				delete pIndex;
	//				return true;
	//			}
	//			else if (*pIndex > deallocatedIndex)
	//			{
	//				--(*pIndex);
	//			}
	//			return false;
	//		});
	//	mpEntityIDs.erase(itr, mpEntityIDs.end());

	//	// 実際のメモリ領域を移動
	//	if (mEntityNum > deallocatedIndex + 1)
	//	{
	//		std::byte* dst = nullptr, * src = nullptr;
	//		std::size_t offset = 0;
	//		for (std::size_t i = 0; i < mArchetype.getTypeCount(); ++i)
	//		{
	//			const auto&& typeSize = mArchetype.getTypeSize(i);
	//			dst = mpMemory + offset + deallocatedIndex * typeSize;
	//			src = dst + typeSize;  // 後ろに1つずらす
	//			std::memmove(dst, src, (mEntityNum - deallocatedIndex - 1) * typeSize);
	//			offset += typeSize * mMaxEntityNum;
	//		}
	//	}

	//	// Entity数更新
	//	--mEntityNum;

	//	// 1/3を切ってる場合はメモリを切り詰める
	//	if (mEntityNum && mEntityNum < (mMaxEntityNum / 3) && mMaxEntityNum > 16)
	//	{
	//		// DEBUG!!!!!!!!!
	//		//std::cerr << "minus realloc : " << mMaxEntityNum / 2 << "\n";

	//		reallocate(mMaxEntityNum / 2);
	//	}
	//}

	//void Chunk::destroy()
	//{
	//	// ここで全ComponentDataに対してデストラクタを呼ぶ
	//	{

	//	}

	//	delete[] mpMemory;
	//	for (auto p : mpEntityIDs)
	//	{
	//		delete p;
	//	}
	//}

	void IChunk::clear()
	{
		// TODO:どの程度だとパフォーマンスが良いのか(対して変わらないと思う)
		constexpr std::size_t maxEntityNum = 1;

		destroy();

		mpMemory = new std::byte[mArchetype.getAllTypeSize() * maxEntityNum]();
		mMaxEntityNum = maxEntityNum;

		mpEntityIDs.clear();

		mpEntityIDs.emplace_back(new std::size_t(0));
		mEntityNum = 0;
	}

	//Entity Chunk::moveTo(const Entity& entity, IChunk& other)
	//{
	//	const std::size_t srcIndex = entity.getID();

	//	// 移動先Entityをallocate
	//	Entity rtn = other.allocate();
	//	const std::size_t dstIndex = rtn.getID();

	//	std::byte* dst = nullptr, * src = nullptr;
	//	std::size_t dstOffset = 0, srcOffset = 0;
	//	std::size_t size = 0;
	//	if (mArchetype.getAllTypeSize() < other.mArchetype.getAllTypeSize())  // 移動元 < 移動先
	//	{
	//		for (std::size_t i = 0; i < mArchetype.getTypeCount(); ++i)
	//		{
	//			srcOffset = mArchetype.getTypeOffset(i, mMaxEntityNum);
	//			dstOffset = other.mArchetype.getTypeOffset(other.mArchetype.getTypeIndex(mArchetype.getTypeHash(i)), other.mMaxEntityNum);

	//			size = mArchetype.getTypeIndex(i);
	//			dst = other.mpMemory + dstOffset + dstIndex * size;
	//			src = mpMemory + srcOffset + srcIndex * size;

	//			// TODO:ここで全ComponentDataに対してコピーコンストラクタを呼ぶ
	//			std::memcpy(dst, src, size);
	//		}
	//	}
	//	else  // 移動元 >= 移動先
	//	{
	//		for (std::size_t i = 0; i < other.mArchetype.getTypeCount(); ++i)
	//		{
	//			srcOffset = other.mArchetype.getTypeOffset(other.mArchetype.getTypeIndex(mArchetype.getTypeHash(i)), other.mMaxEntityNum);
	//			dstOffset = mArchetype.getTypeOffset(i, mMaxEntityNum);

	//			size = mArchetype.getTypeIndex(i);
	//			dst = mpMemory + srcOffset + srcIndex * size;
	//			src = other.mpMemory + dstOffset + dstIndex * size;

	//			// TODO:ここで全ComponentDataに対してコピーコンストラクタを呼ぶ
	//			{
	//				std::memcpy(dst, src, size);
	//			}
	//		}
	//	}

	//	return rtn;
	//}

	//void Chunk::reallocate(const std::size_t newMaxEntityNum)
	//{
	//	assert(newMaxEntityNum != mMaxEntityNum);
	//	assert(newMaxEntityNum >= mEntityNum);

	//	const auto oldMaxEntityNum = mMaxEntityNum;

	//	// 新メモリ割当て
	//	auto&& newMemSize = mArchetype.getAllTypeSize() * newMaxEntityNum;
	//	std::byte* newMem = new std::byte[mArchetype.getAllTypeSize() * newMaxEntityNum]();
	//	std::memset(newMem, 0, newMemSize);

	//	{  // データ移行
	//		std::size_t newOffset = 0, oldOffset = 0;
	//		const auto&& typeCount = mArchetype.getTypeCount();
	//		for (std::size_t i = 0; i < typeCount; ++i)
	//		{
	//			const auto&& typeSize = mArchetype.getTypeSize(i);

	//			// TODO: ここで全ComponentDataに対してコピーコンストラクタを呼ぶ
	//			std::memcpy(newMem + newOffset, mpMemory + oldOffset, typeSize * mEntityNum);

	//			newOffset += typeSize * newMaxEntityNum;
	//			oldOffset += typeSize * oldMaxEntityNum;
	//		}
	//	}

	//	// アドレス移行
	//	delete[] mpMemory;
	//	mpMemory = newMem;

	//	mMaxEntityNum = newMaxEntityNum;
	//}

	std::size_t IChunk::getEntityNum() const
	{
		return mEntityNum;
	}

	void IChunk::dumpMemory() const
	{
		const std::byte* const p = mpMemory;
		for (std::size_t typeIdx = 0; typeIdx < mArchetype.getTypeCount(); ++typeIdx)
		{
			std::cerr << "type begin----------\n";
			const std::size_t offset = mArchetype.getTypeOffset(typeIdx, mMaxEntityNum);
			const std::size_t all = offset + mArchetype.getTypeSize(typeIdx) * mMaxEntityNum;
			for (std::size_t i = offset; i < all; ++i)
			{
				if (i % 4 == 0)
				{
					std::cerr << std::dec << i << " ~ " << i + 3 << " : ";
				}

				std::cerr << std::hex << static_cast<int>(p[i]) << " ";

				if (i % 4 == 3)
				{
					std::cerr << "\n";
				}
			}
			std::cerr << "type end----------\n";
		}

		// もどす
		std::cerr << std::dec;
	}

	void IChunk::dumpIndexMemory() const
	{
		for (std::size_t i = 0; i < mpEntityIDs.size(); ++i)
		{
			std::cerr << "[" << i << "] : " << *mpEntityIDs[i] << "\n";
		}
	}

	//void Chunk::insertEntityIndex(std::size_t* pIndex)
	//{
	//	auto&& iter = std::lower_bound(mpEntityIDs.begin(), mpEntityIDs.end(), pIndex, [](const std::size_t* left, const std::size_t* right)
	//		{ return *left < *right; });
	//	if (iter == mpEntityIDs.end())
	//	{
	//		mpEntityIDs.emplace_back(pIndex);
	//		iter = mpEntityIDs.end() - 1;
	//	}
	//	else
	//	{
	//		iter = mpEntityIDs.insert(iter, pIndex);
	//	}
	//}

}  // namespace mvecs