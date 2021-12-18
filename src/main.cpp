#include <Archetype.hpp>
#include <Chunk.hpp>
#include <IComponentData.hpp>
#include <ISystem.hpp>
#include <TypeInfo.hpp>
#include <World.hpp>
#include <iostream>

struct A : public mvecs::IComponentData
{
    USE_TYPEINFO(A)
    int a;
    double b;
    float c;
};

struct B : public mvecs::IComponentData
{
    USE_TYPEINFO(B)
    int d[5];
};

struct C : public mvecs::IComponentData
{
    USE_TYPEINFO(C)
    double e;
    char f;
};

class TestSystem : public mvecs::ISystem
{
public:
    TestSystem(mvecs::World* const pWorld)
        : ISystem(pWorld)
    {
    }

    virtual void onStart()
    {
        std::cerr << "on start\n";
        int i = 0;
        forEach<A>([&](A& val)
                   {
                       if (++i % 1000 == 0)
                           std::cerr << i << "\n";
                       val.a += 2;
                   });
    }

    virtual void onUpdate()
    {
        std::cerr << "on update\n";
    }

    virtual void onEnd()
    {
        std::cerr << "on end\n";
    }
};

/**
 * @brief main関数
 * 
 * @return int 
 */
int main()
{
    std::cerr << "main function\n";

    // Worldは必ずshared_ptrで構築する
    std::shared_ptr<mvecs::World> world = std::make_shared<mvecs::World>();

    auto e1 = world->createEntity<A, B, C>();
    auto e2 = world->createEntity<A, B>();
    auto e3 = world->createEntity<C, B, A>();

    for (std::size_t i = 0; i < 100000; ++i)
    {
        if (i % 1000 == 0)
            std::cerr << i << "\n";
        world->createEntity<A, B, C>();
    }

    A a;
    a.a = 10;
    a.b = 11;
    a.c = 12;
    world->setComponentData<A>(e2, a);

    world->destroyEntity(e1);

    world->addSystem<TestSystem>();

    world->update();

    A got = world->getComponentData<A>(e2);

    std::cerr << got.a << "\n";

    return 0;
}