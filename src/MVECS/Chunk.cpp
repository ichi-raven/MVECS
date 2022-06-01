#include "../include/MVECS/Chunk.hpp"

#include <cstring>
#include <iostream>

namespace mvecs
{
    Chunk::Chunk(const std::size_t ID, const Archetype& archetype)
        : mID(ID)
        , mArchetype(archetype)
        , mMemory(nullptr)
        , mMaxEntityNum(1)
        , mEntityNum(0)
    //, mNextEntityIndex(0)
    {
    }

    Chunk::~Chunk()
    {
    }

    Chunk::Chunk(Chunk&& src)
        : mID(src.mID)
        , mArchetype(src.mArchetype)
        , mMemory(std::move(src.mMemory))
        , mMaxEntityNum(src.mMaxEntityNum)
        , mEntityNum(src.mEntityNum)
    //, mNextEntityIndex(src.mNextEntityIndex)
    {
    }

    Chunk& Chunk::operator=(Chunk&& src)
    {
        mID           = src.mID;
        mArchetype    = src.mArchetype;
        mMemory       = src.mMemory;
        mMaxEntityNum = src.mMaxEntityNum;
        mEntityNum    = src.mEntityNum;
        // mNextEntityIndex = src.mNextEntityIndex;

        delete[] src.mMemory;

        for (auto p : src.mpEntityIDs)
            delete p;

        return *this;
    }

    std::size_t Chunk::getID() const
    {
        return mID;
    }

    const Archetype& Chunk::getArchetype() const
    {
        return mArchetype;
    }

    Entity Chunk::allocate()
    {
        if (mEntityNum + 1 >= mMaxEntityNum)
        {
            // DEBUG!!!!!!!!
            //std::cerr << "plus realloc : " << mMaxEntityNum * 2 << "\n";
            // メモリを再割り当てする
            reallocate(mMaxEntityNum * 2);  // std::vectorの真似
        }

        // アクセスのオフセット
        // std::size_t* memToIndexPart = reinterpret_cast<std::size_t*>(mMemory + mArchetype.getAllTypeSize() * mMaxEntityNum);
        // std::size_t entityIndex = std::numeric_limits<std::size_t>::max();
        // for (std::size_t i = 0; i < mMaxEntityNum; ++i)
        // {
        //     if (memToIndexPart[i] == entityIndex)
        //     {
        //         entityIndex = i;
        //         memToIndexPart[i] = mEntityNum;
        //         break;
        //     }
        // }

        // assert(entityIndex != std::numeric_limits<std::size_t>::max());
        // Entity entity(entityIndex, mID);

        std::size_t* pIndex = new std::size_t;
        *pIndex             = mEntityNum;
        Entity entity(pIndex, mID);

        // 添字を書き込む
        // memToIndexPart[entity.getID()] = mEntityNum;

        // 新しいindexを挿入
        mpEntityIDs.emplace_back(pIndex);

        // 更新
        //++mNextEntityIndex;
        ++mEntityNum;

        return entity;
    }

