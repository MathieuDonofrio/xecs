![Banner](https://user-images.githubusercontent.com/49959920/104272813-f2ff8d00-546b-11eb-8128-90c5ef943f14.png)

[![Build Status](https://github.com/MathieuDonofrio/xecs/workflows/build/badge.svg)](https://github.com/MathieuDonofrio/xecs/actions)
[![Code Coverage](https://codecov.io/gh/MathieuDonofrio/xecs/branch/master/graph/badge.svg?token=1KD29OJ244)](https://codecov.io/gh/MathieuDonofrio/xecs)

`XECS` is a small but powerfull, header-only entity-component system that uses **compile-time archetypes** written in **modern c++**. 

# Table of Contents

* [Introduction](#introduction)
  * [Motivation](#implementation)
  * [Performance](#performance)
  * [Future](#future)
* [Code Examples](#code-examples)
  * [Tutorial](#tutorial)
* [Build Instructions](#build-instructions)

# Introduction

Entity–component–system (ECS) is an architectural pattern used to build high performance applications such as games or simulations. This pattern is usually a combination the following two philosophies:

* **Data-oriented design over object-oriented design** to take advantage of the cache in todays processors making code more efficient.
* **Composition over inheritance** for better modularity and higher flexibility.

This particular library takes a more modern, and increasingly popular approach know as **archetypes**. Archetypes are essentially a grouping of components that allow us to optimize our storage layout. 

For further details:

* ECS: [ECS Wikipedia](https://en.wikipedia.org/wiki/Entity_component_system)

## Performance

Performance is a huge part of this project. I have fairly benchmarked against some of the most known and powerfull open-source ECS libraries and I can say with pretty good confidence that this library is very efficient. Not just in terms of speed but also in memory. 

This project includes some benchmarks that you can try for yourself and see if your satisfied with the results.

This library performs very well generally, but where it shines the most is when iterating over multiple components with complex relations. This makes this implementation a lot more scalable in terms speed. Iterating over multiple components is at least 2 times faster (very conservative) than any other ECS library.

More memory for more speed is a typical tradeoff in most ECS implementations, however clever compile-time optimizations make this implementation have very little memory overhead. For an entity with any given archetype, the memory cost of storing it is: 2 * (size of entity identifier) + (size of all components in the archetype).

## Motivation

I have many motivations for writing and maintaining this library, notably for the fun, the learning experience and the future projects that will depend on this.

My goal for this project is to provide a simple but powerfull implementation. As of writing this, the project is less than 2000 lines of code with the documentation, all contained in just a handful of header files. The library must be simple, but not at the cost of performance.

Altho there is still work to be done, I believe that I have accomplished my goals reasonably quite well. I am proud of my implementation thus far, so I have open-sourced it and hope that it will help others make high-performance applications or learn about ECS.

## Future

I have several things I would like to implement in the future, here are a few:

* Serialization
* Multi-threaded support
* Shared Components
* Static entites

I have built the libary with these already in mind, and I have quite a few good ideas but i'm still taking my time to make sure I can implement it in the best way possible in future versions.

# Code Examples

## Tutorial

### Include

```cpp
#include <xecs.hpp>
```

### Component creation

Components are simply just structs.

```cpp
struct Position
{
  float x;
  float y;
};

struct Velocity
{
  float x;
  float y;
};

```

### Compile-time configuration

You must first choose a unsigned int type to be your entity type.

```cpp
using entity = uint32_t;
```

You must declare all your archetypes, you can use the builder utility.

```cpp
using archetypes = xecs::archetype_list_builder::
  add<xecs::archetype<Position>>::
  add<xecs::archetype<Position, Velocity>>::
  build;
```

### Registry creation

A registry is where your entities and components are stored and managed.

```cpp
xecs::registry<entity, archetypes> registry;
```

### Entity creation and initialization

Call the create method with all the components that the entity will have (The archetype of the component).

Excample for archetype [Position]:
```cpp
registry.create(Position { 10, 20 });
```

Example for archetype [Position, Velocity]:
```cpp
registry.create(Position { 5, 99 }, Velocity { 3, 5 });
```

### Entity destruction

Call the destroy method and pass the entity.

```cpp
entity entity_to_destroy = registry.create(Position { });

registry.destroy(entity_to_destroy);
```

### Entity component unpacking

You can call the unpack method to obtain a reference of the component your trying to access.

```cpp
entity entity_to_unpack = registry.create(Position { }, Velocity { });

registry.unpack<Position>(entity_to_unpack) = Position { 1, 1 };
registry.unpack<Velocity>(entity_to_unpack) = Velocity { 2, 2 };
```

### Iterating

Iterate over all entity identifiers

```cpp
registry.for_each([](const auto entity)
{
  /* ... */
});
```

Iterate over all entities with a specified component

```cpp
registry.for_each<Position>([](const auto entity, const auto& position)
{
  /* ... */
});
```

Iterate over all entities with multiple specified component

```cpp
registry.for_each<Position, Velocity>([](const auto entity, const auto& position, const auto& velocity)
{
  /* ... */
});
```

Using a view

```cpp
auto view = registry.view<Position, Velocit>();

view.for_each([](const auto entity, const auto& position, const auto& velocity)
{
  /* ... */
});
```

# Build Instructions

## Requirements

To be able to use `XECS` you must have a standard compliant compiler that supports at least C++17.

The following platforms and compilers are continously tested to work:

* Windows
  * msvc
* Windows-2016
  * default
* Ubuntu
  * gcc
  * clang
* MacOS
  * default

## Library

`XECS` is a header-only library. All you need to do is include the headers.

```cpp
#include <xecs.hpp>
```

## Tests

`googletest` is used as our testing framework. CMake will sets all this up for your.

All you need to do is to set the cmake option `XECS_BUILD_TESTING" to `ON`.

Steps on windows using msvc:
* `cd build`
* `cmake -DXECS_BUILD_TESTING=ON ..`
* Open the generated solution
* Build and run the `test` project

Steps on linux:
* `$ cd build`
* `$ cmake -DXECS_BUILD_TESTING=ON ..`
* `$ make`
* `$ ./test/tests`

## Benchmark

No dependancies are used for benchmarking.

All you need to do is to set the cmake option `XECS_BUILD_BENCHMAKING" to `ON`.

Using visual studio:
* `cd build`
* `cmake -DXECS_BUILD_TESTING=ON ..`
* Open the generated solution
* Build and run the `test` project

Using make:
* `$ cd build`
* `$ cmake -XECS_BUILD_BENCHMAKING=ON ..`
* `$ make`
* `$ ./bench/benchmark`


