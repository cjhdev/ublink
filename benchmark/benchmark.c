#include "ublink.h"

#include <stdlib.h>
#include <sys/time.h>
#include <sys/resource.h>

#include <malloc.h>
#include <string.h>

double get_time()
{
    struct timeval t;
    struct timezone tzp;
    gettimeofday(&t, &tzp);
    return t.tv_sec + t.tv_usec*1e-6;
}

#define REPEATS 1000000

struct blink_allocator alloc = {
    .calloc = calloc
};

int main(int argc, const char **argv)
{
    uint8_t outbuf[100U];
    blink_schema_t schema;
    struct blink_stream stream;
    const char syntax[] = 
    "InsertOrder/1 ->\n"
            "string Symbol,  # set to 'IBM'\n"
            "string OrderId, # set to 'ABC123'\n"
            "u32 Price,      # set to 125\n"
            "u32 Quantity    # set to 1000\n";

    const uint8_t compact_form[] = "\x0f\x01\x03\x49\x42\x4d\x06\x41\x42\x43\x31\x32\x33\x7d\xa8\x0f";

    int i;

    double start = get_time();

    /* parse */
    for(i=0; i < REPEATS; i++){

        (void)BLINK_Stream_initBufferReadOnly(&stream, syntax, sizeof(syntax));
        schema = BLINK_Schema_new(&alloc, &stream);
    }

    double end = get_time();

    printf("parse: %g seconds\n", end-start);

    start = get_time();

    blink_object_t obj;

    /* new */
    for(i=0; i < REPEATS; i++){

        obj = BLINK_Object_newGroup(&alloc, BLINK_Schema_getGroupByName(schema, "InsertOrder"));
    }

    end = get_time();

    printf("new group: %g seconds\n", end-start);

    start = get_time();

    

    /* init and encode */
    for(i=0; i < REPEATS; i++){

        (void)BLINK_Stream_initBuffer(&stream, outbuf, sizeof(outbuf));

        BLINK_Object_setString2(obj, "Symbol", "IBM");
        BLINK_Object_setString2(obj, "OrderId", "ABC123");
        BLINK_Object_setUint(obj, "Price", 125U);
        BLINK_Object_setUint(obj, "Quantity", 1000U);

        BLINK_Object_encodeCompact(obj, &stream);
    }

    end = get_time();

    printf("init and encode: %g seconds\n", end-start);


    start = get_time();

    for(i=0; i < REPEATS; i++){

        (void)BLINK_Stream_initBufferReadOnly(&stream, compact_form, sizeof(compact_form));

        obj = BLINK_Object_decodeCompact(&stream, schema, &alloc);
    }

    end = get_time();

    printf("decode: %g seconds \n", end-start);


    exit(EXIT_SUCCESS);    
}
