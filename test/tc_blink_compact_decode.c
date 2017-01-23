/**
 * @example tc_blink_compact_decode.c
 *
 * */

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>

#include "cmocka.h"
#include "blink_compact.h"
#include "blink_stream.h"

static int setupSingleByteZero(void **user)
{
    static const uint8_t in[] = {0x00U};
    static struct blink_stream s;
    *user = (void *)BLINK_Stream_initBufferReadOnly(&s, in, sizeof(in));
    return 0;
}

static void test_BLINK_Compact_decodeU8(void **user)
{
    uint8_t out;
    bool isNull = true;
    uint8_t expectedOut = 0x00;

    assert_true(BLINK_Compact_decodeU8((blink_stream_t)(*user), &out, &isNull));
    assert_int_equal(expectedOut, out);
    assert_false(isNull);
}

static void test_BLINK_Compact_decodeU8_max(void **user)
{
    static const uint8_t in[] = {0xbf, 0x03};
    struct blink_stream stream;
    blink_stream_t s = BLINK_Stream_initBufferReadOnly(&stream, in, sizeof(in));    
    uint8_t out;
    bool isNull = true;
    uint8_t expectedOut = UINT8_MAX;

    assert_true(BLINK_Compact_decodeU8(s, &out, &isNull));
    assert_int_equal(expectedOut, out);
    assert_false(isNull);
}

static void test_BLINK_Compact_decodeU16(void **user)
{
    uint16_t out;
    bool isNull = true;
    uint16_t expectedOut = 0x00;

    assert_true(BLINK_Compact_decodeU16((blink_stream_t)(*user), &out, &isNull));
    assert_int_equal(expectedOut, out);
    assert_false(isNull);
}

static void test_BLINK_Compact_decodeU16_max(void **user)
{
    static const uint8_t in[] = {0xc2,0xff,0xff};
    struct blink_stream stream;
    blink_stream_t s = BLINK_Stream_initBufferReadOnly(&stream, in, sizeof(in));    
    uint16_t out;
    bool isNull = true;
    uint16_t expectedOut = UINT16_MAX;

    assert_true(BLINK_Compact_decodeU16(s, &out, &isNull));
    assert_int_equal(expectedOut, out);
    assert_false(isNull);
}

static void test_BLINK_Compact_decodeU32(void **user)
{
    uint32_t out;
    bool isNull = true;
    uint32_t expectedOut = 0x00;

    assert_true(BLINK_Compact_decodeU32((blink_stream_t)(*user), &out, &isNull));
    assert_int_equal(expectedOut, out);
    assert_false(isNull);
}

static void test_BLINK_Compact_decodeU32_max(void **user)
{
    static const uint8_t in[] = {0xc4,0xff,0xff,0xff,0xff};
    struct blink_stream stream;
    blink_stream_t s = BLINK_Stream_initBufferReadOnly(&stream, in, sizeof(in));
    uint32_t out;
    bool isNull = true;
    uint32_t expectedOut = UINT32_MAX;

    assert_true(BLINK_Compact_decodeU32(s, &out, &isNull));
    assert_int_equal(expectedOut, out);
    assert_false(isNull);
}

static void test_BLINK_Compact_decodeU64(void **user)
{
    uint64_t out;
    bool isNull = true;
    uint64_t expectedOut = 0x00;

    assert_true(BLINK_Compact_decodeU64((blink_stream_t)(*user), &out, &isNull));
    assert_int_equal(expectedOut, out);
    assert_false(isNull);
}

