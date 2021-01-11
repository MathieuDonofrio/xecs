#include "benchmark.h"

#include <registry.hpp>
#include <vector>

using namespace ecs;

struct Position
{
  double x;
  double y;
};

struct Velocity
{
  double x;
  double y;
};

struct Color
{
  uint32_t r;
  uint32_t g;
  uint32_t b;
  uint32_t a;
};

template<size_t ID>
struct Component
{
  uint64_t data1;
  uint64_t data2;
};

void Create_NoComponents()
{
  using entity_type = unsigned int;
  using registered_archetypes = archetype_list_builder::add<
    archetype<>>::build;

  registry<entity_type, registered_archetypes> registry;

  const size_t iterations = 2500000;

  BEGIN_BENCHMARK(Create_NoComponents);

  for (size_t i = 0; i < iterations; i++)
  {
    benchmark::do_not_optimize(registry.create());
    benchmark::do_not_optimize(registry.create());
    benchmark::do_not_optimize(registry.create());
    benchmark::do_not_optimize(registry.create());
  }

  END_BENCHMARK(iterations, 4);

  benchmark::do_not_optimize(registry.size());
}

void Create_OneComponent()
{
  using entity_type = unsigned int;
  using registered_archetypes = archetype_list_builder::add<
    archetype<Position>>::build;

  registry<entity_type, registered_archetypes> registry;

  const size_t iterations = 10000000;

  BEGIN_BENCHMARK(Create_OneComponent);

  for (size_t i = 0; i < iterations; i++)
  {
    benchmark::do_not_optimize(registry.create(Position {}));
  }

  END_BENCHMARK(iterations, 1);

  benchmark::do_not_optimize(registry.size());
}

void Create_TwoComponents()
{
  using entity_type = unsigned int;
  using registered_archetypes = archetype_list_builder::add<
    archetype<Position, Velocity>>::build;

  registry<entity_type, registered_archetypes> registry;

  const size_t iterations = 10000000;

  BEGIN_BENCHMARK(Create_TwoComponents);

  for (size_t i = 0; i < iterations; i++)
  {
    benchmark::do_not_optimize(registry.create(Position {}, Velocity {}));
  }

  END_BENCHMARK(iterations, 1);

  benchmark::do_not_optimize(registry.size());
}

void Create_ThreeComponents()
{
  using entity_type = unsigned int;
  using registered_archetypes = archetype_list_builder::add<
    archetype<Position, Velocity, Color>>::build;

  registry<entity_type, registered_archetypes> registry;

  const size_t iterations = 10000000;

  BEGIN_BENCHMARK(Create_ThreeComponents);

  for (size_t i = 0; i < iterations; i++)
  {
    benchmark::do_not_optimize(registry.create(Position {}, Velocity {}, Color {}));
  }

  END_BENCHMARK(iterations, 1);

  benchmark::do_not_optimize(registry.size());
}

void Destroy_NoComponents()
{
  using entity_type = unsigned int;
  using registered_archetypes = archetype_list_builder::add<
    archetype<>>::build;

  registry<entity_type, registered_archetypes> registry;

  std::vector<entity_type> entities {};

  const size_t iterations = 2500000;

  for (size_t i = 0; i < iterations; i++) entities.push_back(registry.create());

  BEGIN_BENCHMARK(Destroy_NoComponents);

  for (size_t i = 0; i < entities.size(); i += 4)
  {
    registry.destroy(entities[i]);
    registry.destroy(entities[i + 1]);
    registry.destroy(entities[i + 2]);
    registry.destroy(entities[i + 3]);
  }

  END_BENCHMARK(iterations, 4);

  benchmark::do_not_optimize(registry.size());
}

void Destroy_OneComponent()
{
  using entity_type = unsigned int;
  using registered_archetypes = archetype_list_builder::add<
    archetype<Position>>::build;

  registry<entity_type, registered_archetypes> registry;

  std::vector<entity_type> entities {};

  const size_t iterations = 10000000;

  for (size_t i = 0; i < iterations; i++) entities.push_back(registry.create(Position {}));

  BEGIN_BENCHMARK(Destroy_OneComponent);

  for (size_t i = 0; i < entities.size(); i++)
  {
    registry.destroy(entities[i]);
  }

  END_BENCHMARK(iterations, 1);

  benchmark::do_not_optimize(registry.size());
}

