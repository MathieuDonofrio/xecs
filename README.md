![Banner](https://user-images.githubusercontent.com/49959920/104272526-458c7980-546b-11eb-8dc4-684f27406a2e.png)

[![Build Status](https://github.com/MathieuDonofrio/xecs/workflows/build/badge.svg)](https://github.com/MathieuDonofrio/xecs/actions)
[![Code Coverage](https://codecov.io/gh/MathieuDonofrio/xecs/branch/master/graph/badge.svg?token=1KD29OJ244)](https://codecov.io/gh/MathieuDonofrio/xecs)

Xecs is a small but powerfull, header-only entity-component system that uses **compile-time archetypes** written in **modern c++**. 

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

This particular library takes a more modern, and increasingly popular approach know as **archetypes**. Archetypes are essentially a grouping of components that allow us to optimize our storage layout. 

For further details:

* ECS: [ECS Wikipedia](https://en.wikipedia.org/wiki/Entity_component_system)

## Performance

If you haven't noticed, performance is a huge part of this project. I have fairly benchmarked against some of the most known and powerfull open-source ECS libraries and I can say with pretty good confidence that this library is very efficient. Not just in terms of speed but also in memory. 

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

# Code Example

# Build Instructions
