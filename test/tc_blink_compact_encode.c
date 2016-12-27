/**
 * @example __FILE__
 *
 * */

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>

#include "cmocka.h"
#include "blink_compact.h"

void test_BLINK_Compact_encodeVLCNull(void **user)
{
    uint8_t out[1U];
    const uint8_t expectedOut[] = {0xC0};

    assert_int_equal(sizeof(expectedOut), BLINK_Compact_encodeVLCNull(out, sizeof(out)));
    assert_memory_equal(expectedOut, out, sizeof(expectedOut));
}

void test_BLINK_Compact_encodeVLCNull_tooShort(void **user)
{
    uint8_t out[1U];

    assert_int_equal(0U, BLINK_Compact_encodeVLCNull(out, sizeof(out)-1));
}

void test_BLINK_Compact_encodePresent(void **user)
{
    uint8_t out[1U];
    const uint8_t expectedOut[] = {0x01};

    assert_int_equal(sizeof(expectedOut), BLINK_Compact_encodePresent(out, sizeof(out)));
    assert_memory_equal(expectedOut, out, sizeof(expectedOut));
}

void test_BLINK_Compact_encodePresent_tooShort(void **user)
{
    uint8_t out[1U];

    assert_int_equal(0U, BLINK_Compact_encodePresent(out, sizeof(out)-1));
}

void test_BLINK_Compact_encodeBool_true(void **user)
{
    bool in = true;
    uint8_t out[1U];
    const uint8_t expectedOut[] = {0x01};

    assert_int_equal(sizeof(expectedOut), BLINK_Compact_encodeBool(in, out, sizeof(out)));
    assert_memory_equal(expectedOut, out, sizeof(expectedOut));
}

void test_BLINK_Compact_encodeBool_false(void **user)
{
    bool in = false;
    uint8_t out[1U];
    const uint8_t expectedOut[] = {0x00};

    assert_int_equal(sizeof(expectedOut), BLINK_Compact_encodeBool(in, out, sizeof(out)));
    assert_memory_equal(expectedOut, out, sizeof(expectedOut));
}

void test_BLINK_Compact_encodeU8_max(void **user)
{
    uint8_t in = UINT8_MAX;    
    const uint8_t expectedOut[] = {0xbf,0x03};
    uint8_t out[sizeof(expectedOut)];

    assert_int_equal(sizeof(expectedOut), BLINK_Compact_encodeU8(in, out, sizeof(out)));
    assert_memory_equal(expectedOut, out, sizeof(expectedOut));
}

void test_BLINK_Compact_encodeU8_127(void **user)
{
    uint8_t in = 127;    
    const uint8_t expectedOut[] = {0x7f};
    uint8_t out[sizeof(expectedOut)];

    assert_int_equal(sizeof(expectedOut), BLINK_Compact_encodeU8(in, out, sizeof(out)));
    assert_memory_equal(expectedOut, out, sizeof(expectedOut));
}

void test_BLINK_Compact_encodeU8_128(void **user)
{
    uint8_t in = 128;    
    const uint8_t expectedOut[] = {0x80,0x02};
    uint8_t out[sizeof(expectedOut)];

    assert_int_equal(sizeof(expectedOut), BLINK_Compact_encodeU8(in, out, sizeof(out)));
    assert_memory_equal(expectedOut, out, sizeof(expectedOut));
}

void test_BLINK_Compact_encodeU16_max(void **user)
{
    uint16_t in = UINT16_MAX;    
    const uint8_t expectedOut[] = {0xc2,0xff,0xff};
    uint8_t out[sizeof(expectedOut)];

    assert_int_equal(sizeof(expectedOut), BLINK_Compact_encodeU16(in, out, sizeof(out)));
    assert_memory_equal(expectedOut, out, sizeof(expectedOut));
}

void test_BLINK_Compact_encodeU32_max(void **user)
{
    uint32_t in = UINT32_MAX;    
    const uint8_t expectedOut[] = {0xc4,0xff,0xff,0xff,0xff};
    uint8_t out[sizeof(expectedOut)];

    assert_int_equal(sizeof(expectedOut), BLINK_Compact_encodeU32(in, out, sizeof(out)));
    assert_memory_equal(expectedOut, out, sizeof(expectedOut));
}

