
//前方宣言のみ
namespace mvecs
{
    template <typename Key, typename Common>
    class Application;

    template <typename Key, typename Common>
    class Scene;
}  // namespace mvecs

#ifndef MVECS_MVECS_APPLICATION_HPP_
#define MVECS_MVECS_APPLICATION_HPP_

#include <Cutlass/Cutlass.hpp>
#include <cassert>
#include <exception>
#include <functional>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>

namespace mvecs
{
//自作Sceneの定義にはこれを使ってください
//ヘッダに書けばオーバーロードすべき関数の定義はすべて完了します
#define GEN_SCENE(SCENE_TYPE, KEY_TYPE, COMMON_TYPE)                                                                                         \
public:                                                                                                                                      \
    SCENE_TYPE(mvecs::Application<KEY_TYPE, COMMON_TYPE>* application, std::shared_ptr<COMMON_TYPE> Common) : IScene(application, common) {} \
    virtual ~SCENE_TYPE() override;                                                                                                          \
    virtual void init() override;                                                                                                            \
    virtual void update() override;                                                                                                          \
                                                                                                                                             \
private:


    // 使うかどうかわからない

    template <typename Key, typename Common>
    class IScene
    {
    private:  // using宣言部
        using Application_t = Application<Key, Common>;

    public:  //メソッド宣言部
        IScene() = delete;

        IScene(
            Application_t* application,
            std::shared_ptr<Common> common,
            std::shared_ptr<Cutlass::Context> context)
            : mApplication(application)
            , mCommon(common)
            , mSceneChanged(false)
        {
        }

        virtual ~IScene(){};

        virtual void init() = 0;

        virtual void update() = 0;

        inline void initAll()
        {
            init();
        }

        inline void updateAll()
        {
            update();
        }

    protected:  //子以外呼ばなくていい
        void changeScene(const Key& dstSceneKey, bool cachePrevScene = false)
        {
            mSceneChanged = true;
            mApplication->changeScene(dstSceneKey, cachePrevScene);
        }

        void exitApplication()
        {
            mApplication->dispatchEnd();
        }

        const std::shared_ptr<Common>& getCommon() const
        {
            return mCommon;
        }

    private:  //メンバ変数
        std::shared_ptr<Common> mCommon;
        Application_t* mApplication;  //コンストラクタにてnullptrで初期化

        bool mSceneChanged;
    };

    template <typename Key, typename Common>
    class Application
    {
    private:  // using宣言部
        using Scene_t   = std::shared_ptr<IScene<Key, Common>>;
        using Factory_t = std::function<Scene_t()>;

    public:  //メソッド宣言部
        template <typename T>
        Application(std::string_view appName)
            : mCommon(std::make_shared<Common>())
            , mEndFlag(false)
        {
        }

        Application(std::string_view appName)
            : mCommon(std::make_shared<Common>())
            , mEndFlag(false)
        {
        }

        // Noncopyable, Nonmoveable
        Application(const Application&) = delete;
        Application& operator=(const Application&) = delete;
        Application(Application&&)                 = delete;
        Application& operator=(Application&&) = delete;

        ~Application()
        {
        }

        void init(const Key& firstSceneKey)
        {
            mFirstSceneKey = firstSceneKey;
            mEndFlag       = false;

            //開始シーンが設定されていない
            assert(mFirstSceneKey);

            mCurrent.first  = mFirstSceneKey.value();
            mCurrent.second = mScenesFactory[mFirstSceneKey.value()]();
        }

        void update()
        {
            //全体更新
            mCurrent.second->updateAll();
        }

        template <typename InheritedScene, typename = std::enable_if_t<std::is_base_of_v<IScene, InheritedScene>>>
        void addScene(const Key& key)
        {
            if (mScenesFactory.find(key) != mScenesFactory.end())
            {
#ifdef _DEBUG
                assert(!"this key already exist!");
#endif  //_DEBUG
        // release時は止めない
                return;
            }

            mScenesFactory.emplace(
                key,
                [&]()
                {
                    auto&& m = std::make_shared<InheritedScene>(this, mCommon);
                    m->initAll();

                    return m;
                });

            if (!mFirstSceneKey)  //まだ値がなかったら
                mFirstSceneKey = key;
        }

        void changeScene(const Key& dstSceneKey, bool cachePrevScene = false)
        {
            //そのシーンはない
            assert(mScenesFactory.find(dstSceneKey) != mScenesFactory.end());

            if (cachePrevScene)
                mCache = mCurrent;

            if (mCache && dstSceneKey == mCache.value().first)
            {
                mCurrent = mCache.value();
                mCache   = std::nullopt;
            }
            else
            {
                mCurrent.first  = dstSceneKey;
                mCurrent.second = mScenesFactory[dstSceneKey]();
            }
        }

        void dispatchEnd()
        {
            mEndFlag = true;
        }

        bool isEndAll()
        {
            return mEndFlag;
        }

    public:
        std::shared_ptr<Common> mCommon;

    private:
        std::unordered_map<Key, Factory_t> mScenesFactory;
        std::pair<Key, Scene_t> mCurrent;
        std::optional<std::pair<Key, Scene_t>> mCache;
        std::optional<Key> mFirstSceneKey;  // nulloptで初期化
        bool mEndFlag;
    };
}  // namespace mvecs

#endif