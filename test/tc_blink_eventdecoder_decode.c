/**
 * @example __FILE__
 *
 * */

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>

#include "cmocka.h"
#include "blink_event_decoder.h"
#include "blink_schema.h"
#include <string.h>

int setup(void **user)
{
    static const char syntax[] =
    "InsertOrder/1 ->\n"
    "   string Symbol,\n"
    "   string OrderId,\n"
    "   u32 Price,\n"
    "   u32 Quantity\n";
    
    static uint8_t heap[1024U];
    static struct blink_pool pool;
    *user = (void *)BLINK_Schema_new(BLINK_Pool_init(&pool, heap, sizeof(heap)), syntax, sizeof(syntax));
    return 0;
}

void test_BLINK_EventDecode(void **user)
{
    struct blink_decoder decoder;
    const uint8_t in[] = "\x0F\x01\x03\x49\x42\x4D\x06\x41\x42\x43\x31\x32\x33\x7D\xA8\x0F";

    assert_true(BLINK_EventDecoderInit(&decoder, NULL, (blink_schema_t)(*user), NULL) == &decoder);
    assert_int_equal(sizeof(in)-1U, BLINK_EventDecoderDecode(&decoder, in, sizeof(in)-1U));
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup(test_BLINK_EventDecode, setup),        
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
