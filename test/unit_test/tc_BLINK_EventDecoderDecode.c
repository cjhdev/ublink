/**
 * @example tc_BLINK_EventDecoderDecode.c
 *
 * */

#include "unity.h"
#include "blink_event_decoder.h"
#include "blink_parser.h"
#include <string.h>
#include <malloc.h>

static struct blink_schema ctxt;

static const char InsertOrderSchema[] =
    "InsertOrder/1 ->\n"
    "   string Symbol,\n"
    "   string OrderId,\n"
    "   u32 Price,\n"
    "   u32 Quantity\n";

void setUp(void)
{
    BLINK_SchemaInit(&ctxt, calloc, free);
    BLINK_SchemaParse(&ctxt, InsertOrderSchema, sizeof(InsertOrderSchema));
}

void tearDown(void)
{
    BLINK_SchemaDestroy(&ctxt);
}

void test_BLINK_EventDecode(void)
{
    struct blink_decoder decoder;
    const uint8_t in[] = "\x0F\x01\x03\x49\x42\x4D\x06\x41\x42\x43\x31\x32\x33\x7D\xA8\x0F";

    TEST_ASSERT_EQUAL(&decoder, BLINK_EventDecoderInit(&decoder, NULL, &ctxt, NULL));
    TEST_ASSERT_EQUAL(sizeof(in)-1U, BLINK_EventDecoderDecode(&decoder, in, sizeof(in)-1U));
    
}

