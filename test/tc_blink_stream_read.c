/**
 * @example tc_blink_stream_read.c
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
    static const uint8_t buffer[] = "helloworld";
    static struct blink_stream s;
    *user = (void *)BLINK_Stream_initBufferReadOnly(&s, buffer, sizeof(buffer));
    return 0;
}

void test_BLINK_Stream_read_all(void **user)
{
    uint8_t expected[] = "helloworld";
    uint8_t out[sizeof(expected)];
    
    assert_true(BLINK_Stream_read((blink_stream_t)(*user), out, sizeof(out)));
    assert_memory_equal(out, expected, sizeof(expected));
}

void test_BLINK_Stream_read_allParts(void **user)
{
    const char expectedFirst[] = "hello";
    const char expectedSecond[] = "world";
    const char expectedThird[] = "";
    uint8_t out[sizeof("helloworld")];
    
    assert_true(BLINK_Stream_read((blink_stream_t)(*user), out, strlen(expectedFirst)));
    assert_memory_equal(out, expectedFirst, strlen(expectedFirst));

    assert_true(BLINK_Stream_read((blink_stream_t)(*user), out, strlen(expectedSecond)));
    assert_memory_equal(out, expectedSecond, strlen(expectedSecond));

    assert_true(BLINK_Stream_read((blink_stream_t)(*user), out, sizeof(expectedThird)));
    assert_memory_equal(out, expectedThird, sizeof(expectedThird));
}

void test_BLINK_Stream_read_eof(void **user)
{
    uint8_t expected[] = "helloworld";
    uint8_t out[sizeof(expected)+1U];
    
    assert_false(BLINK_Stream_read((blink_stream_t)(*user), out, sizeof(out)));
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup(test_BLINK_Stream_read_all, setupBuffer),        
        cmocka_unit_test_setup(test_BLINK_Stream_read_allParts, setupBuffer),        
        cmocka_unit_test_setup(test_BLINK_Stream_read_eof, setupBuffer),        
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
