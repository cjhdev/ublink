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

    struct blink_stream stream;

    schema = BLINK_Schema_new(pool, BLINK_Stream_initBufferReadOnly(&stream, syntax, sizeof(syntax)));
}

void example(void)
{
    setup();

    blink_group_t msg = BLINK_Object_newGroup(pool, "InsertOrder");

    assert(msg != NULL);

    const char *symbol = "IBM";
    const char *orderID = "12345";

    (void)BLINK_Object_setString(msg, "Symbol", symbol, strlen(symbol));
    (void)BLINK_Object_setString(msg, "OrderID", orderID, strlen(orderID));
    (void)BLINK_Object_setUint(msg, "Price", 100);
    (void)BLINK_Object_setUint(msg, "Quantity", 5);
}
~~~

## Integrating With Your Project

Example makefile snippet:

~~~ mf
INCLUDES += $(DIR_UBLINK)/include

VPATH += $(DIR_UBLINK)/src

SRC += $(wildcard $(DIR_UBLINK)/src/*.c)

OBJECTS += $(SRC:.c=.o)
~~~

Add `#include "ublink.h"` to source files that use the UBlink API.


### Compile Time Options

The following options can be defined at compile time. 

~~~ mf
# remove asserts (default: not defined)
DEFINES += -DNDEBUG

# define the maximum number of references allowed in a chain (default: 10)
DEFINES += -DBLINK_LINK_DEPTH=10

# define the largest literal or name that can be handled by the lexer (default: 100)
DEFINES += -DBLINK_TOKEN_MAX_SIZE=100

# redefine the prefix (default: BLINK_)
# example: remove the prefix entirely
DEFINES += -DBLINK_
~~~

## See Also

[SlowBlink](https://github.com/cjhdev/slow_blink "SlowBlink"): Blink Protocol in Ruby

## License

uBlink has an MIT license.

## Contact

contact@cjh.id.au
    
