/**
 * @example tc_blink_group_iskindof.c
 *
 * */

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>

#include "cmocka.h"
#include "blink_schema.h"
#include "blink_stream.h"
#include "blink_alloc.h"    
#include <string.h>

#include <assert.h>
#include <malloc.h>

static struct blink_allocator alloc = {
    .calloc = calloc,
    .free = free
};

static void test_BLINK_Group_isKindOf_is(void **user)
{
    const char input[] =
        "base\n"
        "derived : base";

    struct blink_stream stream;
    (void)BLINK_Stream_initBufferReadOnly(&stream, (const uint8_t *)input, sizeof(input));
    
    blink_schema_t schema = BLINK_Schema_new(&alloc, &stream);

    assert_true(schema != NULL);

    blink_schema_t base = BLINK_Schema_getGroupByName(schema, "base");
    blink_schema_t derived = BLINK_Schema_getGroupByName(schema, "derived");

    assert_true(base != NULL);
    assert_true(derived != NULL);

    assert_true(BLINK_Group_isKindOf(derived, base));    
}

static void test_BLINK_Group_isKindOf_isMulti(void **user)
{
    const char input[] =
        "base\n"
        "inter : base -> u8 thing\n"
        "derived : inter";

    struct blink_stream stream;
    (void)BLINK_Stream_initBufferReadOnly(&stream, (const uint8_t *)input, sizeof(input));

    blink_schema_t schema = BLINK_Schema_new(&alloc, &stream);

    assert_true(schema != NULL);

    blink_schema_t base = BLINK_Schema_getGroupByName(schema, "base");
    blink_schema_t derived = BLINK_Schema_getGroupByName(schema, "derived");

    assert_true(base != NULL);
    assert_true(derived != NULL);

    assert_true(BLINK_Group_isKindOf(derived, base));    
}

static void test_BLINK_Group_isKindOf_isNot(void **user)
{
    const char input[] =
        "base\n"
        "otherBase";

    struct blink_stream stream;
    (void)BLINK_Stream_initBufferReadOnly(&stream, (const uint8_t *)input, sizeof(input));

    blink_schema_t schema = BLINK_Schema_new(&alloc, &stream);

    assert_true(schema != NULL);

    blink_schema_t base = BLINK_Schema_getGroupByName(schema, "base");
    blink_schema_t otherBase = BLINK_Schema_getGroupByName(schema, "otherBase");

    assert_true(base != NULL);
    assert_true(otherBase != NULL);

    assert_false(BLINK_Group_isKindOf(otherBase, base));    
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_BLINK_Group_isKindOf_is),
        cmocka_unit_test(test_BLINK_Group_isKindOf_isMulti),
        cmocka_unit_test(test_BLINK_Group_isKindOf_isNot),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
