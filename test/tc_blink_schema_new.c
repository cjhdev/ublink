/**
 * @example tc_blink_schema_new.c
 *
 * */

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>

#include "cmocka.h"
#include "blink_schema.h"
#include "blink_stream.h"
#include <string.h>

static int setup(void **user)
{
    static uint8_t heap[1024U];
    static struct blink_pool pool;
    *user = (void *)BLINK_Pool_init(&pool, heap, sizeof(heap));
    return 0;
}

static void test_BLINK_Schema_new_emptyGroup(void **user)
{
    const char input[] = "empty";
    struct blink_stream stream;
    (void)BLINK_Stream_initBufferReadOnly(&stream, (const uint8_t *)input, sizeof(input));
    
    blink_schema_t schema = BLINK_Schema_new((blink_pool_t)(*user), &stream);

    assert_true(schema != NULL);
}

static void test_BLINK_Schema_new_emptySuperGroup(void **user)
{
    struct blink_stream stream;
    const char input[] =
        "super\n"
        "empty : super";
    (void)BLINK_Stream_initBufferReadOnly(&stream, (const uint8_t *)input, sizeof(input));

        
    blink_schema_t schema = BLINK_Schema_new((blink_pool_t)(*user), &stream);

    assert_true(schema != NULL);    
}

static void test_BLINK_Schema_new_undefinedSuperGroup(void **user)
{
    struct blink_stream stream;
    const char input[] = "empty : super";
    (void)BLINK_Stream_initBufferReadOnly(&stream, (const uint8_t *)input, sizeof(input));
    
    blink_schema_t schema = BLINK_Schema_new((blink_pool_t)(*user), &stream);

    assert_true(schema == NULL);
}

static void test_BLINK_Schema_new_groupIsSuperGroup(void **user)
{
    struct blink_stream stream;
    const char input[] = "empty : empty";
    (void)BLINK_Stream_initBufferReadOnly(&stream, (const uint8_t *)input, sizeof(input));
    blink_schema_t schema = BLINK_Schema_new((blink_pool_t)(*user), &stream);

    assert_true(schema == NULL);
}

static void test_BLINK_Schema_new_groupIsSuperGroupByIntermediate(void **user)
{
    struct blink_stream stream;
    const char input[] =
        "intermediate = empty\n"
        "empty : intermediate";
    (void)BLINK_Stream_initBufferReadOnly(&stream, (const uint8_t *)input, sizeof(input));
    blink_schema_t schema = BLINK_Schema_new((blink_pool_t)(*user), &stream);

    assert_true(schema == NULL);
}

static void test_BLINK_Schema_new_superGroupIsDynamic(void **user)
{
    struct blink_stream stream;
    const char input[] =
        "super\n"
        "intermediate = super*\n"
        "empty : intermediate";
    (void)BLINK_Stream_initBufferReadOnly(&stream, (const uint8_t *)input, sizeof(input));
    blink_schema_t schema = BLINK_Schema_new((blink_pool_t)(*user), &stream);

    assert_true(schema == NULL);
}

static void test_BLINK_Schema_new_superGroupIsSequence(void **user)
{
    struct blink_stream stream;
    const char input[] =
        "super\n"
        "intermediate = super []\n"
        "empty : intermediate";
    (void)BLINK_Stream_initBufferReadOnly(&stream, (const uint8_t *)input, sizeof(input));
    blink_schema_t schema = BLINK_Schema_new((blink_pool_t)(*user), &stream);

    assert_true(schema == NULL);
}

static void test_BLINK_Schema_new_greeting(void **user)
{
    struct blink_stream stream;
    const char input[] = "Message/0 -> string Greeting";
    (void)BLINK_Stream_initBufferReadOnly(&stream, (const uint8_t *)input, sizeof(input));
    blink_schema_t schema = BLINK_Schema_new((blink_pool_t)(*user), &stream);
    
    assert_true(schema != NULL);    
}