void test_BLINK_Compact_encodeU64_max(void **user)
{
    uint64_t in = UINT64_MAX;    
    const uint8_t expectedOut[] = {0xc8,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};
    uint8_t out[sizeof(expectedOut)];

    assert_int_equal(sizeof(expectedOut), BLINK_Compact_encodeU64(in, out, sizeof(out)));
    assert_memory_equal(expectedOut, out, sizeof(expectedOut));
}

void test_BLINK_Compact_encodeI8_63(void **user)
{
    int8_t in = 63;    
    const uint8_t expectedOut[] = {0x3f};
    uint8_t out[sizeof(expectedOut)];

    assert_int_equal(sizeof(expectedOut), BLINK_Compact_encodeI8(in, out, sizeof(out)));
    assert_memory_equal(expectedOut, out, sizeof(expectedOut));
}

void test_BLINK_Compact_encodeI8_64(void **user)
{
    int8_t in = 64;    
    const uint8_t expectedOut[] = {0x80, 0x01};
    uint8_t out[sizeof(expectedOut)];

    assert_int_equal(sizeof(expectedOut), BLINK_Compact_encodeI8(in, out, sizeof(out)));
    assert_memory_equal(expectedOut, out, sizeof(expectedOut));
}

void test_BLINK_Compact_encodeI8_minus64(void **user)
{
    int8_t in = -64;    
    const uint8_t expectedOut[] = {0x40};
    uint8_t out[sizeof(expectedOut)];

    assert_int_equal(sizeof(expectedOut), BLINK_Compact_encodeI8(in, out, sizeof(out)));
    assert_memory_equal(expectedOut, out, sizeof(expectedOut));
}

void test_BLINK_Compact_encodeI8_minus65(void **user)
{
    int8_t in = -65;    
    const uint8_t expectedOut[] = {0xbf, 0xfe};
    uint8_t out[sizeof(expectedOut)];

    assert_int_equal(sizeof(expectedOut), BLINK_Compact_encodeI8(in, out, sizeof(out)));
    assert_memory_equal(expectedOut, out, sizeof(expectedOut));
}

void test_BLINK_Compact_encodeI8_min(void **user)
{
    int8_t in = INT8_MIN;    
    const uint8_t expectedOut[] = {0x80,0xfe};
    uint8_t out[sizeof(expectedOut)];

    assert_int_equal(sizeof(expectedOut), BLINK_Compact_encodeI8(in, out, sizeof(out)));
    assert_memory_equal(expectedOut, out, sizeof(expectedOut));
}

void test_BLINK_Compact_encodeI16_min(void **user)
{
    int16_t in = INT16_MIN;    
    const uint8_t expectedOut[] = {0xc2,0x00,0x80};
    uint8_t out[sizeof(expectedOut)];

    assert_int_equal(sizeof(expectedOut), BLINK_Compact_encodeI16(in, out, sizeof(out)));
    assert_memory_equal(expectedOut, out, sizeof(expectedOut));
}

void test_BLINK_Compact_encodeI32_min(void **user)
{
    int32_t in = INT32_MIN;    
    const uint8_t expectedOut[] = {0xc4,0x00,0x00,0x00,0x80};
    uint8_t out[sizeof(expectedOut)];

    assert_int_equal(sizeof(expectedOut), BLINK_Compact_encodeI32(in, out, sizeof(out)));
    assert_memory_equal(expectedOut, out, sizeof(expectedOut));
}

void test_BLINK_Compact_encodeI64_min(void **user)
{
    int64_t in = INT64_MIN;    
    const uint8_t expectedOut[] = {0xc8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80};
    uint8_t out[sizeof(expectedOut)];

    assert_int_equal(sizeof(expectedOut), BLINK_Compact_encodeI64(in, out, sizeof(out)));
    assert_memory_equal(expectedOut, out, sizeof(expectedOut));
}

void test_BLINK_Compact_encodeI8_max(void **user)
{
    int8_t in = INT8_MAX;    
    const uint8_t expectedOut[] = {0xbf,0x01};
    uint8_t out[sizeof(expectedOut)];

    assert_int_equal(sizeof(expectedOut), BLINK_Compact_encodeI8(in, out, sizeof(out)));
    assert_memory_equal(expectedOut, out, sizeof(expectedOut));
}

void test_BLINK_Compact_encodeI16_max(void **user)
{
    int16_t in = INT16_MAX;    
    const uint8_t expectedOut[] = {0xc2,0xff,0x7f};
    uint8_t out[sizeof(expectedOut)];

    assert_int_equal(sizeof(expectedOut), BLINK_Compact_encodeI16(in, out, sizeof(out)));
    assert_memory_equal(expectedOut, out, sizeof(expectedOut));
}

