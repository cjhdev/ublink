#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <string.h>

#include "cmocka.h"
#include "blink_object.h"
#include "blink_stream.h"
#include "blink_schema.h"

#include <malloc.h>

static struct blink_allocator alloc = {
    .calloc = calloc,
    .free = free
};

static int setup(void **user)
{
    static const char input[] =
        "InsertOrder/1 ->\n"
        "   string Symbol,\n"
        "   string OrderId,\n"
        "   u32 Price,\n"
        "   u32 Quantity\n"
        ""
        "CancelOrder/2 ->\n"
        "   string OrderId\n"
        ""
        "OrderInserted/3 ->\n"
        "   string OrderId\n"
        ""
        "OrderCanceled/4 ->\n"
        "   string OrderId\n";
    
    static struct blink_stream stream;
    (void)BLINK_Stream_initBufferReadOnly(&stream, (const uint8_t *)input, sizeof(input));
    *user = (void *)BLINK_Schema_new(&alloc, &stream);
    return 0;
}

static void test_BLINK_Object_encodeCompact(void **user)
{
    uint8_t buffer[100];
    struct blink_stream output;
    const uint8_t expected[] = "\x0F\x01\x03""IBM""\x06""ABC123""\x7D\xA8\x0F";

    (void)BLINK_Stream_initBuffer(&output, buffer, sizeof(buffer));

    blink_object_t group = BLINK_Object_newGroup(&alloc, BLINK_Schema_getGroupByName((blink_schema_t)(*user), "InsertOrder"));

    BLINK_Object_setString2(group, "Symbol", "IBM");
    BLINK_Object_setString2(group, "OrderId", "ABC123");
    BLINK_Object_setUint(group, "Price", 125U);
    BLINK_Object_setUint(group, "Quantity", 1000U);

    BLINK_Object_encodeCompact(group, &output);

    assert_int_equal(sizeof(expected)-1U, BLINK_Stream_tell(&output));
    assert_memory_equal(expected, buffer, sizeof(expected)-1U);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup(test_BLINK_Object_encodeCompact, setup),
    };
    
    return cmocka_run_group_tests(tests, NULL, NULL);
}
