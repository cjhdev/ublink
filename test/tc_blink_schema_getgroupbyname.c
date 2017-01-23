/**
 * @example tc_blink_schema_getgroupbyname.c
 *
 * */

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>

#include "cmocka.h"
#include "blink_schema.h"
#include "blink_stream.h"
#include "blink_pool.h"
#include <string.h>

static int setup(void **user)
{
    static const char input[] =
        "InsertOrder/1 ->\n"
        "   string Symbol,\n"
        "   string OrderId,\n"
        "   u32 Price,\n"
        "   u32 Quantity\n"
        "\n"
        "CancelOrder/2 ->\n"
        "   string OrderId\n"
        "\n"
        "OrderInserted/3 ->\n"
        "   string OrderId\n"
        "\n"
        "OrderCanceled/4 ->\n"
        "   string OrderId\n";
    
    static uint8_t heap[2048U];
    static struct blink_pool pool;
    static struct blink_stream stream;
    (void)BLINK_Stream_initBufferReadOnly(&stream, (const uint8_t *)input, sizeof(input));
    *user = (void *)BLINK_Schema_new(BLINK_Pool_init(&pool, heap, sizeof(heap)), &stream);
    return 0;
}

static void test_BLINK_Schema_getGroupByName_insertOrder(void **user)
{
    const char name[] = "InsertOrder";
    blink_schema_t g = BLINK_Schema_getGroupByName((blink_schema_t)(*user), name);
    assert_true(g != NULL);
}

static void test_BLINK_Schema_getGroupByName_cancelOrder(void **user)
{
    const char name[] = "CancelOrder";
    blink_schema_t g = BLINK_Schema_getGroupByName((blink_schema_t)(*user), name);
    assert_true(g != NULL);
}

static void test_BLINK_Schema_getGroupByName_orderInserted(void **user)
{
    const char name[] = "OrderInserted";
    blink_schema_t g = BLINK_Schema_getGroupByName((blink_schema_t)(*user), name);
    assert_true(g != NULL);
}

static void test_BLINK_Schema_getGroupByName_orderCanceled(void **user)
{
    const char name[] = "OrderCanceled";
    blink_schema_t g = BLINK_Schema_getGroupByName((blink_schema_t)(*user), name);
    assert_true(g != NULL);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup(test_BLINK_Schema_getGroupByName_insertOrder, setup),
        cmocka_unit_test_setup(test_BLINK_Schema_getGroupByName_cancelOrder, setup),
        cmocka_unit_test_setup(test_BLINK_Schema_getGroupByName_orderInserted, setup),
        cmocka_unit_test_setup(test_BLINK_Schema_getGroupByName_orderCanceled, setup),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}