void test_BLINK_Compact_encodeI32_max(void **user)
{
    int32_t in = INT32_MAX;    
    const uint8_t expectedOut[] = {0xc4,0xff,0xff,0xff,0x7f};
    uint8_t out[sizeof(expectedOut)];

    assert_int_equal(sizeof(expectedOut), BLINK_Compact_encodeI32(in, out, sizeof(out)));
    assert_memory_equal(expectedOut, out, sizeof(expectedOut));
}

void test_BLINK_Compact_encodeI64_max(void **user)
{
    int64_t in = INT64_MAX;    
    const uint8_t expectedOut[] = {0xc8,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x7f};
    uint8_t out[sizeof(expectedOut)];

    assert_int_equal(sizeof(expectedOut), BLINK_Compact_encodeI64(in, out, sizeof(out)));
    assert_memory_equal(expectedOut, out, sizeof(expectedOut));
}

void test_BLINK_Compact_encodeF64(void **user)
{
    double in = 0;    
    const uint8_t expectedOut[] = {0x00};
    uint8_t out[sizeof(expectedOut)];

    assert_int_equal(sizeof(expectedOut), BLINK_Compact_encodeF64(in, out, sizeof(out)));
    assert_memory_equal(expectedOut, out, sizeof(expectedOut));
}

void test_BLINK_Compact_encodeBinary(void **user)
{
    const uint8_t in[] = {'h','e','l','l','o'};
    const uint8_t expectedOut[] = {0x05, 'h','e','l','l','o'};
    uint8_t out[sizeof(expectedOut)];

    assert_int_equal(sizeof(expectedOut), BLINK_Compact_encodeBinary(in, sizeof(in), out, sizeof(out)));
    assert_memory_equal(expectedOut, out, sizeof(expectedOut));
}

void test_BLINK_Compact_encodeBinary_tooShort(void **user)
{
    const uint8_t in[] = {'h','e','l','l','o'};
    const uint8_t expectedOut[] = {0x05, 'h','e','l','l','o'};
    uint8_t out[sizeof(expectedOut)];

    assert_int_equal(0U, BLINK_Compact_encodeBinary(in, sizeof(in), out, sizeof(out)-1));
}

void test_BLINK_Compact_encodeString(void **user)
{
    const uint8_t in[] = {'h','e','l','l','o'};
    const uint8_t expectedOut[] = {0x05, 'h','e','l','l','o'};
    uint8_t out[sizeof(expectedOut)];

    assert_int_equal(sizeof(expectedOut), BLINK_Compact_encodeString(in, sizeof(in), out, sizeof(out)));
    assert_memory_equal(expectedOut, out, sizeof(expectedOut));
}

void test_BLINK_Compact_encodeFixed(void **user)
{
    const uint8_t in[] = {'h','e','l','l','o'};
    const uint8_t expectedOut[] = {'h','e','l','l','o'};
    uint8_t out[sizeof(expectedOut)];

    assert_int_equal(sizeof(expectedOut), BLINK_Compact_encodeFixed(in, sizeof(in), out, sizeof(out)));
    assert_memory_equal(expectedOut, out, sizeof(expectedOut));
}

void test_BLINK_Compact_encodeFixed_tooShort(void **user)
{
    const uint8_t in[] = {'h','e','l','l','o'};
    const uint8_t expectedOut[] = {'h','e','l','l','o'};
    uint8_t out[sizeof(expectedOut)];

    assert_int_equal(0U, BLINK_Compact_encodeFixed(in, sizeof(in), out, sizeof(out)-1));
}

void test_BLINK_Compact_encodeOptionalFixed(void **user)
{
    const uint8_t in[] = {'h','e','l','l','o'};
    const uint8_t expectedOut[] = {0x01, 'h','e','l','l','o'};
    uint8_t out[sizeof(expectedOut)];

    assert_int_equal(sizeof(expectedOut), BLINK_Compact_encodeOptionalFixed(in, sizeof(in), out, sizeof(out)));
    assert_memory_equal(expectedOut, out, sizeof(expectedOut));
}

