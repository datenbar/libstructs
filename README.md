# libstructs

A collection of typed data structures for C, generated from templates rather
than written by hand for every type.

## Overview

C has no built-in equivalent to C++ templates. libstructs works around that
limitation with a small code generation step: you write the logic for a
structure once, and a script produces real, distinct C source files for
every type you need (`vector_int`, `vector_float`, `vector_MyStruct`, and so
on). The generated files are ordinary, readable C.

The library has no required dependency on the standard C library inside its
core structures. Memory is requested through a small allocator interface
that you provide, which means the same structure code can run in a normal
hosted program, in an embedded environment, or inside a freestanding
context such as an operating system kernel, without being rewritten.

This project began as a way to build a solid, from-scratch understanding of
data structures and memory management in C, and is intended for general use
in any C project that wants typed containers without pulling in a large
dependency.

## Design principles

- **Typed, not type-erased.** Every generated structure is a real, distinct
  C type. There is no `void*` casting and no runtime type checks.
- **No implicit allocation.** Every structure takes an allocator at creation
  time. The default allocator is backed by the standard library, but any
  allocator matching the same interface can be substituted, including one
  backed by a custom memory manager.
- **Explicit ownership.** Every creation function has a matching cleanup
  function. Nothing is freed automatically.
- **Configurable, not fixed.** Growth behavior, such as when a hash map
  resizes or how much a vector grows by, is read from a small configuration
  header rather than hardcoded, so it can be tuned for the environment the
  library is built for.
- **No exceptions.** C does not have them. Failures are reported through
  return values or through an assertion macro that can be swapped out for a
  panic handler in constrained environments.

## How the code generation works

Templates live in `templates/` and use a placeholder, `@T@`, wherever a type
would normally go. A short Python script, `gen.py`, reads each template,
replaces the placeholder with a concrete type name, and writes the result
into `generated/` as normal `.c` and `.h` files.

```bash
python gen.py
```

Adding support for a new type means adding it to `types.txt` and running the
script again. Nothing about the generated code depends on the preprocessor.

## Building

The project uses CMake, which can detect the amount of memory available on
the build machine and adjust default settings such as initial capacity and
hash map load factor accordingly. This is useful for tuning the library
toward the constraints of a specific target, whether that is a typical
desktop, a resource-limited device, or a kernel with a small fixed heap.

For Arch Linux :

```bash
sudo pacman -S --needed base-devel cmake git python valgrind
cmake -B build
cmake --build build --target generate
cmake --build build
ctest --test-dir build --output-on-failure
```

When cross compiling for a target that is not the machine running the
build, the memory budget can be set manually instead of relying on
detection:

```bash
cmake -B build -DLIBSTRUCTS_TARGET_MEMORY_KB=64
```

See `CMakeLists.txt` for the full list of configurable options.

## Usage example

```c
#include "vector_int.h"
#include "allocator.h"

int main(void) {
    vector_int v = vector_int_create(&libstructs_default_allocator);
    vector_int_push(&v, 42);
    int x = vector_int_get(&v, 0);
    vector_int_free(&v);
    return 0;
}
```

To use a custom allocator, for example one backed by a kernel's memory
manager, implement the `Allocator` interface described in `src/allocator.h`
and pass it to any `_create` function in place of the default.

## Documentation

API reference is generated with Doxygen from comments in the headers under
`src/` and `generated/`, and published automatically to Codeberg Pages on
every push to `main`.

Hosted docs: <https://datenbar.codeberg.page/libstructs/>

To build the same docs locally:

```bash
sudo pacman -S --needed doxygen
cmake --build build --target generate   # generated/ must exist first
doxygen Doxyfile
```

Output lands in `docs/doxygen/html/index.html`.

## Structures

| Structure | Status |
|---|---|
| Allocator abstraction | Planned |
| Vector | Planned |
| Stack, Queue | Planned |
| Singly and doubly linked list | Planned |
| Hash map, hash set | Planned |
| Binary search tree, heap, AVL tree | Planned |
| Graph, union-find | Planned |
| Trie | Planned, only if needed |

## Naming convention

Functions follow the pattern `struct_T_verb`, for example
`vector_int_push` or `hashMap_int_int_get`. This is kept consistent across
every structure so that a convenience dispatch layer can be added later
without special-casing individual structures.

## Non-goals

- Thread safety is not handled in the current version
- The library does not attempt to provide a general `void*`-based container
  type. Typed generation is a deliberate choice.

## License

MIT License. See `LICENSE`.

## Status

Early stage.
