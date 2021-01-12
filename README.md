# ecs
[![Build Status](https://github.com/MathieuDonofrio/ecs/workflows/build/badge.svg)](https://github.com/MathieuDonofrio/ecs/actions)
[![Code Coverage](https://codecov.io/gh/MathieuDonofrio/ecs/branch/master/graph/badge.svg?token=1KD29OJ244)](https://codecov.io/gh/MathieuDonofrio/ecs)

Small, powerfull, header-only pure entity-component system that uses **compile-time archetypes** written in **modern c++**. 

# Table of Contents

* [Introduction](#introduction)
    * [Motivation](#implementation)
    * [Implementation](#implementation)
* [Code Example](#code-example)
* [Build Instructions](#build-instructions)

# Introduction

I normally describe entity-component system pattern as similar to composition (components) but uses a data-oriented design.

Commonly used object-oriented design is not always a good option for high-performance applications because of indirections in memory. Alternatively, data-oriented design is contiguous in memory, this takes advantage of the cache in todays processors, resulting more efficient code.

In the game industry, composition is commonly used over inheritance in gameobjects/entities to allow for better modularity, making implementing various behaviours easier.

This is where the entity-component system (ECS) comes in. ECS is an architectural pattern that uses data-oriented design and composition-like usability. Entities are identifiers (usually unsigned int), components are data and systems are behaviours. Divide and conquer!

## Motivation

I have many motivations for writing and maintaining this library, notably for the fun, the learning experience and the future projects that will depend on this.

My goal for this project is to provide a simple but powerfull implementation. As of writing this, the project is less than 2000 lines of code with the documentation, all contained in just a handful of header files. The library must be simple, but not at the cost of performance.

Altho there is still work to be done, I still believe that I have accomplished my goals quite well. I am proud of my implementation thus far, so I have open-sourced it and hope that it will help others make high-performance applications or learn about ECS.

## Implementation

There is many ways to implement ECS, this library uses a more modern approch that involves archetypes.

ECS without archetypes must use several techniques to properly iterate contiguously over multiple components. When even possible, this results in some overhead with branching conditions, and unessesary complexity. However, this isn't a big deal, and is still faster than most object-oriented approches.

ECS with archetypes allow for us to store are components in a truly contiguous way, this means that we can iterate over more components very efficiently. However, there are still some trade-offs in terms of managing these archetypes and not all implementations of ecs find it nessesarry to do so.

In our implementation, we manage our archetypes at compile-time. By doing this, we get the best of both worlds.

## Performance

If you haven't noticed, performance is a huge part of this project. I have fairly benchmarked against some of the most known and powerfull open-source ECS libraries and I can say with pretty good confidence that this library is very efficient. Not just in terms of speed but also in memory. 

This project includes some benchmarks that you can try for yourself and see if your satisfied with the results.

Generally, creating and deleting entities as well as setting the component data is the same speed or faster than any other library. Iterating over a single component is also the same speed or faster. However, iterating over multiple components is at least 2 times faster (very conservative) than any other library.

In terms of memory, for any ECS library that uses sparse sets, clever memory sharing optimizations make this library use much less memory. For an entity with any given archetype, the memory cost of storing it is: 2 * (size of entity identifier) + (size of all components in the archetype).

## Future

In the future I would like to implement serialization and support multi-threading. I have built the library with both of these in mind, and I have quite a few good ideas but i'm still testing stuff out and making sure I can do it in the best way possible.

# Code Example

# Build Instructions
