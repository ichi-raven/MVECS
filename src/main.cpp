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
    int b;
    int c;
    double test;
    int debug;
};

struct B : public mvecs::IComponentData
{
    USE_TYPEINFO(B)
    int d[5];
};

struct C : public mvecs::IComponentData
{
    USE_TYPEINFO(C)
    char e[10];
    char f;
    float a;
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
        forEachParallel<A>([&](A& val)
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

private:
};

/**
 * @brief main関数
 *
 * @return int
 */
int main()
{
    std::cerr << "main function\n";

    std::unique_ptr<mvecs::World> world = std::make_unique<mvecs::World>();

    auto e1 = world->createEntity<A, B, C>();
    auto e2 = world->createEntity<A, C>();
    auto e3 = world->createEntity<C, B, A>();

    for (std::size_t i = 0; i < 10000; ++i)
    {
        world->createEntity<A, B, C>();
    }

    A a1, a2;
    a1.a = 1;
    a1.b = 2;
    a1.c = 3;
    a1.test = 0;
    a1.debug = -1;

    a2.a = 10;
    a2.b = 11;
    a2.c = 12;
    a2.test = 0;
    a2.debug = -1;

    world->setComponentData<A>(e1, a1);
    world->setComponentData<A>(e3, a2);

    A got1 = world->getComponentData<A>(e1);
    world->destroyEntity(e1);
    world->createEntity<A, B, C>();

    world->addSystem<TestSystem>();
    world->createEntity<A, B, C>();

    world->update();

    std::cerr << "end all\n";

    return 0;
}