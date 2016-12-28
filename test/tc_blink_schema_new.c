/**
 * @example tc_blink_schema_new.c
 *
 * */

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>

#include "cmocka.h"
#include "blink_schema.h"
#include <string.h>

int setup(void **user)
{
    static uint8_t heap[1024U];
    static struct blink_pool pool;
    *user = (void *)BLINK_Pool_init(&pool, heap, sizeof(heap));
    return 0;
}

void test_BLINK_Schema_new_emptyGroup(void **user)
{
    const char input[] = "empty";
    blink_schema_t schema = BLINK_Schema_new((blink_pool_t)(*user), input, sizeof(input));

    assert_true(schema != NULL);
    assert_true(BLINK_Schema_getGroupByName(schema, "empty") != NULL);
}

void test_BLINK_Schema_new_emptySuperGroup(void **user)
{
    const char input[] =
        "super\n"
        "empty : super";
    blink_schema_t schema = BLINK_Schema_new((blink_pool_t)(*user), input, sizeof(input));

    assert_true(schema != NULL);
    assert_true(BLINK_Schema_getGroupByName(schema, "empty") != NULL);
    assert_true(BLINK_Schema_getGroupByName(schema, "super") != NULL);
}

void test_BLINK_Schema_new_undefinedSuperGroup(void **user)
{
    const char input[] = "empty : super";
    blink_schema_t schema = BLINK_Schema_new((blink_pool_t)(*user), input, sizeof(input));

    assert_true(schema == NULL);
}

void test_BLINK_Schema_new_groupIsSuperGroup(void **user)
{
    const char input[] = "empty : empty";
    blink_schema_t schema = BLINK_Schema_new((blink_pool_t)(*user), input, sizeof(input));

    assert_true(schema == NULL);
}

void test_BLINK_Schema_new_groupIsSuperGroupByIntermediate(void **user)
{
    const char input[] =
        "intermediate = empty\n"
        "empty : intermediate";
    blink_schema_t schema = BLINK_Schema_new((blink_pool_t)(*user), input, sizeof(input));

    assert_true(schema == NULL);
}

void test_BLINK_Schema_new_superGroupIsDynamic(void **user)
{
    const char input[] =
        "super\n"
        "intermediate = super*\n"
        "empty : intermediate";
    blink_schema_t schema = BLINK_Schema_new((blink_pool_t)(*user), input, sizeof(input));

    assert_true(schema == NULL);
}

void test_BLINK_Schema_new_superGroupIsSequence(void **user)
{
    const char input[] =
        "super\n"
        "intermediate = super []\n"
        "empty : intermediate";
    blink_schema_t schema = BLINK_Schema_new((blink_pool_t)(*user), input, sizeof(input));

    assert_true(schema == NULL);
}

void test_BLINK_Schema_new_greeting(void **user)
{
    const char input[] = "Message/0 -> string Greeting";
    blink_schema_t schema = BLINK_Schema_new((blink_pool_t)(*user), input, sizeof(input));
    
    assert_true(schema != NULL);
    assert_true(BLINK_Schema_getGroupByName(schema, "Message") != NULL);
    assert_true(BLINK_Schema_getGroupByID(schema, 0) != NULL);
    
    assert_ptr_equal(BLINK_Schema_getGroupByID(schema, 0), BLINK_Schema_getGroupByName(schema, "Message"));

    struct blink_field_iterator iter;
    BLINK_FieldIterator_init(&iter, BLINK_Schema_getGroupByName(schema, "Message"));

    blink_field_t f = BLINK_FieldIterator_next(&iter);

    assert_true(f != NULL);

    const char *name = BLINK_Field_getName(f);
    enum blink_type_tag tag = BLINK_Field_getType(f);
    uint32_t size = BLINK_Field_getSize(f);
    
    assert_int_equal(BLINK_TYPE_STRING, tag);
    assert_int_equal(0xffffffff, size);

    assert_string_equal("Greeting", name);

    assert_false(BLINK_Field_isOptional(f));

    assert_true(BLINK_FieldIterator_next(&iter) == NULL);
}

