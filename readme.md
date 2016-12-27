uBlink
=======

[![Build Status](https://travis-ci.org/cjhdev/ublink.svg?branch=master)](https://travis-ci.org/cjhdev/ublink)

UBlink is C library for building compact binary messages according to
Blink Protocol schemas. 

## Highlights

- Hand coded schema parser and lexer
- Compact form encode/decode primitives
- allocate/reset pool based allocator
- Tests
- Linted to MISRA 2012

## Example

~~~
#include "ublink.h"

#define ARBITRARY_HEAP_SIZE 1024U

/* initialise a pool for the schema */
uint8_t schemaHeap[ARBITRARY_HEAP_SIZE];
struct blink_pool schemaPool;
(void)BLINK_Pool_init(&schemaPool, schemaHeap, sizeof(schemaHeap));

const char syntax[] =
    "InsertOrder/1 ->\n"
    "   string Symbol,\n"
    "   string OrderId,\n"
    "   u32 Price,\n"
    "   u32 Quantity\n";

blink_schema_t schema = BLINK_Schema_new(&schemaPool, syntax, sizeof(syntax));

~~~

## Integrating With Your Project

Example makefile snippet:

~~~
INCLUDES += $(DIR_UBLINK)/include

VPATH += $(DIR_UBLINK)/src

SRC := $(wildcard $(DIR_UBLINK)/src/*.c)

OBJECTS += $(SRC:.c=.o)
~~~

Add `#include "ublink.h"` to source files that use the UBlink API.


### Compile Time Options

The following options can be defined at compile time. 

~~~
# remove asserts (default: not defined)
DEFINES += -DNDEBUG

# define the maximum number of references allowed in a chain (default: 10)
DEFINES += -DBLINK_LINK_DEPTH=10
~~~

## See Also

[SlowBlink](https://github.com/cjhdev/slow_blink "SlowBlink"): Blink Protocol in Ruby

## License

uBlink has an MIT license.

## Contact

contact@cjh.id.au
    
