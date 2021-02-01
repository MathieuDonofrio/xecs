# XECS TODO

Compleated tasks are marked with and 'x', they will be removed at the next release.

## Patch

High priority!

These tasks may be minor bug fixes or optimizations. They do not change the interface.

- [x] Runtime library bug with CMake and MSVC 
- [x] Test build error in WSL Ubuntu

## Minor

These tasks are small or large updates that are non breaking for non deprecated functionalities.

- [ ] Doxygen
- [ ] Replace malloc/new with better allocation
- [ ] Ability to make an entity swap archetypes
- [x] Compile-time archetype sorting for views
- [ ] Archetype list generation from component list
- [ ] Shared components
- [ ] Static entities
- [ ] Multi-threaded support

## Major

These tasks are large updates with potentially breaking changes.

- [ ] Replacement of archetype list for component list

## Experimental

These tasks are experiments to be done in the experimental branch.

- [ ] Entity id's with extra storage bits (versions?)
- [ ] Native Serialization
