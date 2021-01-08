#include "src/archetype.hpp"

#include <iostream>

struct RequireBase
{
};

template<typename... Components>
struct Require : RequireBase
{
  
};

struct Position
{
  float x;
  float y;
};

struct Velocity : Require<Position>
{
  float x;
  float y;
};

int main()
{
  using namespace ecs;

  using registry = registry_builder::
    add<archetype<int>>::
      add<archetype<float>>::
        add<archetype<int, bool>>::
          build;

  registry r;

  (void)r;

  std::cout << std::is_base_of_v<Velocity, Require<Position>> << std::endl;

  return 0;
}