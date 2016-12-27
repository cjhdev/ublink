/**
 * @example __FILE__
 *
 * */

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>

#include "cmocka.h"
#include "blink_schema.h"
#include <string.h>

#include <assert.h>

int setup(void **user)
{
    static uint8_t heap[500U];
    static struct blink_pool pool;
    *user = BLINK_Pool_init(&pool, heap, sizeof(heap));    
    return 0;
}

void test_BLINK_Group_isKindOf_is(void **user)
{
    const char input[] =
        "base\n"
        "derived : base";

    blink_schema_t schema = BLINK_Schema_new((blink_pool_t)(*user), input, sizeof(input));

    assert_true(schema != NULL);

    blink_group_t base = BLINK_Schema_getGroupByName(schema, "base");
    blink_group_t derived = BLINK_Schema_getGroupByName(schema, "derived");

    assert_true(base != NULL);
    assert_true(derived != NULL);

    assert_true(BLINK_Group_isKindOf(derived, base));    
}

void test_BLINK_Group_isKindOf_isMulti(void **user)
{
    const char input[] =
        "base\n"
        "inter : base -> u8 thing\n"
        "derived : inter";

    blink_schema_t schema = BLINK_Schema_new((blink_pool_t)(*user), input, sizeof(input));

    assert_true(schema != NULL);

    blink_group_t base = BLINK_Schema_getGroupByName(schema, "base");
    blink_group_t derived = BLINK_Schema_getGroupByName(schema, "derived");

    assert_true(base != NULL);
    assert_true(derived != NULL);

    assert_true(BLINK_Group_isKindOf(derived, base));    
}

void test_BLINK_Group_isKindOf_isNot(void **user)
{
    const char input[] =
        "base\n"
        "otherBase";

    blink_schema_t schema = BLINK_Schema_new((blink_pool_t)(*user), input, sizeof(input));

    assert_true(schema != NULL);

    blink_group_t base = BLINK_Schema_getGroupByName(schema, "base");
    blink_group_t otherBase = BLINK_Schema_getGroupByName(schema, "otherBase");

    assert_true(base != NULL);
    assert_true(otherBase != NULL);

    assert_false(BLINK_Group_isKindOf(otherBase, base));    
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup(test_BLINK_Group_isKindOf_is, setup),
        cmocka_unit_test_setup(test_BLINK_Group_isKindOf_isMulti, setup),
        cmocka_unit_test_setup(test_BLINK_Group_isKindOf_isNot, setup),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
