# ecs
[![Build Status](https://github.com/MathieuDonofrio/ecs/workflows/build/badge.svg)](https://github.com/MathieuDonofrio/ecs/actions)
[![Code Coverage](https://codecov.io/gh/MathieuDonofrio/ecs/branch/master/graph/badge.svg?token=1KD29OJ244)](https://codecov.io/gh/MathieuDonofrio/ecs)

Small, powerfull, header-only pure entity-component system that uses **compile-time archetypes** written in **modern c++**. 

# Table of Contents

* [Introduction](#introduction)
    * [Motivation](#implementation)
    * [Performance](#performance)
    * [Future](#future)
* [Code Example](#code-example)
* [Build Instructions](#build-instructions)

# Introduction

Entity–component–system (ECS) is an architectural pattern used to build high performance applications such as games or simulations. This pattern is usually a combination the following two philosophies:

* **Data-oriented design over object-oriented design** to take advantage of the cache in todays processors making code more efficient.
* **Composition over inheritance** for better modularity and higher flexibility.

This particular library takes a more modern, and increasingly popular approach know as **archetypes**. Archetypes as essentially a grouping of components that allow us to optimize our storage layout. 

For further details:

* ECS on Wikipedia: [ECS Wikipedia](https://en.wikipedia.org/wiki/Entity_component_system)

## Performance

If you haven't noticed, performance is a huge part of this project. I have fairly benchmarked against some of the most known and powerfull open-source ECS libraries and I can say with pretty good confidence that this library is very efficient. Not just in terms of speed but also in memory. 

This project includes some benchmarks that you can try for yourself and see if your satisfied with the results.

Generally, creating and deleting entities as well as setting the component data is the same speed or faster than any other library. Iterating over a single component is also the same speed or faster. However, iterating over multiple components is at least 2 times faster (very conservative) than any other library.

In terms of memory, for any ECS library that uses sparse sets, clever memory sharing optimizations make this library use much less memory. For an entity with any given archetype, the memory cost of storing it is: 2 * (size of entity identifier) + (size of all components in the archetype).

## Motivation

I have many motivations for writing and maintaining this library, notably for the fun, the learning experience and the future projects that will depend on this.

My goal for this project is to provide a simple but powerfull implementation. As of writing this, the project is less than 2000 lines of code with the documentation, all contained in just a handful of header files. The library must be simple, but not at the cost of performance.

Altho there is still work to be done, I still believe that I have accomplished my goals quite well. I am proud of my implementation thus far, so I have open-sourced it and hope that it will help others make high-performance applications or learn about ECS.

## Future

I have several things I would like to implement in the future, here are a few:

* Serialization
* Multi-threaded support
* Shared Components
* Static entites

I have built the libary with these already in mind, and I have quite a few good ideas but i'm still testing stuff out and making sure I can implement it in the best way possible.

# Code Example

# Build Instructions
