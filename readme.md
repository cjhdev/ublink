uBlink
=======

[![Build Status](https://travis-ci.org/cjhdev/ublink.svg?branch=master)](https://travis-ci.org/cjhdev/ublink)

uBlink is a C99 implementation of [The Blink Protocol](http://www.blinkprotocol.org/ "The Blink Protocol") suitable for embedded applications.

## Highlights

- Compact form encode/decode primitives
- Hand coded schema parser and lexer
- Allocate-only malloc minimum requirement (i.e. you can use a simple heap that is reset instead of calling free)
- Tests and coverage statistics
- Linted to MISRA 2012

## Compile Time Options

~~~c
// remove asserts (default: not defined)
#define NDEBUG

// define the maximum inheritence depth (default: 10)
#define BLINK_INHERIT_DEPTH 10

// define the maximum number of references allowed in a chain (default: 10)
#define BLINK_LINK_DEPTH    10

// define the maximum number of nested groups in a message (default: 10)
#define BLINK_NEST_DEPTH    10
~~~

## Todo

- Overflow protection for parsing signed and unsigned integers in blink_lexer.c
- Location support in blink_parser.c
- Event driven decoder

## See Also

[SlowBlink](https://github.com/cjhdev/slow_blink "SlowBlink"): Blink Protocol in Ruby

## License

uBlink has an MIT license.

## Contact

contact@cjh.id.au
    