void test_BLINK_Compact_encodeOptionalFixed_tooShort(void **user)
{
    const uint8_t in[] = {'h','e','l','l','o'};
    const uint8_t expectedOut[] = {0x01, 'h','e','l','l','o'};
    uint8_t out[sizeof(expectedOut)];

    assert_int_equal(0U, BLINK_Compact_encodeOptionalFixed(in, sizeof(in), out, sizeof(out)-1));
}

void test_BLINK_Compact_encodeDecimal(void **user)
{
    int64_t mantissa = 0;
    int8_t exponent = 0;
    
    const uint8_t expectedOut[] = {0x00, 0x00};
    uint8_t out[sizeof(expectedOut)];

    assert_int_equal(sizeof(expectedOut), BLINK_Compact_encodeDecimal(mantissa, exponent, out, sizeof(out)));
    assert_memory_equal(expectedOut, out, sizeof(expectedOut));
}

void test_BLINK_Compact_encodeDecimal_tooShort(void **user)
{
    int64_t mantissa = 0;
    int8_t exponent = 0;
    
    const uint8_t expectedOut[] = {0x00, 0x00};
    uint8_t out[sizeof(expectedOut)];

    assert_int_equal(0U, BLINK_Compact_encodeDecimal(mantissa, exponent, out, sizeof(out)-1));
}

void test_BLINK_Compact_encodeDecimal_wayTooShort(void **user)
{
    int64_t mantissa = 0;
    int8_t exponent = 0;
    
    const uint8_t expectedOut[] = {0x00, 0x00};
    uint8_t out[sizeof(expectedOut)];

    assert_int_equal(0U, BLINK_Compact_encodeDecimal(mantissa, exponent, out, sizeof(out)-2));
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_BLINK_Compact_encodeVLCNull),
        cmocka_unit_test(test_BLINK_Compact_encodeVLCNull_tooShort),
        cmocka_unit_test(test_BLINK_Compact_encodePresent),
        cmocka_unit_test(test_BLINK_Compact_encodePresent_tooShort),
        cmocka_unit_test(test_BLINK_Compact_encodeBool_true),
        cmocka_unit_test(test_BLINK_Compact_encodeBool_false),
        cmocka_unit_test(test_BLINK_Compact_encodeU8_max),
        cmocka_unit_test(test_BLINK_Compact_encodeU8_127),
        cmocka_unit_test(test_BLINK_Compact_encodeU8_128),
        cmocka_unit_test(test_BLINK_Compact_encodeU16_max),
        cmocka_unit_test(test_BLINK_Compact_encodeU32_max),
        cmocka_unit_test(test_BLINK_Compact_encodeU64_max),
        cmocka_unit_test(test_BLINK_Compact_encodeI8_63),
        cmocka_unit_test(test_BLINK_Compact_encodeI8_64),
        cmocka_unit_test(test_BLINK_Compact_encodeI8_minus64),
        cmocka_unit_test(test_BLINK_Compact_encodeI8_minus65),
        cmocka_unit_test(test_BLINK_Compact_encodeI8_min),
        cmocka_unit_test(test_BLINK_Compact_encodeI16_min),
        cmocka_unit_test(test_BLINK_Compact_encodeI32_min),
        cmocka_unit_test(test_BLINK_Compact_encodeI64_min),
        cmocka_unit_test(test_BLINK_Compact_encodeI8_max),
        cmocka_unit_test(test_BLINK_Compact_encodeI16_max),
        cmocka_unit_test(test_BLINK_Compact_encodeI32_max),
        cmocka_unit_test(test_BLINK_Compact_encodeI64_max),
        cmocka_unit_test(test_BLINK_Compact_encodeF64),
        cmocka_unit_test(test_BLINK_Compact_encodeBinary),
        cmocka_unit_test(test_BLINK_Compact_encodeBinary_tooShort),
        cmocka_unit_test(test_BLINK_Compact_encodeString),
        cmocka_unit_test(test_BLINK_Compact_encodeFixed),
        cmocka_unit_test(test_BLINK_Compact_encodeFixed_tooShort),
        cmocka_unit_test(test_BLINK_Compact_encodeOptionalFixed),
        cmocka_unit_test(test_BLINK_Compact_encodeOptionalFixed_tooShort),
        cmocka_unit_test(test_BLINK_Compact_encodeDecimal),
        cmocka_unit_test(test_BLINK_Compact_encodeDecimal_tooShort),
        cmocka_unit_test(test_BLINK_Compact_encodeDecimal_wayTooShort)
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