static void test_BLINK_Schema_new_namespace_emptyGroup(void **user)
{
    struct blink_stream stream;
    const char input[] = "namespace test empty";
    (void)BLINK_Stream_initBufferReadOnly(&stream, (const uint8_t *)input, sizeof(input));
    blink_schema_t schema = BLINK_Schema_new((blink_pool_t)(*user), &stream);

    assert_true(schema != NULL);
}

static void test_BLINK_Schema_new_enum_single(void **user)
{
    struct blink_stream stream;
    const char input[] = "test = | lonely";
    (void)BLINK_Stream_initBufferReadOnly(&stream, (const uint8_t *)input, sizeof(input));
    blink_schema_t schema = BLINK_Schema_new((blink_pool_t)(*user), &stream);

    assert_true(schema != NULL);
}

static void test_BLINK_Schema_new_circular_type_reference(void **user)
{
    struct blink_stream stream;
    const char input[] =
        "test = testTwo\n"
        "testTwo = test";
    (void)BLINK_Stream_initBufferReadOnly(&stream, (const uint8_t *)input, sizeof(input));
    blink_schema_t schema = BLINK_Schema_new((blink_pool_t)(*user), &stream);

    assert_true(schema == NULL);    
}

static void test_BLINK_Schema_new_duplicate_type_definition(void **user)
{
    struct blink_stream stream;
    const char input[] =
        "test = u8\n"
        "test = u16";
    (void)BLINK_Stream_initBufferReadOnly(&stream, (const uint8_t *)input, sizeof(input));
    blink_schema_t schema = BLINK_Schema_new((blink_pool_t)(*user), &stream);

    assert_true(schema == NULL);    
}

static void test_BLINK_Schema_new_duplicate_type_group_definition(void **user)
{
    struct blink_stream stream;
    const char input[] =
        "test = u8\n"
        "test -> u16 field";
    (void)BLINK_Stream_initBufferReadOnly(&stream, (const uint8_t *)input, sizeof(input));
    blink_schema_t schema = BLINK_Schema_new((blink_pool_t)(*user), &stream);

    assert_true(schema == NULL);    
}

static void test_BLINK_Schema_new_duplicate_group_definition(void **user)
{
    struct blink_stream stream;
    const char input[] =
        "test -> u8 field\n"
        "test -> u16 field";
    (void)BLINK_Stream_initBufferReadOnly(&stream, (const uint8_t *)input, sizeof(input));
    blink_schema_t schema = BLINK_Schema_new((blink_pool_t)(*user), &stream);

    assert_true(schema == NULL);    
}

static void test_BLINK_Schema_new_duplicate_enum_definition(void **user)
{
    struct blink_stream stream;
    const char input[] =
        "test = | bla\n"
        "test = | bla";
    (void)BLINK_Stream_initBufferReadOnly(&stream, (const uint8_t *)input, sizeof(input));
    blink_schema_t schema = BLINK_Schema_new((blink_pool_t)(*user), &stream);

    assert_true(schema == NULL);    
}

static void test_BLINK_Schema_new_duplicate_enum_field(void **user)
{
    struct blink_stream stream;
    const char input[] = "test = bla | bla";
    (void)BLINK_Stream_initBufferReadOnly(&stream, (const uint8_t *)input, sizeof(input));
    blink_schema_t schema = BLINK_Schema_new((blink_pool_t)(*user), &stream);

    assert_true(schema == NULL);    
}

static void test_BLINK_Schema_new_ambiguous_enum_value(void **user)
{
    struct blink_stream stream;
    const char input[] = "Month = Jan/1 | Feb | Mar/2";
    (void)BLINK_Stream_initBufferReadOnly(&stream, (const uint8_t *)input, sizeof(input));
    blink_schema_t schema = BLINK_Schema_new((blink_pool_t)(*user), &stream);

    assert_true(schema == NULL);    
}

static void test_BLINK_Schema_new_enum_value_upperLimit(void **user)
{
    struct blink_stream stream;
    const char input[] = "Month = | Jan/2147483648";
    (void)BLINK_Stream_initBufferReadOnly(&stream, (const uint8_t *)input, sizeof(input));    
    blink_schema_t schema = BLINK_Schema_new((blink_pool_t)(*user), &stream);

    assert_true(schema == NULL);    
}

