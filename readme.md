uBlink
=======

uBlink is a C99 implementation of [The Blink Protocol](http://www.blinkprotocol.org/ "The Blink Protocol") suitable for resource constrained applications.

This project is currently under development and not very useful.

## Highlights

- Zero-copy schema definition parser
- Event driven compact form decoder
- Compact form encode/decode functions

## Examples

### Create a blink_schema object from a single schema definition

~~~c
#include "blink_schema.h"

const char syntax[] = "Hello/0 -> string greeting";

struct blink_schema s;
BLINK_NewSchema(&s);
BLINK_Parse(&s, syntax, sizeof(syntax));
BLINK_DestroySchema(&s);
~~~

## Compile Time Options

~~~c
// remove asserts (default: not defined)
#define NDEBUG

// define the maximum inheritence depth (default: 10)
#define BLINK_INHERIT_DEPTH   10
~~~

## License

uBlink has an MIT license.

## Contact

contact@cjh.id.au
    
