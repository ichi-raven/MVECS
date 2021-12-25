
#include "MVECS.hpp"
#include <iostream>

struct A : public mvecs::IComponentData
{
    COMPONENT_DATA(A)
    int a;
    int b;
    int c;
    double test;
    int debug;
};

struct B : public mvecs::IComponentData
{
    COMPONENT_DATA(B)
    int d[5];
};

struct C : public mvecs::IComponentData
{
    COMPONENT_DATA(C)
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

    return 0;
}