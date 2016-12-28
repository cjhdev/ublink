/**
 * @example tc_blink_stream_write.c
 *
 * */

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>

#include "cmocka.h"
#include "blink_stream.h"
#include <string.h>

int setupBuffer(void **user)
{
    static uint8_t buffer[sizeof("helloworld")];
    static struct blink_stream_output_buffer s;
    *user = (void *)BLINK_Stream_initOutputBuffer(&s, buffer, sizeof(buffer));
    return 0;
}

void test_BLINK_Stream_write_all(void **user)
{
    const char in[] = "helloworld";
    
    assert_true(BLINK_Stream_write((blink_stream_t)(*user), (const uint8_t *)in, sizeof(in)));
}

void test_BLINK_Stream_write_eof(void **user)
{
    const char in[] = "helloworld ";
    
    assert_false(BLINK_Stream_write((blink_stream_t)(*user), (const uint8_t *)in, sizeof(in)));
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup(test_BLINK_Stream_write_all, setupBuffer),        
        cmocka_unit_test_setup(test_BLINK_Stream_write_eof, setupBuffer),        
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