void Destroy_TwoComponents()
{
  using entity_type = unsigned int;
  using registered_archetypes = archetype_list_builder::add<
    archetype<Position, Velocity>>::build;

  registry<entity_type, registered_archetypes> registry;

  std::vector<entity_type> entities {};

  const size_t iterations = 10000000;

  for (size_t i = 0; i < iterations; i++) entities.push_back(registry.create(Position {}, Velocity {}));

  BEGIN_BENCHMARK(Destroy_TwoComponents);

  for (size_t i = 0; i < entities.size(); i++)
  {
    registry.destroy(entities[i]);
  }

  END_BENCHMARK(iterations, 1);

  benchmark::do_not_optimize(registry.size());
}

void Destroy_ThreeComponents()
{
  using entity_type = unsigned int;
  using registered_archetypes = archetype_list_builder::add<
    archetype<Position, Velocity, Color>>::build;

  registry<entity_type, registered_archetypes> registry;

  std::vector<entity_type> entities {};

  const size_t iterations = 10000000;

  for (size_t i = 0; i < iterations; i++) entities.push_back(registry.create(Position {}, Velocity {}, Color {}));

  BEGIN_BENCHMARK(Destroy_ThreeComponents);

  for (size_t i = 0; i < entities.size(); i++)
  {
    registry.destroy(entities[i]);
  }

  END_BENCHMARK(iterations, 1);

  benchmark::do_not_optimize(registry.size());
}

void Destroy_TwoArchetypes()
{
  using entity_type = unsigned int;
  using registered_archetypes = archetype_list_builder::
    add<archetype<Position>>::
      add<archetype<Velocity>>::build;

  registry<entity_type, registered_archetypes> registry;

  std::vector<entity_type> entities {};

  const size_t iterations = 10000000;

  for (size_t i = 0; i < iterations; i++)
  {
    if (i % 2 == 0) entities.push_back(registry.create(Position {}));
    else
      entities.push_back(registry.create(Velocity {}));
  }

  BEGIN_BENCHMARK(Destroy_TwoArchetypes);

  for (size_t i = 0; i < entities.size(); i++)
  {
    registry.destroy(entities[i]);
  }

  END_BENCHMARK(iterations, 1);

  benchmark::do_not_optimize(registry.size());
}

void Destroy_ThreeArchetypes()
{
  using entity_type = unsigned int;
  using registered_archetypes = archetype_list_builder::
    add<archetype<Position>>::
      add<archetype<Velocity>>::
        add<archetype<Color>>::build;

  registry<entity_type, registered_archetypes> registry;

  std::vector<entity_type> entities {};

  const size_t iterations = 10000000;

  for (size_t i = 0; i < iterations; i++)
  {
    switch (i % 3)
    {
    case 0: entities.push_back(registry.create(Position {})); break;
    case 1: entities.push_back(registry.create(Velocity {})); break;
    case 2: entities.push_back(registry.create(Color {})); break;
    }
  }

  BEGIN_BENCHMARK(Destroy_ThreeArchetypes);

  for (size_t i = 0; i < entities.size(); i++)
  {
    registry.destroy(entities[i]);
  }

  END_BENCHMARK(iterations, 1);

  benchmark::do_not_optimize(registry.size());
}

void Destroy_TenArchetypesTwoComponents()
{
  using entity_type = unsigned int;
  using registered_archetypes = archetype_list_builder::
    add<archetype<Position, Component<0>>>::
      add<archetype<Position, Component<1>>>::
        add<archetype<Position, Component<2>>>::
          add<archetype<Position, Component<3>>>::
            add<archetype<Position, Component<4>>>::
              add<archetype<Position, Component<5>>>::
                add<archetype<Position, Component<6>>>::
                  add<archetype<Position, Component<7>>>::
                    add<archetype<Position, Component<8>>>::
                      add<archetype<Position, Component<9>>>::build;

  registry<entity_type, registered_archetypes> registry;

  std::vector<entity_type> entities {};

  const size_t iterations = 10000000;

  for (size_t i = 0; i < iterations; i++)
  {
    switch (i % 10)
    {
    case 0: entities.push_back(registry.create(Position {}, Component<0> {})); break;
    case 1: entities.push_back(registry.create(Position {}, Component<1> {})); break;
    case 2: entities.push_back(registry.create(Position {}, Component<2> {})); break;
    case 3: entities.push_back(registry.create(Position {}, Component<3> {})); break;
    case 4: entities.push_back(registry.create(Position {}, Component<4> {})); break;
    case 5: entities.push_back(registry.create(Position {}, Component<5> {})); break;
    case 6: entities.push_back(registry.create(Position {}, Component<6> {})); break;
    case 7: entities.push_back(registry.create(Position {}, Component<7> {})); break;
    case 8: entities.push_back(registry.create(Position {}, Component<8> {})); break;
    case 9: entities.push_back(registry.create(Position {}, Component<9> {})); break;
    }
  }

  BEGIN_BENCHMARK(Destroy_TenArchetypesTwoComponents);

  for (size_t i = 0; i < entities.size(); i += 10)
  {
    registry.destroy(entities[i]);
    registry.destroy(entities[i + 1]);
    registry.destroy(entities[i + 2]);
    registry.destroy(entities[i + 3]);
    registry.destroy(entities[i + 4]);
    registry.destroy(entities[i + 5]);
    registry.destroy(entities[i + 6]);
    registry.destroy(entities[i + 7]);
    registry.destroy(entities[i + 8]);
    registry.destroy(entities[i + 9]);
  }

  END_BENCHMARK(iterations / 10, 10);

  benchmark::do_not_optimize(registry.size());
}