static void test_BLINK_Schema_new_enum_value_lowerLimit(void **user)
{
    struct blink_stream stream;
    const char input[] = "Month = | Jan/-2147483649";
    (void)BLINK_Stream_initBufferReadOnly(&stream, (const uint8_t *)input, sizeof(input));    
    blink_schema_t schema = BLINK_Schema_new((blink_pool_t)(*user), &stream);

    assert_true(schema == NULL);    
}

static void test_BLINK_Schema_new_duplicate_group_field(void **user)
{
    struct blink_stream stream;
    const char input[] = "test -> u8 bla, u8 bla";
    (void)BLINK_Stream_initBufferReadOnly(&stream, (const uint8_t *)input, sizeof(input));
    blink_schema_t schema = BLINK_Schema_new((blink_pool_t)(*user), &stream);

    assert_true(schema == NULL);    
}

static void test_BLINK_Schema_new_superGroupShadowField(void **user)
{
    struct blink_stream stream;
    const char input[] =
        "super -> u8 field\n"
        "test : super -> u16 field";
    (void)BLINK_Stream_initBufferReadOnly(&stream, (const uint8_t *)input, sizeof(input));        
    blink_schema_t schema = BLINK_Schema_new((blink_pool_t)(*user), &stream);

    assert_true(schema == NULL);
}

static void test_BLINK_Schema_new_superSuperGroupShadowField(void **user)
{
    struct blink_stream stream;
    const char input[] =
        "superSuper -> u8 field\n"
        "super : superSuper -> u8 different\n"
        "test : super -> u16 field";
    (void)BLINK_Stream_initBufferReadOnly(&stream, (const uint8_t *)input, sizeof(input));
    blink_schema_t schema = BLINK_Schema_new((blink_pool_t)(*user), &stream);

    assert_true(schema == NULL);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup(test_BLINK_Schema_new_emptyGroup, setup),
        cmocka_unit_test_setup(test_BLINK_Schema_new_emptySuperGroup, setup),
        cmocka_unit_test_setup(test_BLINK_Schema_new_undefinedSuperGroup, setup),
        cmocka_unit_test_setup(test_BLINK_Schema_new_groupIsSuperGroup, setup),
        cmocka_unit_test_setup(test_BLINK_Schema_new_groupIsSuperGroupByIntermediate, setup),
        cmocka_unit_test_setup(test_BLINK_Schema_new_superGroupIsDynamic, setup),
        cmocka_unit_test_setup(test_BLINK_Schema_new_superGroupIsSequence, setup),
        cmocka_unit_test_setup(test_BLINK_Schema_new_greeting, setup),
        cmocka_unit_test_setup(test_BLINK_Schema_new_namespace_emptyGroup, setup),
        cmocka_unit_test_setup(test_BLINK_Schema_new_enum_single, setup),
        cmocka_unit_test_setup(test_BLINK_Schema_new_circular_type_reference, setup),
        cmocka_unit_test_setup(test_BLINK_Schema_new_duplicate_type_definition, setup),
        cmocka_unit_test_setup(test_BLINK_Schema_new_duplicate_type_group_definition, setup),
        cmocka_unit_test_setup(test_BLINK_Schema_new_duplicate_group_definition, setup),
        cmocka_unit_test_setup(test_BLINK_Schema_new_duplicate_enum_definition, setup),
        cmocka_unit_test_setup(test_BLINK_Schema_new_duplicate_enum_field, setup),
        cmocka_unit_test_setup(test_BLINK_Schema_new_ambiguous_enum_value, setup),
        cmocka_unit_test_setup(test_BLINK_Schema_new_enum_value_upperLimit, setup),
        cmocka_unit_test_setup(test_BLINK_Schema_new_enum_value_lowerLimit, setup),
        cmocka_unit_test_setup(test_BLINK_Schema_new_duplicate_group_field, setup),
        cmocka_unit_test_setup(test_BLINK_Schema_new_superGroupShadowField, setup),
        cmocka_unit_test_setup(test_BLINK_Schema_new_superSuperGroupShadowField, setup),
    };
    return cmocka_run_group_tests(tests, setup, NULL);
}