    void Chunk::deallocate(const Entity& entity)
    {
        // アクセスのオフセット
        // std::size_t* memToIndexPart = reinterpret_cast<std::size_t*>(mMemory + mArchetype.getAllTypeSize() * mMaxEntityNum);

        // 削除した部分のindexをクリア
        // std::size_t deallocatedIndex   = memToIndexPart[entity.getID()];
        std::size_t deallocatedIndex = entity.getID();

        // memToIndexPart[entity.getID()] = std::numeric_limits<std::size_t>::max();

        // 削除した場所以後の割り当てられた全ての添字を書き換える
        // for (std::size_t i = 0; i < mEntityNum; ++i)
        // {
        //     if (memToIndexPart[i] > deallocatedIndex && memToIndexPart[i] != std::numeric_limits<std::size_t>::max())
        //         --memToIndexPart[i];
        //     assert(memToIndexPart[i] >= 0);
        //     // assert(memToIndexPart[i] < mMaxEntityNum);
        // }

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
            std::byte *dst = nullptr, *src = nullptr;
            std::size_t offset = 0;
            for (std::size_t i = 0; i < mArchetype.getTypeCount(); ++i)
            {
                const auto&& typeSize = mArchetype.getTypeSize(i);
                dst                   = mMemory + offset + deallocatedIndex * typeSize;
                src                   = dst + typeSize;  // 後ろに1つずらす
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

    void Chunk::clear()
    {
        constexpr std::size_t maxEntityNum = 1;
        delete[] mMemory;

        // mMemory    = new std::byte[(mArchetype.getAllTypeSize() + sizeof(std::size_t)) * maxEntityNum]();
        mMemory       = new std::byte[mArchetype.getAllTypeSize() * maxEntityNum]();
        mMaxEntityNum = maxEntityNum;

        // indexクリア, 新規要素挿入
        for (auto& p : mpEntityIDs)
            delete p;

        int debug = 0;

        mpEntityIDs.clear();

        std::size_t* pIndex = new std::size_t;
        *pIndex             = 0;
        mpEntityIDs.emplace_back(pIndex);
        mEntityNum = 0;

        // ID-index部のメモリクリア
        // std::memset(mMemory + mArchetype.getAllTypeSize() * maxEntityNum, 0xFF, maxEntityNum * sizeof(std::size_t));
    }

    void Chunk::destroy()
    {
        delete[] mMemory;
        for (auto p : mpEntityIDs)
            delete p;
    }

    Entity Chunk::moveTo(const Entity& entity, Chunk& other)
    {
        // アクセスのオフセット
        // const std::size_t* srcMemToIndexPart = reinterpret_cast<std::size_t*>(mMemory + mArchetype.getAllTypeSize() * mMaxEntityNum);
        // const std::size_t srcIndex           = srcMemToIndexPart[entity.getID()];
        // assert(srcIndex != std::numeric_limits<size_t>::max() || !"this entity was not allocated!");

        const std::size_t srcIndex = entity.getID();

        // 移動先Entityをallocate
        Entity rtn = other.allocate();
        // const std::size_t* dstMemToIndexPart = reinterpret_cast<std::size_t*>(other.mMemory + other.mArchetype.getAllTypeSize() * other.mMaxEntityNum);
        // const std::size_t dstIndex           = dstMemToIndexPart[rtn.getID()];
        const std::size_t dstIndex = rtn.getID();

        std::byte *dst = nullptr, *src = nullptr;
        std::size_t dstOffset = 0, srcOffset = 0;
        std::size_t size = 0;
        if (mArchetype.getAllTypeSize() < other.mArchetype.getAllTypeSize())  // 移動元 < 移動先
        {
            for (std::size_t i = 0; i < mArchetype.getTypeCount(); ++i)
            {
                srcOffset = mArchetype.getTypeOffset(i, mMaxEntityNum);
                dstOffset = other.mArchetype.getTypeOffset(other.mArchetype.getTypeIndex(mArchetype.getTypeHash(i)), other.mMaxEntityNum);

                size = mArchetype.getTypeIndex(i);
                dst  = other.mMemory + dstOffset + dstIndex * size;
                src  = mMemory + srcOffset + srcIndex * size;

                std::memcpy(dst, src, size);
            }
        }
        else  // 移動元 >= 移動先
        {
            for (std::size_t i = 0; i < other.mArchetype.getTypeCount(); ++i)
            {
                srcOffset = other.mArchetype.getTypeOffset(other.mArchetype.getTypeIndex(mArchetype.getTypeHash(i)), other.mMaxEntityNum);
                dstOffset = mArchetype.getTypeOffset(i, mMaxEntityNum);

                size = mArchetype.getTypeIndex(i);
                dst  = mMemory + srcOffset + srcIndex * size;
                src  = other.mMemory + dstOffset + dstIndex * size;

                std::memcpy(dst, src, size);
            }
        }

        return rtn;
    }

    void Chunk::reallocate(const std::size_t newMaxEntityNum)
    {
        assert(newMaxEntityNum != mMaxEntityNum);
        assert(newMaxEntityNum >= mEntityNum);

        const auto oldMaxEntityNum = mMaxEntityNum;

        // 新メモリ割当て
        // std::byte* newMem = new std::byte[(mArchetype.getAllTypeSize() + sizeof(std::size_t)) * newMaxEntityNum];
        auto&& newMemSize = mArchetype.getAllTypeSize() * newMaxEntityNum;
        std::byte* newMem = new std::byte[mArchetype.getAllTypeSize() * newMaxEntityNum]();
        std::memset(newMem, 0, newMemSize);

        {  // データ移行
            std::size_t newOffset = 0, oldOffset = 0;
            const auto&& typeCount = mArchetype.getTypeCount();
            for (std::size_t i = 0; i < typeCount; ++i)
            {
                const auto&& typeSize = mArchetype.getTypeSize(i);
                std::memcpy(newMem + newOffset, mMemory + oldOffset, typeSize * mEntityNum);

                newOffset += typeSize * newMaxEntityNum;
                oldOffset += typeSize * oldMaxEntityNum;
            }

            // ID-indexテーブル部
            // if (oldMaxEntityNum < newMaxEntityNum)  // 確保領域を増やす場合(index部をクリアする必要がある)
            // {
            //     std::memcpy(newMem + newOffset, mMemory + oldOffset, sizeof(std::size_t) * oldMaxEntityNum);
            //     // ID-index部の新規のメモリクリア
            //     const auto&& newOffsetToIndex = (mArchetype.getAllTypeSize() * newMaxEntityNum) + (sizeof(std::size_t) * oldMaxEntityNum);
            //     std::memset(newMem + newOffsetToIndex, 0xFF, sizeof(std::size_t) * (newMaxEntityNum - oldMaxEntityNum));
            // }
            // else  // 減らす場合
            // {
            //     std::memcpy(newMem + newOffset, mMemory + oldOffset, sizeof(std::size_t) * newMaxEntityNum);
            // }
        }

        // アドレス移行
        delete[] mMemory;
        mMemory = newMem;

        // // 更新
        // if (oldMaxEntityNum > newMaxEntityNum)  // 確保領域を減らす場合は次のEntityに割り当てるIDを戻す必要がある
        //     mNextEntityIndex =

        mMaxEntityNum = newMaxEntityNum;
    }

    std::size_t Chunk::getEntityNum() const
    {
        return mEntityNum;
    }

    void Chunk::dumpMemory() const
    {
        const std::byte* const p = mMemory;
        for (std::size_t typeIdx = 0; typeIdx < mArchetype.getTypeCount(); ++typeIdx)
        {
            std::cerr << "type begin----------\n";
            const std::size_t offset = mArchetype.getTypeOffset(typeIdx, mMaxEntityNum);
            const std::size_t all    = offset + mArchetype.getTypeSize(typeIdx) * mMaxEntityNum;
            for (std::size_t i = offset; i < all; ++i)
            {
                if (i % 4 == 0)
                    std::cerr << std::dec << i << " ~ " << i + 3 << " : ";
                std::cerr << std::hex << static_cast<int>(p[i]) << " ";
                if (i % 4 == 3)
                    std::cerr << "\n";
            }
            std::cerr << "type end----------\n";
        }
        std::cerr << std::dec;
    }

    void Chunk::dumpIndexMemory() const
    {
        // // アクセスのオフセット
        // const std::size_t* const memToIndexPart = reinterpret_cast<std::size_t*>(mMemory + mArchetype.getAllTypeSize() * mMaxEntityNum);

        // for (std::size_t i = 0; i < mMaxEntityNum; ++i)
        //     std::cerr << "[" << i << "] : " << memToIndexPart[i] << "\n";

        for (std::size_t i = 0; i < mpEntityIDs.size(); ++i)
        {
            std::cerr << "[" << i << "] : " << *mpEntityIDs[i] << "\n";
        }
    }

    void Chunk::insertEntityIndex(std::size_t* pIndex)
    {
        auto&& iter = std::lower_bound(mpEntityIDs.begin(), mpEntityIDs.end(), pIndex, [](const std::size_t* left, const std::size_t* right)
                                       { return *left < *right; });
        if (iter == mpEntityIDs.end())
        {
            mpEntityIDs.emplace_back(pIndex);
            iter = mpEntityIDs.end() - 1;
        }
        else
            iter = mpEntityIDs.insert(iter, pIndex);
    }

}  // namespace mvecs