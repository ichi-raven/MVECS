#include "../../include/MVECS/Application.hpp"
#include "../../include/MVECS/World.hpp"
#include "../../include/MVECS/ISystem.hpp"

#include <iostream>

struct Common
{

};

struct A
{
	COMPONENT_DATA(A);

	A()
	{
		std::cout << "A\n";
	}

	A(const A& src)
	{
		std::cout << "copy A\n";
	}

	~A()
	{
		std::cout << "~A\n";
	}
};

struct B
{
	COMPONENT_DATA(B);

	B()
	{
		std::cout << "B\n";
	}

	~B()
	{
		std::cout << "~B\n";
	}
};

struct C
{
	COMPONENT_DATA(C);

	C()
	{
		std::cout << "C\n";
	}

	~C()
	{
		std::cout << "~C\n";
	}
};

class System : public mvecs::ISystem<int, Common>
{
	SYSTEM(System, int, Common);

	virtual void onInit()
	{
		for (int i = 0; i < 12; ++i)
		{
			mEntity = createEntity<A, B, C>();
		}

	}

	virtual void onUpdate()
	{

	}

	virtual void onEnd()
	{
		
	}

	std::optional<mvecs::Entity> mEntity;
};

int main()
{
	mvecs::Application<int, Common> app;

	auto& world = app.add(0);

	world.addSystem<System>();

	app.start(0);

	app.update();

	return 0;
}