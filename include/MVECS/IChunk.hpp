#pragma once
#ifndef MVECS_MVECS_CHUNK_HPP_
#define MVECS_MVECS_CHUNK_HPP_

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <memory>
#include <vector>

#include "Archetype.hpp"
#include "ComponentArray.hpp"
#include "Entity.hpp"

/**
 * @brief mvecs
 *
 */
namespace mvecs
{
    /**
     * @brief Chunk�N���X�̃C���^�t�F�[�X ComponentData��
     *
     */
    class IChunk
    {
    public:
        /**
         * @brief �^����ꂽArchetype����Chunk���\�z����
         *
         * @param entitySize Chunk������Entity�̍ő吔
         * @return Chunk �\�z����Chunk
         */
        static IChunk create(const std::size_t ID, const Archetype& archetype, const std::size_t maxEntityNum = 1)
        {

            IChunk rtn(ID, archetype);
            //rtn.mMemory = new std::byte[(archetype.getAllTypeSize() + sizeof(std::size_t)) * maxEntityNum]();
            std::size_t memSize = archetype.getAllTypeSize() * maxEntityNum;
            rtn.mMemory = new std::byte[memSize]();
            // �������N���A
            std::memset(rtn.mMemory, 0, memSize);
            rtn.mMaxEntityNum = maxEntityNum;
            assert(rtn.mMaxEntityNum != 0);

            // ID-index���̃������N���A
            // std::memset(rtn.mMemory + archetype.getAllTypeSize() * maxEntityNum, 0xFF, maxEntityNum * sizeof(std::size_t));

            return rtn;
        }

        /**
         * @brief �R���X�g���N�^
         *
         * @param ID
         */
        IChunk(const std::size_t ID, const Archetype& archetype);

        /**
         * @brief �f�X�g���N�^
         *
         */
        ~IChunk();

        /**
         * @brief �R�s�[�R���X�g���N�^��delete
         *
         * @param src
         */
        IChunk(IChunk& src) = delete;

        /**
         * @brief ����ɂ��R�s�[��delete
         *
         * @param src
         * @return Chunk&
         */
        IChunk& operator=(const IChunk& src) = delete;

        /**
         * @brief ���[�u�R���X�g���N�^
         *
         * @param chunk ���[�u��
         */
        IChunk(IChunk&& src);

        /**
         * @brief ���[�u�R���X�g���N�^ ���Z�q�I�[�o�[���[�h
         *
         * @param src
         * @return Chunk&
         */
        IChunk& operator=(IChunk&& src);

        /**
         * @brief Chunk��ID���擾����
         *
         * @return std::size_t ID
         */
        std::size_t getID() const;

        /**
         * @brief Archetype���擾����
         *
         * @return Archetype
         */
        const Archetype& getArchetype() const;

        /**
         * @brief Entity�̗̈���m�ۂ���
         * @details ���ۂ̒l�͉����������܂�Ă��Ȃ��̂Œ���
         * @return Entity
         */
        virtual Entity allocate();

        /**
         * @brief �w�肵��Entity���폜���ă��������l�߂�
         *
         * @param entity �폜����Entity
         */
        virtual void deallocate(const Entity& entity);

        /**
         * @brief �������S�̂��N���A���Asize=1�ɂ���
         *
         */
        void clear();

        /**
         * @brief ���S��Chunk��j������(��������S�ĉ������)
         *
         */
        void destroy();

        /**
         * @brief entity��other��Chunk�Ɉړ�����
         *
         * @param other �ړ���Chunk
         */
        Entity moveTo(const Entity& entity, Chunk& other);

