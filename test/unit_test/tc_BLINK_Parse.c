#include "unity.h"
#include "blink_parser.h"
#include <string.h>

static struct blink_schema ctxt;

void setUp(void)
{    
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
    TEST_ASSERT_EQUAL_PTR(&iter, BLINK_NewFieldIterator(BLINK_GetGroupByName(&ctxt, "Message", strlen("Message")), &iter));

    TEST_ASSERT_TRUE(BLINK_NextField(&iter) != NULL);
    TEST_ASSERT_TRUE(BLINK_NextField(&iter) == NULL);
}

void test_BLINK_Parse_namespace_emptyGroup(void)
{
    const char input[] = "namespace test empty";

    TEST_ASSERT_EQUAL_PTR(&ctxt, BLINK_Parse(&ctxt, input, sizeof(input)));
    TEST_ASSERT_TRUE(BLINK_GetGroupByName(&ctxt, "test:empty", strlen("test:empty")) != NULL);
}
