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
    
    struct blink_stream stream;
    (void)BLINK_Stream_initBufferReadOnly(&stream, (const uint8_t *)input, sizeof(input));
    *user = (void *)BLINK_Schema_new(&alloc, &stream);
    return 0;
}

static void test_BLINK_Object_decodeCompact(void **user)
{
    struct blink_stream input;
    const uint8_t buffer[] = "\x0F\x01\x03""IBM""\x06""ABC123""\x7D\xA8\x0F";

    (void)BLINK_Stream_initBufferReadOnly(&input, buffer, sizeof(buffer));

    blink_object_t group = BLINK_Object_decodeCompact(&input, (blink_schema_t)(*user), &alloc);

    assert_true(group != NULL);

    assert_false(BLINK_Object_fieldIsNull(group, "Symbol"));
    assert_false(BLINK_Object_fieldIsNull(group, "OrderId"));
    assert_false(BLINK_Object_fieldIsNull(group, "Price"));
    assert_false(BLINK_Object_fieldIsNull(group, "Quantity"));

        
    const char *symbol;
    uint32_t symbolLen;
    const char *orderid;
    uint32_t orderidLen;

    BLINK_Object_getString(group, "Symbol", &symbol, &symbolLen);
    BLINK_Object_getString(group, "OrderId", &orderid, &orderidLen);

    assert_int_equal(3U, symbolLen);
    assert_memory_equal("IBM", symbol, symbolLen);

    assert_int_equal(6U, orderidLen);    
    assert_memory_equal("ABC123", orderid, orderidLen);
    
    assert_int_equal(125U, BLINK_Object_getUint(group, "Price"));
    assert_int_equal(1000U, BLINK_Object_getUint(group, "Quantity"));
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup(test_BLINK_Object_decodeCompact, setup),
    };
    
    return cmocka_run_group_tests(tests, NULL, NULL);
}
