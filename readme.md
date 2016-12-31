uBlink
=======

[![Build Status](https://travis-ci.org/cjhdev/ublink.svg?branch=master)](https://travis-ci.org/cjhdev/ublink)

[Blink Protocol](http://www.blinkprotocol.org/ "Blink Protocol") in C.

## Highlights

- Hand coded schema parser and lexer
- Compact form encode/decode primitives
- Allocate/reset pool based allocator
- Message input/output streams
- Tests

## Example

~~~ C
#include "ublink.h"

blink_pool_t pool;
blink_schema_t schema;

static void setup(void)
{
    static uint8_t heap[ARBITRARY_HEAP_SIZE];
    static struct blink_pool p;
    pool = BLINK_Pool_init(&p, heap, sizeof(heap));

    static const char syntax[] =
        "InsertOrder/1 ->\n"
        "   string Symbol,\n"
        "   string OrderId,\n"
        "   u32 Price,\n"
        "   u32 Quantity\n";

    schema = BLINK_Schema_new(pool, syntax, sizeof(syntax));
}

void example(void)
{
    setup();
}
~~~

## Integrating With Your Project

Example makefile snippet:

``` mf
INCLUDES += $(DIR_UBLINK)/include

VPATH += $(DIR_UBLINK)/src

SRC += $(wildcard $(DIR_UBLINK)/src/*.c)

OBJECTS += $(SRC:.c=.o)
```

Add `#include "ublink.h"` to source files that use the UBlink API.


### Compile Time Options

The following options can be defined at compile time. 

``` mf
# remove asserts (default: not defined)
DEFINES += -DNDEBUG

# define the maximum number of references allowed in a chain (default: 10)
DEFINES += -DBLINK_LINK_DEPTH=10
```

## See Also

[SlowBlink](https://github.com/cjhdev/slow_blink "SlowBlink"): Blink Protocol in Ruby

## License

uBlink has an MIT license.

## Contact

contact@cjh.id.au
    