        /**
         * @brief ComponentData�̒l����������
         *
         * @tparam T ComponentData�̌^
         * @param entity �������ݐ�Entity
         * @param value ��������ComponentData
         */
        template <typename T, typename = std::enable_if_t<IsComponentDataType<T>>>
        void setComponentData(const Entity& entity, const T& value)
        {
            assert(mArchetype.isIn<T>() || !"T is not in Archetype");
            // ID-index�e�[�u��
            // const std::size_t* memToIndexPart = reinterpret_cast<std::size_t*>(mMemory + mArchetype.getAllTypeSize() * mMaxEntityNum);
            // const std::size_t index           = memToIndexPart[entity.getID()];

            // assert(index != std::numeric_limits<size_t>::max() || !"this entity was not allocated!");
            assert(entity.getID() <= mEntityNum || !"invalid entity ID!");

            // �������ތ^�܂ł̃I�t�Z�b�g
            const std::size_t offset = mArchetype.getTypeOffset(mArchetype.getTypeIndex<T>(), mMaxEntityNum);

            // ��������
            std::memcpy(mMemory + offset + entity.getID() * sizeof(T), &value, sizeof(T));
        }

        /**
         * @brief  ComponentData�̒l���擾����
         *
         * @tparam T �擾����ComponentData�̌^
         * @tparam typename ComponentData�^�����肷��
         * @param entity �擾���Entity
         * @return �擾�����l
         */
        template <typename T, typename = std::enable_if_t<IsComponentDataType<T>>>
        T& getComponentData(const Entity& entity) const
        {
            assert(mArchetype.isIn<T>() || !"T is not in Archetype");
            // const std::size_t* memToIndexPart = reinterpret_cast<std::size_t*>(mMemory + mArchetype.getAllTypeSize() * mMaxEntityNum);
            // const std::size_t index           = memToIndexPart[entity.getID()];
            // assert(index != std::numeric_limits<size_t>::max() || !"this entity was not allocated!");

            // �������ތ^�܂ł̃I�t�Z�b�g
            std::size_t offset = mArchetype.getTypeOffset(mArchetype.getTypeIndex<T>(), mMaxEntityNum);

            // �ǂݏo��
            // return *reinterpret_cast<T*>(mMemory + offset + index * sizeof(T));
            return *reinterpret_cast<T*>(mMemory + offset + entity.getID() * sizeof(T));
        }

        /**
         * @brief Entity���������烁���������蓖�Ē���
         *
         * @param maxEntityNum �V�����ő�Entity��
         */
        void reallocate(const std::size_t maxEntityNum);

        /**
         * @brief �w�肵���^��ComponentArray���擾����
         * @details �n���ꂽ�A�h���X�͖����ɂȂ�\�������邽�ߑ���ɂ͒��ӂ���
         * @tparam T ComponentData�̌^
         * @tparam typename ComponentData�^����p
         * @return ComponentArray<T>
         */
        template <typename T, typename = std::enable_if_t<IsComponentDataType<T>>>
        ComponentArray<T> getComponentArray() const
        {
            assert(mArchetype.isIn<T>() || !"T is not in Archetype");

            // �g�p����^�܂ł̃I�t�Z�b�g
            std::size_t offset = mArchetype.getTypeOffset(mArchetype.getTypeIndex<T>(), mMaxEntityNum);

            return ComponentArray<T>(reinterpret_cast<T*>(mMemory + offset), mEntityNum);
        }

        /**
         * @brief ���݂�Entity�̌����擾����
         *
         * @return std::size_t ��
         */
        std::size_t getEntityNum() const;

        /**
         * @brief �������_���v
         *
         */
        void dumpMemory() const;

        /**
         * @brief Entity��ID�Ǝ��ۂ̃�������Ή��Â��Ă��镔�����_���v����
         *
         */
        void dumpIndexMemory() const;

    private:

        void insertEntityIndex(std::size_t* pIndex);

        //! Chunk��ID
        std::size_t mID;

        //! ���ƂƂȂ�Archetype
        Archetype mArchetype;
        //! �������A�h���X
        std::byte* mMemory;
        //! ���蓖�Ă���ő��Entity��
        std::size_t mMaxEntityNum;
        //! ���݂�Entity��
        std::size_t mEntityNum;
        //! Entity�Ɋ��蓖�Ă�e�[�u���̓Y��(ID)
        // std::size_t mNextEntityIndex;

        //!  ���蓖�Ă�Entity������ID�A�h���X(destroy�ɉ����ď���������)
        std::vector<std::size_t*> mpEntityIDs;
    };
}  // namespace mvecs

#endif