void Destroy_TenArchetypesTwoComponents_KnownTypes()
{
  using entity_type = unsigned int;
  using registered_archetypes = archetype_list_builder::
    add<archetype<Position, Component<0>>>::
      add<archetype<Position, Component<1>>>::
        add<archetype<Position, Component<2>>>::
          add<archetype<Position, Component<3>>>::
            add<archetype<Position, Component<4>>>::
              add<archetype<Position, Component<5>>>::
                add<archetype<Position, Component<6>>>::
                  add<archetype<Position, Component<7>>>::
                    add<archetype<Position, Component<8>>>::
                      add<archetype<Position, Component<9>>>::build;

  registry<entity_type, registered_archetypes> registry;

  std::vector<entity_type> entities {};

  const size_t iterations = 10000000;

  for (size_t i = 0; i < iterations; i++)
  {
    switch (i % 10)
    {
    case 0: entities.push_back(registry.create(Position {}, Component<0> {})); break;
    case 1: entities.push_back(registry.create(Position {}, Component<1> {})); break;
    case 2: entities.push_back(registry.create(Position {}, Component<2> {})); break;
    case 3: entities.push_back(registry.create(Position {}, Component<3> {})); break;
    case 4: entities.push_back(registry.create(Position {}, Component<4> {})); break;
    case 5: entities.push_back(registry.create(Position {}, Component<5> {})); break;
    case 6: entities.push_back(registry.create(Position {}, Component<6> {})); break;
    case 7: entities.push_back(registry.create(Position {}, Component<7> {})); break;
    case 8: entities.push_back(registry.create(Position {}, Component<8> {})); break;
    case 9: entities.push_back(registry.create(Position {}, Component<9> {})); break;
    }
  }

  BEGIN_BENCHMARK(Destroy_TenArchetypesTwoComponents_KnownTypes);

  for (size_t i = 0; i < entities.size(); i += 10)
  {
    registry.destroy<Position, Component<0>>(entities[i]);
    registry.destroy<Position, Component<1>>(entities[i + 1]);
    registry.destroy<Position, Component<2>>(entities[i + 2]);
    registry.destroy<Position, Component<3>>(entities[i + 3]);
    registry.destroy<Position, Component<4>>(entities[i + 4]);
    registry.destroy<Position, Component<5>>(entities[i + 5]);
    registry.destroy<Position, Component<6>>(entities[i + 6]);
    registry.destroy<Position, Component<7>>(entities[i + 7]);
    registry.destroy<Position, Component<8>>(entities[i + 8]);
    registry.destroy<Position, Component<9>>(entities[i + 9]);
  }

  END_BENCHMARK(iterations / 10, 10);

  benchmark::do_not_optimize(registry.size());
}

void Iterate_STD_Vector_AsComparaison()
{
  using entity_type = unsigned int;

  std::vector<entity_type> v;

  const size_t iterations = 10000000;

  for (size_t i = 0; i < iterations; i++)
  {
    v.push_back(i);
  }

  BEGIN_BENCHMARK(Iterate_STD_Vector_AsComparaison);

  for (auto it = v.begin(); it != v.end(); ++it)
  {
    auto entity = *it;
    benchmark::do_not_optimize(entity);
  }

  END_BENCHMARK(iterations, 1);

  benchmark::do_not_optimize(v.size());
}

void Iterate_NoComponents()
{
  using entity_type = unsigned int;
  using registered_archetypes = archetype_list_builder::add<
    archetype<>>::build;

  registry<entity_type, registered_archetypes> registry;

  const size_t iterations = 10000000;

  for (size_t i = 0; i < iterations; i++)
  {
    registry.create();
  }

  BEGIN_BENCHMARK(Iterate_NoComponents);

  registry.for_each([](entity_type entity)
    { benchmark::do_not_optimize(entity); });

  END_BENCHMARK(iterations, 1);

  benchmark::do_not_optimize(registry.size());
}