void test_BLINK_Schema_new_namespace_emptyGroup(void **user)
{
    const char input[] = "namespace test empty";
    blink_schema_t schema = BLINK_Schema_new((blink_pool_t)(*user), input, sizeof(input));

    assert_true(schema != NULL);
    assert_true(BLINK_Schema_getGroupByName(schema, "test:empty") != NULL);
}

void test_BLINK_Schema_new_enum_single(void **user)
{
    const char input[] = "test = | lonely";
    blink_schema_t schema = BLINK_Schema_new((blink_pool_t)(*user), input, sizeof(input));

    assert_true(schema != NULL);
}

void test_BLINK_Schema_new_circular_type_reference(void **user)
{
    const char input[] =
        "test = testTwo\n"
        "testTwo = test";
    blink_schema_t schema = BLINK_Schema_new((blink_pool_t)(*user), input, sizeof(input));

    assert_true(schema == NULL);    
}

void test_BLINK_Schema_new_duplicate_type_definition(void **user)
{
    const char input[] =
        "test = u8\n"
        "test = u16";
    blink_schema_t schema = BLINK_Schema_new((blink_pool_t)(*user), input, sizeof(input));

    assert_true(schema == NULL);    
}

void test_BLINK_Schema_new_duplicate_type_group_definition(void **user)
{
    const char input[] =
        "test = u8\n"
        "test -> u16 field";
    blink_schema_t schema = BLINK_Schema_new((blink_pool_t)(*user), input, sizeof(input));

    assert_true(schema == NULL);    
}

void test_BLINK_Schema_new_duplicate_group_definition(void **user)
{
    const char input[] =
        "test -> u8 field\n"
        "test -> u16 field";
    blink_schema_t schema = BLINK_Schema_new((blink_pool_t)(*user), input, sizeof(input));

    assert_true(schema == NULL);    
}

void test_BLINK_Schema_new_duplicate_enum_definition(void **user)
{
    const char input[] =
        "test = | bla\n"
        "test = | bla";
    blink_schema_t schema = BLINK_Schema_new((blink_pool_t)(*user), input, sizeof(input));

    assert_true(schema == NULL);    
}

void test_BLINK_Schema_new_duplicate_enum_field(void **user)
{
    const char input[] = "test = bla | bla";
    blink_schema_t schema = BLINK_Schema_new((blink_pool_t)(*user), input, sizeof(input));

    assert_true(schema == NULL);    
}

void test_BLINK_Schema_new_ambiguous_enum_value(void **user)
{
    const char input[] = "Month = Jan/1 | Feb | Mar/2";
    blink_schema_t schema = BLINK_Schema_new((blink_pool_t)(*user), input, sizeof(input));

    assert_true(schema == NULL);    
}

void test_BLINK_Schema_new_enum_value_upperLimit(void **user)
{
    const char input[] = "Month = | Jan/2147483648";
    blink_schema_t schema = BLINK_Schema_new((blink_pool_t)(*user), input, sizeof(input));

    assert_true(schema == NULL);    
}

void test_BLINK_Schema_new_enum_value_lowerLimit(void **user)
{
    const char input[] = "Month = | Jan/-2147483649";
    blink_schema_t schema = BLINK_Schema_new((blink_pool_t)(*user), input, sizeof(input));

    assert_true(schema == NULL);    
}

void test_BLINK_Schema_new_duplicate_group_field(void **user)
{
    const char input[] = "test -> u8 bla, u8 bla";
    blink_schema_t schema = BLINK_Schema_new((blink_pool_t)(*user), input, sizeof(input));

    assert_true(schema == NULL);    
}

void test_BLINK_Schema_new_superGroupShadowField(void **user)
{
    const char input[] =
        "super -> u8 field\n"
        "test : super -> u16 field";
    blink_schema_t schema = BLINK_Schema_new((blink_pool_t)(*user), input, sizeof(input));

    assert_true(schema == NULL);
}

void test_BLINK_Schema_new_superSuperGroupShadowField(void **user)
{
    const char input[] =
        "superSuper -> u8 field\n"
        "super : superSuper -> u8 different\n"
        "test : super -> u16 field";
    blink_schema_t schema = BLINK_Schema_new((blink_pool_t)(*user), input, sizeof(input));

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


