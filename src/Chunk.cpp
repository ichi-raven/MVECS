#include "../include/Chunk.hpp"

#include <cstring>
#include <iostream>

namespace mvecs
{
    Chunk::Chunk(std::size_t ID, Archetype archetype)
        : mID(ID)
        , mArchetype(archetype)
        , mMaxEntityNum(1)
        , mEntityNum(0)
        , mNextEntityIndex(0)
    {
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
        if (mEntityNum >= mMaxEntityNum)
        {
            // メモリを再割り当てする
            reallocate(mMaxEntityNum * 2);  // std::vectorの真似
        }

        Entity entity(mNextEntityIndex, mID);

        // アクセスのオフセット
        const std::size_t&& memOffset = mArchetype.getAllTypeSize() * mMaxEntityNum;
        std::size_t* memToIndexPart   = reinterpret_cast<std::size_t*>(mMemory.get() + memOffset);

        // WARNING : DEBUG!!!!
        // {
        //     std::cerr << "allocated index : " << entity.getID() << "\n";
        //     std::cerr << "before-----------\n";
        //     for (std::size_t i = 0; i < mNextEntityIndex; ++i)
        //     {
        //         std::cerr << memToIndexPart[i] << "\n";
        //     }
        //     std::cerr << "\n\n";
        // }

        // 添字を書き込む
        memToIndexPart[entity.getID()] = mEntityNum;

        // // 更新(for DEBUG)
        // ++mNextEntityIndex;
        // ++mEntityNum;

        // WARNING : DEBUG!!!!
        // {
        //     std::cerr << "after------------\n";
        //     for (std::size_t i = 0; i < mMaxEntityNum; ++i)
        //     {
        //         std::cerr << memToIndexPart[i] << "\n";
        //     }
        //     std::cerr << "\n\n";
        // }

        // // 更新
        ++mNextEntityIndex;
        ++mEntityNum;

        return entity;
    }

    void Chunk::deallocate(const Entity& entity)
    {
        // アクセスのオフセット
        const std::size_t&& memOffset = mArchetype.getAllTypeSize() * mMaxEntityNum;
        std::size_t* memToIndexPart   = reinterpret_cast<std::size_t*>(mMemory.get() + memOffset);

        // 削除した部分のindexをクリア
        std::size_t deallocatedIndex   = memToIndexPart[entity.getID()];
        memToIndexPart[entity.getID()] = std::numeric_limits<std::size_t>::max();

        // 削除した場所以後の全ての添字を書き換える
        for (std::size_t i = 1; i < mEntityNum; ++i)
            if (--memToIndexPart[i] < 0)  // 書き込み
                assert(!"invalid index!");

        // 実際のメモリ領域を移動
        if (mEntityNum > deallocatedIndex)
        {
            std::byte *dst = nullptr, *src = nullptr;
            std::size_t offset = 0;
            for (std::size_t i = 0; i < mArchetype.getTypeCount(); ++i)
            {
                auto&& typeSize = mArchetype.getTypeSize(i);
                dst             = mMemory.get() + offset + deallocatedIndex * typeSize;
                src             = dst + typeSize;  // 後ろに1つずらす
                std::memmove(dst, src, (mEntityNum - deallocatedIndex - 1) * typeSize);
                offset += mMaxEntityNum * typeSize;
            }
        }

        // Entity数更新
        --mEntityNum;

        if (mEntityNum <= mMaxEntityNum / 2)
          reallocate(mMaxEntityNum / 2);
    }

    void Chunk::reallocate(std::size_t newMaxEntityNum)
    {
        assert(newMaxEntityNum != mMaxEntityNum);
        assert(newMaxEntityNum >= mEntityNum);

        // 新メモリ割当て
        std::byte* newMem = new std::byte[(mArchetype.getAllTypeSize() + sizeof(std::size_t)) * newMaxEntityNum];

        {  // データ移行
            size_t newOffset = 0, oldOffset = 0;
            size_t typeSize = 0;
            const auto& oldMaxEntityNum = mMaxEntityNum;
            for (size_t i = 0; i < mArchetype.getTypeCount(); ++i)
            {
                // DEBUG!!!
                //std::cerr << "DEBUG : " << i << "\n";
                std::memcpy(newMem + newOffset, mMemory.get() + oldOffset, mArchetype.getTypeSize(i) * mEntityNum);

                typeSize = mArchetype.getTypeSize(i);
                newOffset += typeSize * newMaxEntityNum;
                oldOffset += typeSize * oldMaxEntityNum;
            }

            // ID-indexテーブル部
            std::memcpy(newMem + newOffset, mMemory.get() + oldOffset, sizeof(std::size_t) * oldMaxEntityNum);
        }

        // アドレス移行
        mMemory.reset(newMem);
        mMaxEntityNum = newMaxEntityNum;
    }

}  // namespace mvecs