void Iterate_OneComponent()
{
  using entity_type = unsigned int;
  using registered_archetypes = archetype_list_builder::add<
    archetype<Position>>::build;

  registry<entity_type, registered_archetypes> registry;

  const size_t iterations = 10000000;

  for (size_t i = 0; i < iterations; i++)
  {
    registry.create(Position {});
  }

  BEGIN_BENCHMARK(Iterate_OneComponent);

  registry.for_each<Position>([](auto entity, auto& position)
    {
      benchmark::do_not_optimize(entity);
      benchmark::do_not_optimize(position);
    });

  END_BENCHMARK(iterations, 1);

  benchmark::do_not_optimize(registry.size());
}

void Iterate_TwoComponents()
{
  using entity_type = unsigned int;
  using registered_archetypes = archetype_list_builder::add<
    archetype<Position, Velocity>>::build;

  registry<entity_type, registered_archetypes> registry;

  const size_t iterations = 10000000;

  for (size_t i = 0; i < iterations; i++)
  {
    registry.create(Position {}, Velocity {});
  }

  BEGIN_BENCHMARK(Iterate_TwoComponents);

  registry.for_each<Position, Velocity>([](auto entity, auto& position, auto& velocity)
    {
      benchmark::do_not_optimize(entity);
      benchmark::do_not_optimize(position);
      benchmark::do_not_optimize(velocity);
    });

  END_BENCHMARK(iterations, 1);

  benchmark::do_not_optimize(registry.size());
}

void Iterate_ThreeComponents()
{
  using entity_type = unsigned int;
  using registered_archetypes = archetype_list_builder::add<
    archetype<Position, Velocity, Color>>::build;

  registry<entity_type, registered_archetypes> registry;

  const size_t iterations = 10000000;

  for (size_t i = 0; i < iterations; i++)
  {
    registry.create(Position {}, Velocity {}, Color {});
  }

  BEGIN_BENCHMARK(Iterate_ThreeComponents);

  registry.for_each<Position, Velocity, Color>([](auto entity, auto& position, auto& velocity, auto& color)
    {
      benchmark::do_not_optimize(entity);
      benchmark::do_not_optimize(position);
      benchmark::do_not_optimize(velocity);
      benchmark::do_not_optimize(color);
    });

  END_BENCHMARK(iterations, 1);

  benchmark::do_not_optimize(registry.size());
}

void Iterate_TenComponents()
{
  using entity_type = unsigned int;
  using registered_archetypes = archetype_list_builder::add<
    archetype<
      Component<0>,
      Component<1>,
      Component<2>,
      Component<3>,
      Component<4>,
      Component<5>,
      Component<6>,
      Component<7>,
      Component<8>,
      Component<9>>>::build;

  registry<entity_type, registered_archetypes> registry;

  const size_t iterations = 10000000;

  for (size_t i = 0; i < iterations; i++)
  {
    registry.create(
      Component<0> {},
      Component<1> {},
      Component<2> {},
      Component<3> {},
      Component<4> {},
      Component<5> {},
      Component<6> {},
      Component<7> {},
      Component<8> {},
      Component<9> {});
  }

  BEGIN_BENCHMARK(Iterate_TenComponents);

  registry.for_each<
    Component<0>,
    Component<1>,
    Component<2>,
    Component<3>,
    Component<4>,
    Component<5>,
    Component<6>,
    Component<7>,
    Component<8>,
    Component<9>>(
    [](auto entity, auto& c0, auto& c1, auto& c2, auto& c3, auto& c4, auto& c5, auto& c6, auto& c7, auto& c8, auto& c9)
    {
      benchmark::do_not_optimize(entity);
      benchmark::do_not_optimize(c0);
      benchmark::do_not_optimize(c1);
      benchmark::do_not_optimize(c2);
      benchmark::do_not_optimize(c3);
      benchmark::do_not_optimize(c4);
      benchmark::do_not_optimize(c5);
      benchmark::do_not_optimize(c6);
      benchmark::do_not_optimize(c7);
      benchmark::do_not_optimize(c8);
      benchmark::do_not_optimize(c9);
    });

  END_BENCHMARK(iterations, 1);

  benchmark::do_not_optimize(registry.size());
}

