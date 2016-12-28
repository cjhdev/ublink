/**
 * @example __FILE__
 *
 * */

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>

#include "cmocka.h"
#include "blink_schema.h"
#include "blink_debug.h"
#include <string.h>

int setup(void **user)
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
    
    static uint8_t heap[10240U];
    static struct blink_pool pool;
    *user = (void *)BLINK_Schema_new(BLINK_Pool_init(&pool, heap, sizeof(heap)), input, sizeof(input));
    return 0;
}

void test_BLINK_Schema_getGroupByID_insertOrder(void **user)
{
    const char name[] = "InsertOrder";
    blink_group_t g = BLINK_Schema_getGroupByID((blink_schema_t)(*user), 1);
    assert_true(g != NULL);
    assert_true(g == BLINK_Schema_getGroupByName((blink_schema_t)(*user), name));
}

void test_BLINK_Schema_getGroupByID_cancelOrder(void **user)
{
    const char name[] = "CancelOrder";
    blink_group_t g = BLINK_Schema_getGroupByID((blink_schema_t)(*user), 2);
    assert_true(g != NULL);
    assert_true(g == BLINK_Schema_getGroupByName((blink_schema_t)(*user), name));
}

void test_BLINK_Schema_getGroupByID_orderInserted(void **user)
{
    const char name[] = "OrderInserted";
    blink_group_t g = BLINK_Schema_getGroupByID((blink_schema_t)(*user), 3);
    assert_true(g != NULL);
    assert_true(g == BLINK_Schema_getGroupByName((blink_schema_t)(*user), name));
}

void test_BLINK_Schema_getGroupByID_orderCanceled(void **user)
{
    const char name[] = "OrderCanceled";
    blink_group_t g = BLINK_Schema_getGroupByID((blink_schema_t)(*user), 4);
    assert_true(g != NULL);
    assert_true(g == BLINK_Schema_getGroupByName((blink_schema_t)(*user), name));
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup(test_BLINK_Schema_getGroupByID_insertOrder, setup),
        cmocka_unit_test_setup(test_BLINK_Schema_getGroupByID_cancelOrder, setup),
        cmocka_unit_test_setup(test_BLINK_Schema_getGroupByID_orderInserted, setup),
        cmocka_unit_test_setup(test_BLINK_Schema_getGroupByID_orderCanceled, setup),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}