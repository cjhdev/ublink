/**
 * @example tc_BLINK_SchemaGetGroupByID.c
 *
 * */

#include "unity.h"
#include "blink_parser.h"
#include <string.h>
#include <malloc.h>

static struct blink_schema ctxt;

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

void setUp(void)
{
    BLINK_SchemaParse(BLINK_SchemaInit(&ctxt, calloc, free), input, sizeof(input));
}

void tearDown(void)
{
    BLINK_SchemaDestroy(&ctxt);
}

void test_BLINK_SchemaGetGroupByID_insertOrder(void)
{
    const char name[] = "InsertOrder";
    const struct blink_group *g = BLINK_SchemaGetGroupByID(&ctxt, 1);
    TEST_ASSERT_TRUE(g != NULL);
    TEST_ASSERT_TRUE(g == BLINK_SchemaGetGroupByName(&ctxt, name, strlen(name)));
}

void test_BLINK_SchemaGetGroupByID_cancelOrder(void)
{
    const char name[] = "CancelOrder";
    const struct blink_group *g = BLINK_SchemaGetGroupByID(&ctxt, 2);
    TEST_ASSERT_TRUE(g != NULL);
    TEST_ASSERT_TRUE(g == BLINK_SchemaGetGroupByName(&ctxt, name, strlen(name)));
}

void test_BLINK_SchemaGetGroupByID_orderInserted(void)
{
    const char name[] = "OrderInserted";
    const struct blink_group *g = BLINK_SchemaGetGroupByID(&ctxt, 3);
    TEST_ASSERT_TRUE(g != NULL);
    TEST_ASSERT_TRUE(g == BLINK_SchemaGetGroupByName(&ctxt, name, strlen(name)));
}

void test_BLINK_SchemaGetGroupByID_orderCanceled(void)
{
    const char name[] = "OrderCanceled";
    const struct blink_group *g = BLINK_SchemaGetGroupByID(&ctxt, 4);
    TEST_ASSERT_TRUE(g != NULL);
    TEST_ASSERT_TRUE(g == BLINK_SchemaGetGroupByName(&ctxt, name, strlen(name)));
}