static void test_BLINK_Compact_decodeU64_max(void **user)
{
    static const uint8_t in[] = {0xc8, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    struct blink_stream stream;
    blink_stream_t s = BLINK_Stream_initBufferReadOnly(&stream, in, sizeof(in));
    uint64_t out;
    bool isNull = true;
    uint64_t expectedOut = UINT64_MAX;

    assert_true(BLINK_Compact_decodeU64(s, &out, &isNull));
    assert_int_equal(expectedOut, out);
    assert_false(isNull);
}

static void test_BLINK_Compact_decodeI8(void **user)
{
    int8_t out;
    bool isNull = true;
    int8_t expectedOut = 0x00;

    assert_true(BLINK_Compact_decodeI8((blink_stream_t)(*user), &out, &isNull));
    assert_int_equal(expectedOut, out);
    assert_false(isNull);
}

static void test_BLINK_Compact_decodeI8_min(void **user)
{
    const uint8_t in[] = {0x80, 0xfe};
    struct blink_stream stream;
    blink_stream_t s = BLINK_Stream_initBufferReadOnly(&stream, in, sizeof(in));
    int8_t out;
    bool isNull = true;
    int8_t expectedOut = INT8_MIN;

    assert_true(BLINK_Compact_decodeI8(s, &out, &isNull));
    assert_int_equal(expectedOut, out);
    assert_false(isNull);
}

static void test_BLINK_Compact_decodeI8_max(void **user)
{
    static const uint8_t in[] = {0xbf,0x01};
    struct blink_stream stream;
    blink_stream_t s = BLINK_Stream_initBufferReadOnly(&stream, in, sizeof(in));
    int8_t out;
    bool isNull = true;
    int8_t expectedOut = INT8_MAX;

    assert_true(BLINK_Compact_decodeI8(s, &out, &isNull));
    assert_int_equal(expectedOut, out);
    assert_false(isNull);
}

static void test_BLINK_Compact_decodeI16(void **user)
{
    int16_t out;
    bool isNull = true;
    int16_t expectedOut = 0x00;

    assert_true(BLINK_Compact_decodeI16((blink_stream_t)(*user), &out, &isNull));
    assert_int_equal(expectedOut, out);
    assert_false(isNull);
}

static void test_BLINK_Compact_decodeI16_min(void **user)
{
    const uint8_t in[] = {0xc2, 0x00, 0x80};
    struct blink_stream stream;
    blink_stream_t s = BLINK_Stream_initBufferReadOnly(&stream, in, sizeof(in));
    int16_t out;
    bool isNull = true;
    int16_t expectedOut = INT16_MIN;

    assert_true(BLINK_Compact_decodeI16(s, &out, &isNull));
    assert_int_equal(expectedOut, out);
    assert_false(isNull);
}
static void test_BLINK_Compact_decodeI16_max(void **user)
{
    const uint8_t in[] = {0xc2, 0xff, 0x7f};
    struct blink_stream stream;
    blink_stream_t s = BLINK_Stream_initBufferReadOnly(&stream, in, sizeof(in));    
    int16_t out;
    bool isNull = true;
    int16_t expectedOut = INT16_MAX;

    assert_true(BLINK_Compact_decodeI16(s, &out, &isNull));
    assert_int_equal(expectedOut, out);
    assert_false(isNull);
}

static void test_BLINK_Compact_decodeI32(void **user)
{
    int32_t out;
    bool isNull = true;
    int32_t expectedOut = 0x00;

    assert_true(BLINK_Compact_decodeI32((blink_stream_t)(*user), &out, &isNull));
    assert_int_equal(expectedOut, out);
    assert_false(isNull);
}    

static void test_BLINK_Compact_decodeI32_min(void **user)
{
    const uint8_t in[] = {0xc4, 0x00, 0x00, 0x00, 0x80};
    struct blink_stream stream;
    blink_stream_t s = BLINK_Stream_initBufferReadOnly(&stream, in, sizeof(in));
    int32_t out;
    bool isNull = true;
    int32_t expectedOut = INT32_MIN;

    assert_true(BLINK_Compact_decodeI32(s, &out, &isNull));
    assert_int_equal(expectedOut, out);
    assert_false(isNull);
}

static void test_BLINK_Compact_decodeI32_max(void **user)
{
    const uint8_t in[] = {0xc4, 0xff,0xff,0xff,0x7f};
    struct blink_stream stream;
    blink_stream_t s = BLINK_Stream_initBufferReadOnly(&stream, in, sizeof(in));
    int32_t out;
    bool isNull = true;
    int32_t expectedOut = INT32_MAX;

    assert_true(BLINK_Compact_decodeI32(s, &out, &isNull));
    assert_int_equal(expectedOut, out);
    assert_false(isNull);
}

static void test_BLINK_Compact_decodeI64(void **user)
{
    int64_t out;
    bool isNull = true;
    int64_t expectedOut = 0x00;

    assert_true(BLINK_Compact_decodeI64((blink_stream_t)(*user), &out, &isNull));
    assert_int_equal(expectedOut, out);
    assert_false(isNull);
}

static void test_BLINK_Compact_decodeI64_min(void **user)
{
    const uint8_t in[] = {0xc8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80};
    struct blink_stream stream;
    blink_stream_t s = BLINK_Stream_initBufferReadOnly(&stream, in, sizeof(in));    
    int64_t out;
    bool isNull = true;
    int64_t expectedOut = INT64_MIN;

    assert_true(BLINK_Compact_decodeI64(s, &out, &isNull));
    assert_int_equal(expectedOut, out);
    assert_false(isNull);
}

static void test_BLINK_Compact_decodeI64_max(void **user)
{
    static const uint8_t in[] = {0xc8, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f};
    struct blink_stream stream;
    blink_stream_t s = BLINK_Stream_initBufferReadOnly(&stream, in, sizeof(in));
    int64_t out;
    bool isNull = true;
    int64_t expectedOut = INT64_MAX;

    assert_true(BLINK_Compact_decodeI64(s, &out, &isNull));
    assert_int_equal(expectedOut, out);
    assert_false(isNull);
}

static void test_BLINK_Compact_decodeF64(void **user)
{
    double out;
    bool isNull = true;
    double expectedOut = 0;

    assert_true(BLINK_Compact_decodeF64((blink_stream_t)(*user), &out, &isNull));
    assert_int_equal(expectedOut, out);
    assert_false(isNull);
}

static void test_BLINK_Compact_decodeBool(void **user)
{
    bool out;
    bool isNull = true;
    bool expectedOut = false;

    assert_true(BLINK_Compact_decodeBool((blink_stream_t)(*user), &out, &isNull));
    assert_int_equal(expectedOut, out);
    assert_false(isNull);
}

static void test_BLINK_Compact_Decimal(void **user)
{
    const uint8_t in[] = {0x00, 0x00};
    struct blink_stream stream;
    blink_stream_t s = BLINK_Stream_initBufferReadOnly(&stream, in, sizeof(in));
    int64_t mantissa;
    int8_t exponent;        
    bool isNull = true;
    int64_t expectedMantissa = 0;
    int8_t expectedExponent = 0;

    assert_true(BLINK_Compact_decodeDecimal(s, &mantissa, &exponent, &isNull));        
    assert_int_equal(expectedMantissa, mantissa);        
    assert_int_equal(expectedExponent, exponent);        
}

static void test_BLINK_Compact_Decimal_nullMantissa(void **user)
{
    const uint8_t in[] = {0x00, 0xc0};
    struct blink_stream stream;
    blink_stream_t s = BLINK_Stream_initBufferReadOnly(&stream, in, sizeof(in));
    int64_t mantissa;
    int8_t exponent;        
    bool isNull = true;
    
    assert_int_equal(0, BLINK_Compact_decodeDecimal(s, &mantissa, &exponent, &isNull));        
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup(test_BLINK_Compact_decodeU8, setupSingleByteZero),
        cmocka_unit_test(test_BLINK_Compact_decodeU8_max),
        cmocka_unit_test_setup(test_BLINK_Compact_decodeU16, setupSingleByteZero),
        cmocka_unit_test(test_BLINK_Compact_decodeU16_max),
        cmocka_unit_test_setup(test_BLINK_Compact_decodeU32, setupSingleByteZero),
        cmocka_unit_test(test_BLINK_Compact_decodeU32_max),
        cmocka_unit_test_setup(test_BLINK_Compact_decodeU64, setupSingleByteZero),
        cmocka_unit_test(test_BLINK_Compact_decodeU64_max),
        cmocka_unit_test_setup(test_BLINK_Compact_decodeI8, setupSingleByteZero),
        cmocka_unit_test(test_BLINK_Compact_decodeI8_min),
        cmocka_unit_test(test_BLINK_Compact_decodeI8_max),
        cmocka_unit_test_setup(test_BLINK_Compact_decodeI16, setupSingleByteZero),
        cmocka_unit_test(test_BLINK_Compact_decodeI16_min),
        cmocka_unit_test(test_BLINK_Compact_decodeI16_max),
        cmocka_unit_test_setup(test_BLINK_Compact_decodeI32, setupSingleByteZero),
        cmocka_unit_test(test_BLINK_Compact_decodeI32_min),
        cmocka_unit_test(test_BLINK_Compact_decodeI32_max),
        cmocka_unit_test_setup(test_BLINK_Compact_decodeI64, setupSingleByteZero),
        cmocka_unit_test(test_BLINK_Compact_decodeI64_min),
        cmocka_unit_test(test_BLINK_Compact_decodeI64_max),
        cmocka_unit_test_setup(test_BLINK_Compact_decodeF64, setupSingleByteZero),
        cmocka_unit_test_setup(test_BLINK_Compact_decodeBool, setupSingleByteZero),
        cmocka_unit_test(test_BLINK_Compact_Decimal),
        cmocka_unit_test(test_BLINK_Compact_Decimal_nullMantissa)
    };
    
    return cmocka_run_group_tests(tests, NULL, NULL);
}
