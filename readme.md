uBlink
=======

[![Build Status](https://travis-ci.org/cjhdev/ublink.svg?branch=master)](https://travis-ci.org/cjhdev/ublink)

[Blink Protocol](http://www.blinkprotocol.org/ "Blink Protocol") in C.

## Highlights

- Hand coded schema parser and lexer
- Compact form encode/decode primitives
- Requires malloc but this can be a simple linear allocator
- User configurable IO streams
- Tests

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

# include a file before BLINK_ERROR and BLINK_DEBUG are defined (default: not defined)
DEFINES += -DBLINK_DEBUG_INCLUDE='#include <ruby.h>'

# remove all BLINK_DEBUG() and BLINK_ERROR() messages from code (default: not defined)
DEFINES += -DBLINK_NO_DEBUG_MESSAGE

# define your own BLINK_DEBUG() macro (default: defined as shown)
DEFINES += -DBLINK_DEBUG(...)='do{fprintf(stderr, __VA_ARGS__);fprintf(stderr, "\n");}while(0);'

# define your own BLINK_ERROR() macro (default: defined as shown)
DEFINES += -DBLINK_ERROR(...)='do{fprintf(stderr, __VA_ARGS__);fprintf(stderr, "\n");}while(0);'

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
    