void Iterate_TenArchetypesNoComponents()
{
  using entity_type = unsigned int;
  using registered_archetypes = archetype_list_builder::
    add<archetype<Component<0>>>::
      add<archetype<Component<1>>>::
        add<archetype<Component<2>>>::
          add<archetype<Component<3>>>::
            add<archetype<Component<4>>>::
              add<archetype<Component<5>>>::
                add<archetype<Component<6>>>::
                  add<archetype<Component<7>>>::
                    add<archetype<Component<8>>>::
                      add<archetype<Component<9>>>::build;

  registry<entity_type, registered_archetypes> registry;

  const size_t iterations = 10000000;

  for (size_t i = 0; i < iterations; i++)
  {
    switch (i % 10)
    {
    case 0: registry.create(Component<0> {}); break;
    case 1: registry.create(Component<1> {}); break;
    case 2: registry.create(Component<2> {}); break;
    case 3: registry.create(Component<3> {}); break;
    case 4: registry.create(Component<4> {}); break;
    case 5: registry.create(Component<5> {}); break;
    case 6: registry.create(Component<6> {}); break;
    case 7: registry.create(Component<7> {}); break;
    case 8: registry.create(Component<8> {}); break;
    case 9: registry.create(Component<9> {}); break;
    }
  }

  BEGIN_BENCHMARK(Iterate_TenArchetypesNoComponents);

  registry.for_each([](auto entity)
    { benchmark::do_not_optimize(entity); });

  END_BENCHMARK(iterations, 1);

  benchmark::do_not_optimize(registry.size());
}

void Iterate_STDVectorToCompare_WithSomeWork()
{
  std::vector<unsigned int> entities;
  std::vector<Position> positions;
  std::vector<Velocity> velocities;

  const size_t iterations = 10000000;

  for (size_t i = 0; i < iterations; i++)
  {
    entities.push_back(i);
    double d = static_cast<double>(i);
    positions.push_back(Position { d, d });
    velocities.push_back(Velocity { d, d });
  }

  BEGIN_BENCHMARK(Iterate_STDVectorToCompare_WithSomeWork);

  for (size_t i = 0; i < iterations; i++)
  {
    auto& position = positions[i];
    auto& velocity = velocities[i];

    position.x *= velocity.x * velocity.x;
    position.y *= velocity.y * velocity.y;
    velocity.x *= 0.98956;
    velocity.y *= 0.98789;

    benchmark::do_not_optimize(entities[i]);
  }

  END_BENCHMARK(iterations, 1);

  double sum = 0;

  for (size_t i = 0; i < iterations; i++)
  {
    auto& position = positions[i];
    auto& velocity = velocities[i];

    sum += position.x + position.y + velocity.x + velocity.y;
  }

  benchmark::do_not_optimize(sum);
}

void Iterate_WithSomeWork()
{
  using entity_type = unsigned int;
  using registered_archetypes = archetype_list_builder::
    add<archetype<Position, Velocity>>::
      add<archetype<Position, Velocity, Color>>::
        build;

  registry<entity_type, registered_archetypes> registry;

  const size_t iterations = 10000000;

  for (size_t i = 0; i < iterations; i++)
  {
    double d = static_cast<double>(i);
    if (i % 2)
      registry.create(Position { d, d }, Velocity { d, d });
    else
      registry.create(Position { d, d }, Velocity { d, d }, Color {});
  }

  BEGIN_BENCHMARK(Iterate_WithSomeWork);

  registry.for_each<Position, Velocity>([](auto entity, auto& position, auto& velocity)
    {
      position.x *= velocity.x * velocity.x;
      position.y *= velocity.y * velocity.y;
      velocity.x *= 0.98956;
      velocity.y *= 0.98789;
      benchmark::do_not_optimize(entity);
    });

  END_BENCHMARK(iterations, 1);

  double sum = 0;

  registry.for_each<Position, Velocity>([&sum](auto entity, auto& position, auto& velocity)
    { sum += position.x + position.y + velocity.x + velocity.y; });

  benchmark::do_not_optimize(sum);
  benchmark::do_not_optimize(registry.size());
}

int main()
{
  Create_NoComponents();
  Create_OneComponent();
  Create_TwoComponents();
  Create_ThreeComponents();

  Destroy_NoComponents();
  Destroy_OneComponent();
  Destroy_TwoComponents();
  Destroy_ThreeComponents();
  Destroy_TwoArchetypes();
  Destroy_ThreeArchetypes();
  Destroy_TenArchetypesTwoComponents();
  Destroy_TenArchetypesTwoComponents_KnownTypes();

  Iterate_STD_Vector_AsComparaison();
  Iterate_NoComponents();
  Iterate_OneComponent();
  Iterate_TwoComponents();
  Iterate_ThreeComponents();
  Iterate_TenComponents();
  Iterate_TenArchetypesNoComponents();
  Iterate_STDVectorToCompare_WithSomeWork();
  Iterate_WithSomeWork();

  return 0;
}