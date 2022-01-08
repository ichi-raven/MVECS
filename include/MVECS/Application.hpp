#ifndef MVECS_MVECS_APPLICATION_HPP_
#define MVECS_MVECS_APPLICATION_HPP_

#include <unordered_map>
#include <cassert>

namespace mvecs
{
    template<typename Key, typename Common>
    class World;

    /**
     * @brief World遷移とWorld間共有オブジェクトなどを管理する最上位存在
     */
    template <typename Key, typename Common>
    class Application
    {
    public:
        /**
         * @brief コンストラクタ
         *
         */
        Application()
            : mCurrent(nullptr)
            , mEnded(false)
            , mInitialized(false)
        {
        }

        /**
         * @brief Worldを追加する
         *
         * @param key キー
         * @param world World
         */
        World<Key, Common>& add(const Key& key)
        {
            return mWorlds.emplace(key, this);
        }

        /**
         * @brief 指定したWorldへの参照を取得する
         * 
         * @param key 
         * @return std::unique_ptr<World<Key, Common>>& 
         */
        World<Key, Common>& get(const Key& key)
        {
            return mWorlds.at(key);
        }

        /**
         * @brief 示したキーのworldに変更する
         *
         * @param key worldのキー
         * @param reset trueならworldの現在の状態を削除、falseなら保持する
         */
        void change(const Key& key, bool reset = true)
        {
            auto&& iter = mWorlds.find(key);
            assert(iter != mWorlds.end()|| !"invalid world key!");

            if (reset)
                mCurrent->end();

            mCurrent = iter->second;
            mCurrent->init();
        }

        /**
         * @brief 初期化(change呼ぶだけ)
         * 
         * @param key 開始するWorldのキー
         */
        void initialize(const Key& key)
        {
            change(key);
            mInitialized = true;
        }

        /**
         * @brief 全体の更新を行う
         *
         * @return true 終了
         * @return false 終了しない
         */
        void update()
        {
            assert(mInitialized || !"application was not initialized yet!");
            assert(mCurrent || !"world is not registered yet!");

            mCurrent->update();
        }

        /**
         * @brief 終了する
         *
         */
        void dispatchEnd()
        {
            mEnded = true;
            mCurrent->end();
            mCurrent = nullptr;
        }

        /**
         * @brief 終了したかどうかを判定する
         * 
         */
        bool ended()
        {
            return mEnded;
        }

        /**
         * @brief 共有領域を取得する
         * 
         * @return Common& 
         */
        Common& common()
        {
            return mCommon;
        }

    private:
        using umap = std::unordered_map<Key, World<Key, Common>>;
        umap mWorlds;

        World<Key, Common>* mCurrent;

        Common mCommon;

        bool mEnded;
        bool mInitialized;
    };
}  // namespace mvecs

#endif