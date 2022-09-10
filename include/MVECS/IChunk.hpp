#pragma once
#ifndef MVECS_MVECS_ICHUNK_HPP_
#define MVECS_MVECS_ICHUNK_HPP_

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
         * @brief �R���X�g���N�^
         *
         * @param ID
         */
        IChunk(const std::size_t ID, const Archetype& archetype);

        /**
         * @brief �f�X�g���N�^
         *
         */
        virtual ~IChunk();

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
         * @return IChunk&
         */
        IChunk& operator=(const IChunk& src) = delete;

        /**
         * @brief ���[�u�R���X�g���N�^
         *
         * @param src ���[�u��
         */
        IChunk(IChunk&& src) noexcept;

        /**
         * @brief ���[�u�R���X�g���N�^ ���Z�q�I�[�o�[���[�h
         *
         * @param src
         * @return IChunk&
         */
        IChunk& operator=(IChunk&& src) noexcept;

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
        virtual Entity allocate() = 0;

        /**
         * @brief �w�肵��Entity���폜���ă��������l�߂�
         *
         * @param entity �폜����Entity
         */
        virtual void deallocate(const Entity& entity) = 0;

        /**
         * @brief �������S�̂��N���A���Asize=1�ɂ���
         *
         */
        void clear();

        /**
         * @brief ���S��Chunk��j������(��������S�ĉ������)
         *
         */
        virtual void destroy() = 0;

        /**
         * @brief entity��other��Chunk�Ɉړ�����
         *
         * @param other �ړ���Chunk
         */
        //Entity moveTo(const Entity& entity, Chunk& other);

        /**
         * @brief ComponentData�̒l����������
         *
         * @tparam T ComponentData�̌^
         * @param entity �������ݐ�Entity
         * @param value ��������ComponentData
         */
        //template <typename T, typename = std::enable_if_t<IsComponentDataType<T>>>
        //void setComponentData(const Entity& entity, const T& value)
        //{
        //    assert(mArchetype.isIn<T>() || !"T is not in Archetype");
        //    assert(entity.getID() <= mEntityNum || !"invalid entity ID!");

        //    // �������ތ^�܂ł̃I�t�Z�b�g
        //    const std::size_t offset = mArchetype.getTypeOffset(mArchetype.getTypeIndex<T>(), mMaxEntityNum);

        //    // ��������
        //    std::memcpy(mpMemory + offset + entity.getID() * sizeof(T), &value, sizeof(T));
        //}

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

            // �������ތ^�܂ł̃I�t�Z�b�g
            std::size_t offset = mArchetype.getTypeOffset(mArchetype.getTypeIndex<T>(), mMaxEntityNum);

            // �ǂݏo��
            return *(reinterpret_cast<T*>(mpMemory + offset + entity.getID() * sizeof(T)));
        }

        /**
         * @brief Entity���������烁���������蓖�Ē���
         *
         * @param maxEntityNum �V�����ő�Entity��
         */
        virtual void reallocate(const std::size_t maxEntityNum) = 0;

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

            return ComponentArray<T>(reinterpret_cast<T*>(mpMemory + offset), mEntityNum);
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

    protected:

        //void insertEntityIndex(std::size_t* pIndex);

        //! Chunk��ID
        std::size_t mID;

        //! ���ƂƂȂ�Archetype
        Archetype mArchetype;
        //! �������A�h���X
        std::byte* mpMemory;
        //! ���蓖�Ă���ő��Entity��
        std::size_t mMaxEntityNum;
        //! ���݂�Entity��
        std::size_t mEntityNum;

        //!  ���蓖�Ă�Entity������ID�A�h���X(destroy�ɉ����ď���������)
        std::vector<std::size_t*> mpEntityIDs;
    };
}  // namespace mvecs

#endif