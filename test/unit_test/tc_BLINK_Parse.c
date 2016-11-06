/**
 * @example tc_BLINK_Parse.c
 *
 * */

#include "unity.h"
#include "blink_parser.h"
#include <string.h>
#include <malloc.h>

static struct blink_schema ctxt;

void setUp(void)
{
    BLINK_InitSchema(&ctxt, calloc, free); 
}

void tearDown(void)
{
    BLINK_DestroySchema(&ctxt);
}

void test_BLINK_Parse_emptyGroup(void)
{
    const char input[] = "empty";

    TEST_ASSERT_EQUAL_PTR(&ctxt, BLINK_Parse(&ctxt, input, sizeof(input)));
    TEST_ASSERT_TRUE(BLINK_GetGroupByName(&ctxt, "empty", strlen("empty")) != NULL);
}

void test_BLINK_Parse_emptySuperGroup(void)
{
    const char input[] = "super empty : super";

    TEST_ASSERT_EQUAL_PTR(&ctxt, BLINK_Parse(&ctxt, input, sizeof(input)));
    TEST_ASSERT_TRUE(BLINK_GetGroupByName(&ctxt, "empty", strlen("empty")) != NULL);
    TEST_ASSERT_TRUE(BLINK_GetGroupByName(&ctxt, "super", strlen("super")) != NULL);
}

void test_BLINK_Parse_undefinedSuperGroup(void)
{
    const char input[] = "empty : super";

    TEST_ASSERT_EQUAL_PTR(NULL, BLINK_Parse(&ctxt, input, sizeof(input)));
}

void test_BLINK_Parse_greeting(void)
{
    const char input[] = "Message/0 -> string Greeting";
    
    TEST_ASSERT_EQUAL_PTR(&ctxt, BLINK_Parse(&ctxt, input, sizeof(input)));
    TEST_ASSERT_TRUE(BLINK_GetGroupByName(&ctxt, "Message", strlen("Message")) != NULL);
    TEST_ASSERT_TRUE(BLINK_GetGroupByID(&ctxt, 0) != NULL);
    TEST_ASSERT_EQUAL(BLINK_GetGroupByID(&ctxt, 0), BLINK_GetGroupByName(&ctxt, "Message", strlen("Message")));

    struct blink_field_iterator iter;
    TEST_ASSERT_EQUAL_PTR(&iter, BLINK_InitFieldIterator(&iter, BLINK_GetGroupByName(&ctxt, "Message", strlen("Message"))));

    const struct blink_field *f = BLINK_NextField(&iter);
    TEST_ASSERT_TRUE(f != NULL);

    size_t len;
    const char *name = BLINK_GetFieldName(f, &len);
    enum blink_type_tag tag = BLINK_GetFieldType(f);
    uint32_t size = BLINK_GetFieldSize(f);
    
    TEST_ASSERT_EQUAL(TYPE_STRING, tag);
    TEST_ASSERT_EQUAL(0xffffffff, size);

    TEST_ASSERT_EQUAL(strlen("Greeting"), len);
    TEST_ASSERT_EQUAL_STRING_LEN("Greeting", name, len);
    TEST_ASSERT_EQUAL_STRING_LEN("Greeting", name, len);

    TEST_ASSERT_FALSE(BLINK_FieldIsOptional(f));

    TEST_ASSERT_TRUE(BLINK_NextField(&iter) == NULL);
}

void test_BLINK_Parse_namespace_emptyGroup(void)
{
    const char input[] = "namespace test empty";

    TEST_ASSERT_EQUAL_PTR(&ctxt, BLINK_Parse(&ctxt, input, sizeof(input)));
    TEST_ASSERT_TRUE(BLINK_GetGroupByName(&ctxt, "test:empty", strlen("test:empty")) != NULL);
}

void test_BLINK_Parse_enum_single(void)
{
    const char input[] = "test = | lonely";

    TEST_ASSERT_EQUAL_PTR(&ctxt, BLINK_Parse(&ctxt, input, sizeof(input)));
}

void test_BLINK_Parse_circular_type_reference(void)
{
    const char input[] = "test = testTwo testTwo = test";

    TEST_ASSERT_EQUAL_PTR(NULL, BLINK_Parse(&ctxt, input, sizeof(input)));    
}

void test_BLINK_Parse_duplicate_type_definition(void)
{
    const char input[] = "test = u8 test = u16";

    TEST_ASSERT_EQUAL_PTR(NULL, BLINK_Parse(&ctxt, input, sizeof(input)));    
}

void test_BLINK_Parse_duplicate_type_group_definition(void)
{
    const char input[] = "test = u8 test -> u16 field";

    TEST_ASSERT_EQUAL_PTR(NULL, BLINK_Parse(&ctxt, input, sizeof(input)));    
}

void test_BLINK_Parse_duplicate_group_definition(void)
{
    const char input[] = "test -> u8 field test -> u16 field";

    TEST_ASSERT_EQUAL_PTR(NULL, BLINK_Parse(&ctxt, input, sizeof(input)));    
}

void test_BLINK_Parse_duplicate_enum_definition(void)
{
    const char input[] = "test = | bla test = | bla";

    TEST_ASSERT_EQUAL_PTR(NULL, BLINK_Parse(&ctxt, input, sizeof(input)));    
}

void test_BLINK_Parse_duplicate_enum_field(void)
{
    const char input[] = "test = bla | bla";

    TEST_ASSERT_EQUAL_PTR(NULL, BLINK_Parse(&ctxt, input, sizeof(input)));    
}

void test_BLINK_Parse_duplicate_group_field(void)
{
    const char input[] = "test -> u8 bla, u8 bla";

    TEST_ASSERT_EQUAL_PTR(NULL, BLINK_Parse(&ctxt, input, sizeof(input)));    
}


