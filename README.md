# pika

Simple, easy-to-use, header-only argument parser library written in C, for C
that's as fast as getopt

Examples are in examples/
[basic.c](examples/basic.c) is an example of usage of this library,
[basic_getopt.c](examples/basic_getopt.c) is an example of the same thing BUT with getopt, so feel free to benchmark.

Pika uses a schema-oriented approach, so to use it
you must declare a static schema object,
that describes all of your supported arguments.
