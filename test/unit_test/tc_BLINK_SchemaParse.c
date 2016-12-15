/**
 * @example tc_BLINK_SchemaParse.c
 *
 * */

#include "unity.h"
#include "blink_parser.h"
#include <string.h>
#include <malloc.h>

static struct blink_schema ctxt;

void setUp(void)
{
    BLINK_SchemaInit(&ctxt, calloc, free); 
}

void tearDown(void)
{
    BLINK_SchemaDestroy(&ctxt);
}

void test_BLINK_SchemaParse_emptyGroup(void)
{
    const char input[] = "empty";

    TEST_ASSERT_EQUAL_PTR(&ctxt, BLINK_SchemaParse(&ctxt, input, sizeof(input)));
    TEST_ASSERT_TRUE(BLINK_SchemaGetGroupByName(&ctxt, "empty", strlen("empty")) != NULL);
}

void test_BLINK_SchemaParse_emptySuperGroup(void)
{
    const char input[] =
        "super\n"
        "empty : super";

    TEST_ASSERT_EQUAL_PTR(&ctxt, BLINK_SchemaParse(&ctxt, input, sizeof(input)));
    TEST_ASSERT_TRUE(BLINK_SchemaGetGroupByName(&ctxt, "empty", strlen("empty")) != NULL);
    TEST_ASSERT_TRUE(BLINK_SchemaGetGroupByName(&ctxt, "super", strlen("super")) != NULL);
}

void test_BLINK_SchemaParse_undefinedSuperGroup(void)
{
    const char input[] = "empty : super";

    TEST_ASSERT_EQUAL_PTR(NULL, BLINK_SchemaParse(&ctxt, input, sizeof(input)));
}

void test_BLINK_SchemaParse_groupIsSuperGroup(void)
{
    const char input[] = "empty : empty";

    TEST_ASSERT_EQUAL_PTR(NULL, BLINK_SchemaParse(&ctxt, input, sizeof(input)));
}

void test_BLINK_SchemaParse_groupIsSuperGroupByIntermediate(void)
{
    const char input[] =
        "intermediate = empty\n"
        "empty : intermediate";

    TEST_ASSERT_EQUAL_PTR(NULL, BLINK_SchemaParse(&ctxt, input, sizeof(input)));
}

void test_BLINK_SchemaParse_superGroupIsDynamic(void)
{
    const char input[] =
        "super\n"
        "intermediate = super*\n"
        "empty : intermediate";

    TEST_ASSERT_EQUAL_PTR(NULL, BLINK_SchemaParse(&ctxt, input, sizeof(input)));
}

void test_BLINK_SchemaParse_superGroupIsSequence(void)
{
    const char input[] =
        "super\n"
        "intermediate = super []\n"
        "empty : intermediate";

    TEST_ASSERT_EQUAL_PTR(NULL, BLINK_SchemaParse(&ctxt, input, sizeof(input)));
}

void test_BLINK_SchemaParse_greeting(void)
{
    const char input[] = "Message/0 -> string Greeting";
    
    TEST_ASSERT_EQUAL_PTR(&ctxt, BLINK_SchemaParse(&ctxt, input, sizeof(input)));
    TEST_ASSERT_TRUE(BLINK_SchemaGetGroupByName(&ctxt, "Message", strlen("Message")) != NULL);
    TEST_ASSERT_TRUE(BLINK_SchemaGetGroupByID(&ctxt, 0) != NULL);
    TEST_ASSERT_EQUAL(BLINK_SchemaGetGroupByID(&ctxt, 0), BLINK_SchemaGetGroupByName(&ctxt, "Message", strlen("Message")));

    struct blink_field_iterator iter;
    BLINK_FieldIteratorInit(&iter, BLINK_SchemaGetGroupByName(&ctxt, "Message", strlen("Message")));

    const struct blink_field *f = BLINK_FieldIteratorNext(&iter);
    TEST_ASSERT_TRUE(f != NULL);

    size_t len;
    const char *name = BLINK_FieldGetName(f, &len);
    enum blink_type_tag tag = BLINK_FieldGetType(f);
    uint32_t size = BLINK_FieldGetSize(f);
    
    TEST_ASSERT_EQUAL(BLINK_TYPE_STRING, tag);
    TEST_ASSERT_EQUAL(0xffffffff, size);

    TEST_ASSERT_EQUAL(strlen("Greeting"), len);
    TEST_ASSERT_EQUAL_STRING_LEN("Greeting", name, len);
    TEST_ASSERT_EQUAL_STRING_LEN("Greeting", name, len);

    TEST_ASSERT_FALSE(BLINK_FieldGetIsOptional(f));

    TEST_ASSERT_TRUE(BLINK_FieldIteratorNext(&iter) == NULL);
}

