# pika

Simple, header-only argument parser library written in C, for C
that's as fast as getopt

Examples are in examples/

[basic.c](examples/basic.c) is an example of usage of this library,

[basic_getopt.c](examples/basic_getopt.c) is an example of the same thing BUT with getopt, so feel free to benchmark.

Pika uses a schema-oriented approach, so to use it
you must declare a static schema object,
that describes all of your supported arguments.

## Features

- Schema-oriented - declare your args once, get parsing, defaults, and an optional `--help` for free
- Header-only - just drop `pika.h` in your project and you're done
- No dynamic allocation - arena-based, stack allocated
- Typed getters - `PikaGetBool`, `PikaGetCStr`, `PikaGetU64`, etc.
- Ability to omit Pika prefix if that's what you want
- Auto-generated `--help`
- Short aliases, multi-alias, and long options
- Pretty error messages

## Getting started
Drop `pika.h` into your project, define `PIKA_OMIT_PREFIX` if you want unprefixed names, and declare a schema. See [examples/basic.c](examples/basic.c) for a full example.

## Supported Types

| Macro |  C type   |  Getter  |
|-------|-----------|----------|
| `BOOL`| `bool`    | `PikaGetBool`|
| `CSTR`| `char*`   | `PikaGetCStr`|
| `U32` | `uint32_t`| `PikaGetU32` |
| `I32` | `int32_t` | `PikaGetI32` |
| `U64` | `uint64_t`| `PikaGetU64` |
| `I64` | `int64_t` | `PikaGetI64` |

## Examples

- [examples/basic.c](examples/basic.c) — pika usage
- [examples/basic_getopt.c](examples/basic_getopt.c) — equivalent with getopt, for benchmarking

## Performance

Benchmarked with `hyperfine -N` against getopt — effectively identical. No heap allocations; all storage is stack-allocated via a fixed-size arena.
