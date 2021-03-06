#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>

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

static void test_BLINK_Object_newGroup(void **user)
{
    blink_object_t retval = BLINK_Object_newGroup(&alloc, BLINK_Schema_getGroupByName((blink_schema_t)(*user), "InsertOrder"));

    assert_true(retval != NULL);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup(test_BLINK_Object_newGroup, setup),
    };
    
    return cmocka_run_group_tests(tests, NULL, NULL);
}
