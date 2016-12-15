/**
 * @example tc_BLINK_GroupIsKindOf.c
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

void test_BLINK_GroupIsKindOf_is(void)
{
    const char input[] =
        "base\n"
        "derived : base";

    TEST_ASSERT_EQUAL_PTR(&ctxt, BLINK_SchemaParse(&ctxt, input, sizeof(input)));

    const struct blink_group *base = BLINK_SchemaGetGroupByName(&ctxt, "base", strlen("base"));
    const struct blink_group *derived = BLINK_SchemaGetGroupByName(&ctxt, "derived", strlen("derived"));

    TEST_ASSERT_TRUE(base != NULL);
    TEST_ASSERT_TRUE(derived != NULL);

    TEST_ASSERT_TRUE(BLINK_GroupIsKindOf(derived, base));    
}

void test_BLINK_GroupIsKindOf_isMulti(void)
{
    const char input[] =
        "base\n"
        "inter : base -> u8 thing\n"
        "derived : inter";

    TEST_ASSERT_EQUAL_PTR(&ctxt, BLINK_SchemaParse(&ctxt, input, sizeof(input)));

    const struct blink_group *base = BLINK_SchemaGetGroupByName(&ctxt, "base", strlen("base"));
    const struct blink_group *derived = BLINK_SchemaGetGroupByName(&ctxt, "derived", strlen("derived"));

    TEST_ASSERT_TRUE(base != NULL);
    TEST_ASSERT_TRUE(derived != NULL);

    TEST_ASSERT_TRUE(BLINK_GroupIsKindOf(derived, base));    
}

void test_BLINK_GroupIsKindOf_isNot(void)
{
    const char input[] =
        "base\n"
        "otherBase";

    TEST_ASSERT_EQUAL_PTR(&ctxt, BLINK_SchemaParse(&ctxt, input, sizeof(input)));

    const struct blink_group *base = BLINK_SchemaGetGroupByName(&ctxt, "base", strlen("base"));
    const struct blink_group *otherBase = BLINK_SchemaGetGroupByName(&ctxt, "otherBase", strlen("otherBase"));

    TEST_ASSERT_TRUE(base != NULL);
    TEST_ASSERT_TRUE(otherBase != NULL);

    TEST_ASSERT_FALSE(BLINK_GroupIsKindOf(otherBase, base));    
}