void test_BLINK_SchemaParse_namespace_emptyGroup(void)
{
    const char input[] = "namespace test empty";

    TEST_ASSERT_EQUAL_PTR(&ctxt, BLINK_SchemaParse(&ctxt, input, sizeof(input)));
    TEST_ASSERT_TRUE(BLINK_SchemaGetGroupByName(&ctxt, "test:empty", strlen("test:empty")) != NULL);
}

void test_BLINK_SchemaParse_enum_single(void)
{
    const char input[] = "test = | lonely";

    TEST_ASSERT_EQUAL_PTR(&ctxt, BLINK_SchemaParse(&ctxt, input, sizeof(input)));
}

void test_BLINK_SchemaParse_circular_type_reference(void)
{
    const char input[] =
        "test = testTwo\n"
        "testTwo = test";

    TEST_ASSERT_EQUAL_PTR(NULL, BLINK_SchemaParse(&ctxt, input, sizeof(input)));    
}

void test_BLINK_SchemaParse_duplicate_type_definition(void)
{
    const char input[] =
        "test = u8\n"
        "test = u16";

    TEST_ASSERT_EQUAL_PTR(NULL, BLINK_SchemaParse(&ctxt, input, sizeof(input)));    
}

void test_BLINK_SchemaParse_duplicate_type_group_definition(void)
{
    const char input[] =
        "test = u8\n"
        "test -> u16 field";

    TEST_ASSERT_EQUAL_PTR(NULL, BLINK_SchemaParse(&ctxt, input, sizeof(input)));    
}

void test_BLINK_SchemaParse_duplicate_group_definition(void)
{
    const char input[] =
        "test -> u8 field\n"
        "test -> u16 field";

    TEST_ASSERT_EQUAL_PTR(NULL, BLINK_SchemaParse(&ctxt, input, sizeof(input)));    
}

void test_BLINK_SchemaParse_duplicate_enum_definition(void)
{
    const char input[] =
        "test = | bla\n"
        "test = | bla";

    TEST_ASSERT_EQUAL_PTR(NULL, BLINK_SchemaParse(&ctxt, input, sizeof(input)));    
}

void test_BLINK_SchemaParse_duplicate_enum_field(void)
{
    const char input[] = "test = bla | bla";

    TEST_ASSERT_EQUAL_PTR(NULL, BLINK_SchemaParse(&ctxt, input, sizeof(input)));    
}

void test_BLINK_SchemaParse_ambiguous_enum_value(void)
{
    const char input[] = "Month = Jan/1 | Feb | Mar/2";

    TEST_ASSERT_EQUAL_PTR(NULL, BLINK_SchemaParse(&ctxt, input, sizeof(input)));    
}

void test_BLINK_SchemaParse_enum_value_upperLimit(void)
{
    const char input[] = "Month = | Jan/2147483648";

    TEST_ASSERT_EQUAL_PTR(NULL, BLINK_SchemaParse(&ctxt, input, sizeof(input)));    
}

void test_BLINK_SchemaParse_enum_value_lowerLimit(void)
{
    const char input[] = "Month = | Jan/-2147483649";

    TEST_ASSERT_EQUAL_PTR(NULL, BLINK_SchemaParse(&ctxt, input, sizeof(input)));    
}

void test_BLINK_SchemaParse_duplicate_group_field(void)
{
    const char input[] = "test -> u8 bla, u8 bla";

    TEST_ASSERT_EQUAL_PTR(NULL, BLINK_SchemaParse(&ctxt, input, sizeof(input)));    
}

void test_BLINK_SchemaParse_superGroupShadowField(void)
{
    const char input[] =
        "super -> u8 field\n"
        "test : super -> u16 field";

    TEST_ASSERT_EQUAL_PTR(NULL, BLINK_SchemaParse(&ctxt, input, sizeof(input)));
}

void test_BLINK_SchemaParse_superSuperGroupShadowField(void)
{
    const char input[] =
        "superSuper -> u8 field\n"
        "super : superSuper -> u8 different\n"
        "test : super -> u16 field";

    TEST_ASSERT_EQUAL_PTR(NULL, BLINK_SchemaParse(&ctxt, input, sizeof(input)));
}



