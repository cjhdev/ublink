/**
 * @example tc_BLINK_Encode.c
 *
 * */

#include "unity.h"
#include "blink_compact_form.h"

void setUp(void)
{    
}

void tearDown(void)
{
}

void test_BLINK_EncodeVLCNull(void)
{
    uint8_t out[1U];
    const uint8_t expectedOut[] = {0xC0};

    TEST_ASSERT_EQUAL(sizeof(expectedOut), BLINK_EncodeVLCNull(out, sizeof(out)));
    TEST_ASSERT_EQUAL_MEMORY(expectedOut, out, sizeof(expectedOut));
}

void test_BLINK_EncodeVLCNull_tooShort(void)
{
    uint8_t out[1U];

    TEST_ASSERT_EQUAL(0U, BLINK_EncodeVLCNull(out, sizeof(out)-1));
}

void test_BLINK_EncodePresent(void)
{
    uint8_t out[1U];
    const uint8_t expectedOut[] = {0x01};

    TEST_ASSERT_EQUAL(sizeof(expectedOut), BLINK_EncodePresent(out, sizeof(out)));
    TEST_ASSERT_EQUAL_MEMORY(expectedOut, out, sizeof(expectedOut));
}

void test_BLINK_EncodePresent_tooShort(void)
{
    uint8_t out[1U];

    TEST_ASSERT_EQUAL(0U, BLINK_EncodePresent(out, sizeof(out)-1));
}

void test_BLINK_EncodeBool_true(void)
{
    bool in = true;
    uint8_t out[1U];
    const uint8_t expectedOut[] = {0x01};

    TEST_ASSERT_EQUAL(sizeof(expectedOut), BLINK_EncodeBool(in, out, sizeof(out)));
    TEST_ASSERT_EQUAL_MEMORY(expectedOut, out, sizeof(expectedOut));
}

void test_BLINK_EncodeBool_false(void)
{
    bool in = false;
    uint8_t out[1U];
    const uint8_t expectedOut[] = {0x00};

    TEST_ASSERT_EQUAL(sizeof(expectedOut), BLINK_EncodeBool(in, out, sizeof(out)));
    TEST_ASSERT_EQUAL_MEMORY(expectedOut, out, sizeof(expectedOut));
}

void test_BLINK_EncodeU8_max(void)
{
    uint8_t in = UINT8_MAX;    
    const uint8_t expectedOut[] = {0xbf,0x03};
    uint8_t out[sizeof(expectedOut)];

    TEST_ASSERT_EQUAL(sizeof(expectedOut), BLINK_EncodeU8(in, out, sizeof(out)));
    TEST_ASSERT_EQUAL_MEMORY(expectedOut, out, sizeof(expectedOut));
}

void test_BLINK_EncodeU8_127(void)
{
    uint8_t in = 127;    
    const uint8_t expectedOut[] = {0x7f};
    uint8_t out[sizeof(expectedOut)];

    TEST_ASSERT_EQUAL(sizeof(expectedOut), BLINK_EncodeU8(in, out, sizeof(out)));
    TEST_ASSERT_EQUAL_MEMORY(expectedOut, out, sizeof(expectedOut));
}

void test_BLINK_EncodeU8_128(void)
{
    uint8_t in = 128;    
    const uint8_t expectedOut[] = {0x80,0x02};
    uint8_t out[sizeof(expectedOut)];

    TEST_ASSERT_EQUAL(sizeof(expectedOut), BLINK_EncodeU8(in, out, sizeof(out)));
    TEST_ASSERT_EQUAL_MEMORY(expectedOut, out, sizeof(expectedOut));
}

void test_BLINK_EncodeU16_max(void)
{
    uint16_t in = UINT16_MAX;    
    const uint8_t expectedOut[] = {0xc2,0xff,0xff};
    uint8_t out[sizeof(expectedOut)];

    TEST_ASSERT_EQUAL(sizeof(expectedOut), BLINK_EncodeU16(in, out, sizeof(out)));
    TEST_ASSERT_EQUAL_MEMORY(expectedOut, out, sizeof(expectedOut));
}

void test_BLINK_EncodeU32_max(void)
{
    uint32_t in = UINT32_MAX;    
    const uint8_t expectedOut[] = {0xc4,0xff,0xff,0xff,0xff};
    uint8_t out[sizeof(expectedOut)];

    TEST_ASSERT_EQUAL(sizeof(expectedOut), BLINK_EncodeU32(in, out, sizeof(out)));
    TEST_ASSERT_EQUAL_MEMORY(expectedOut, out, sizeof(expectedOut));
}

void test_BLINK_EncodeU64_max(void)
{
    uint64_t in = UINT64_MAX;    
    const uint8_t expectedOut[] = {0xc8,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};
    uint8_t out[sizeof(expectedOut)];

    TEST_ASSERT_EQUAL(sizeof(expectedOut), BLINK_EncodeU64(in, out, sizeof(out)));
    TEST_ASSERT_EQUAL_MEMORY(expectedOut, out, sizeof(expectedOut));
}

void test_BLINK_EncodeI8_63(void)
{
    int8_t in = 63;    
    const uint8_t expectedOut[] = {0x3f};
    uint8_t out[sizeof(expectedOut)];

    TEST_ASSERT_EQUAL(sizeof(expectedOut), BLINK_EncodeI8(in, out, sizeof(out)));
    TEST_ASSERT_EQUAL_MEMORY(expectedOut, out, sizeof(expectedOut));
}

void test_BLINK_EncodeI8_64(void)
{
    int8_t in = 64;    
    const uint8_t expectedOut[] = {0x80, 0x01};
    uint8_t out[sizeof(expectedOut)];

    TEST_ASSERT_EQUAL(sizeof(expectedOut), BLINK_EncodeI8(in, out, sizeof(out)));
    TEST_ASSERT_EQUAL_MEMORY(expectedOut, out, sizeof(expectedOut));
}

void test_BLINK_EncodeI8_minus64(void)
{
    int8_t in = -64;    
    const uint8_t expectedOut[] = {0x40};
    uint8_t out[sizeof(expectedOut)];

    TEST_ASSERT_EQUAL(sizeof(expectedOut), BLINK_EncodeI8(in, out, sizeof(out)));
    TEST_ASSERT_EQUAL_MEMORY(expectedOut, out, sizeof(expectedOut));
}

void test_BLINK_EncodeI8_minus65(void)
{
    int8_t in = -65;    
    const uint8_t expectedOut[] = {0xbf, 0xfe};
    uint8_t out[sizeof(expectedOut)];

    TEST_ASSERT_EQUAL(sizeof(expectedOut), BLINK_EncodeI8(in, out, sizeof(out)));
    TEST_ASSERT_EQUAL_MEMORY(expectedOut, out, sizeof(expectedOut));
}

void test_BLINK_EncodeI8_min(void)
{
    int8_t in = INT8_MIN;    
    const uint8_t expectedOut[] = {0x80,0xfe};
    uint8_t out[sizeof(expectedOut)];

    TEST_ASSERT_EQUAL(sizeof(expectedOut), BLINK_EncodeI8(in, out, sizeof(out)));
    TEST_ASSERT_EQUAL_MEMORY(expectedOut, out, sizeof(expectedOut));
}

void test_BLINK_EncodeI16_min(void)
{
    int16_t in = INT16_MIN;    
    const uint8_t expectedOut[] = {0xc2,0x00,0x80};
    uint8_t out[sizeof(expectedOut)];

    TEST_ASSERT_EQUAL(sizeof(expectedOut), BLINK_EncodeI16(in, out, sizeof(out)));
    TEST_ASSERT_EQUAL_MEMORY(expectedOut, out, sizeof(expectedOut));
}

void test_BLINK_EncodeI32_min(void)
{
    int32_t in = INT32_MIN;    
    const uint8_t expectedOut[] = {0xc4,0x00,0x00,0x00,0x80};
    uint8_t out[sizeof(expectedOut)];

    TEST_ASSERT_EQUAL(sizeof(expectedOut), BLINK_EncodeI32(in, out, sizeof(out)));
    TEST_ASSERT_EQUAL_MEMORY(expectedOut, out, sizeof(expectedOut));
}

void test_BLINK_EncodeI64_min(void)
{
    int64_t in = INT64_MIN;    
    const uint8_t expectedOut[] = {0xc8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80};
    uint8_t out[sizeof(expectedOut)];

    TEST_ASSERT_EQUAL(sizeof(expectedOut), BLINK_EncodeI64(in, out, sizeof(out)));
    TEST_ASSERT_EQUAL_MEMORY(expectedOut, out, sizeof(expectedOut));
}

void test_BLINK_EncodeI8_max(void)
{
    int8_t in = INT8_MAX;    
    const uint8_t expectedOut[] = {0xbf,0x01};
    uint8_t out[sizeof(expectedOut)];

    TEST_ASSERT_EQUAL(sizeof(expectedOut), BLINK_EncodeI8(in, out, sizeof(out)));
    TEST_ASSERT_EQUAL_MEMORY(expectedOut, out, sizeof(expectedOut));
}

void test_BLINK_EncodeI16_max(void)
{
    int16_t in = INT16_MAX;    
    const uint8_t expectedOut[] = {0xc2,0xff,0x7f};
    uint8_t out[sizeof(expectedOut)];

    TEST_ASSERT_EQUAL(sizeof(expectedOut), BLINK_EncodeI16(in, out, sizeof(out)));
    TEST_ASSERT_EQUAL_MEMORY(expectedOut, out, sizeof(expectedOut));
}

void test_BLINK_EncodeI32_max(void)
{
    int32_t in = INT32_MAX;    
    const uint8_t expectedOut[] = {0xc4,0xff,0xff,0xff,0x7f};
    uint8_t out[sizeof(expectedOut)];

    TEST_ASSERT_EQUAL(sizeof(expectedOut), BLINK_EncodeI32(in, out, sizeof(out)));
    TEST_ASSERT_EQUAL_MEMORY(expectedOut, out, sizeof(expectedOut));
}

void test_BLINK_EncodeI64_max(void)
{
    int64_t in = INT64_MAX;    
    const uint8_t expectedOut[] = {0xc8,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x7f};
    uint8_t out[sizeof(expectedOut)];

    TEST_ASSERT_EQUAL(sizeof(expectedOut), BLINK_EncodeI64(in, out, sizeof(out)));
    TEST_ASSERT_EQUAL_MEMORY(expectedOut, out, sizeof(expectedOut));
}

void test_BLINK_EncodeF64(void)
{
    double in = 0;    
    const uint8_t expectedOut[] = {0x00};
    uint8_t out[sizeof(expectedOut)];

    TEST_ASSERT_EQUAL(sizeof(expectedOut), BLINK_EncodeF64(in, out, sizeof(out)));
    TEST_ASSERT_EQUAL_MEMORY(expectedOut, out, sizeof(expectedOut));
}

void test_BLINK_EncodeBinary(void)
{
    const uint8_t in[] = {'h','e','l','l','o'};
    const uint8_t expectedOut[] = {0x05, 'h','e','l','l','o'};
    uint8_t out[sizeof(expectedOut)];

    TEST_ASSERT_EQUAL(sizeof(expectedOut), BLINK_EncodeBinary(in, sizeof(in), out, sizeof(out)));
    TEST_ASSERT_EQUAL_MEMORY(expectedOut, out, sizeof(expectedOut));
}

void test_BLINK_EncodeBinary_tooShort(void)
{
    const uint8_t in[] = {'h','e','l','l','o'};
    const uint8_t expectedOut[] = {0x05, 'h','e','l','l','o'};
    uint8_t out[sizeof(expectedOut)];

    TEST_ASSERT_EQUAL(0U, BLINK_EncodeBinary(in, sizeof(in), out, sizeof(out)-1));
}

void test_BLINK_EncodeString(void)
{
    const uint8_t in[] = {'h','e','l','l','o'};
    const uint8_t expectedOut[] = {0x05, 'h','e','l','l','o'};
    uint8_t out[sizeof(expectedOut)];

    TEST_ASSERT_EQUAL(sizeof(expectedOut), BLINK_EncodeString(in, sizeof(in), out, sizeof(out)));
    TEST_ASSERT_EQUAL_MEMORY(expectedOut, out, sizeof(expectedOut));
}

void test_BLINK_EncodeFixed(void)
{
    const uint8_t in[] = {'h','e','l','l','o'};
    const uint8_t expectedOut[] = {'h','e','l','l','o'};
    uint8_t out[sizeof(expectedOut)];

    TEST_ASSERT_EQUAL(sizeof(expectedOut), BLINK_EncodeFixed(in, sizeof(in), out, sizeof(out)));
    TEST_ASSERT_EQUAL_MEMORY(expectedOut, out, sizeof(expectedOut));
}

void test_BLINK_EncodeFixed_tooShort(void)
{
    const uint8_t in[] = {'h','e','l','l','o'};
    const uint8_t expectedOut[] = {'h','e','l','l','o'};
    uint8_t out[sizeof(expectedOut)];

    TEST_ASSERT_EQUAL(0U, BLINK_EncodeFixed(in, sizeof(in), out, sizeof(out)-1));
}

void test_BLINK_EncodeOptionalFixed(void)
{
    const uint8_t in[] = {'h','e','l','l','o'};
    const uint8_t expectedOut[] = {0x01, 'h','e','l','l','o'};
    uint8_t out[sizeof(expectedOut)];

    TEST_ASSERT_EQUAL(sizeof(expectedOut), BLINK_EncodeOptionalFixed(in, sizeof(in), out, sizeof(out)));
    TEST_ASSERT_EQUAL_MEMORY(expectedOut, out, sizeof(expectedOut));
}

void test_BLINK_EncodeOptionalFixed_tooShort(void)
{
    const uint8_t in[] = {'h','e','l','l','o'};
    const uint8_t expectedOut[] = {0x01, 'h','e','l','l','o'};
    uint8_t out[sizeof(expectedOut)];

    TEST_ASSERT_EQUAL(0U, BLINK_EncodeOptionalFixed(in, sizeof(in), out, sizeof(out)-1));
}

void test_BLINK_EncodeDecimal(void)
{
    int64_t mantissa = 0;
    int8_t exponent = 0;
    
    const uint8_t expectedOut[] = {0x00, 0x00};
    uint8_t out[sizeof(expectedOut)];

    TEST_ASSERT_EQUAL(sizeof(expectedOut), BLINK_EncodeDecimal(mantissa, exponent, out, sizeof(out)));
    TEST_ASSERT_EQUAL_MEMORY(expectedOut, out, sizeof(expectedOut));
}

void test_BLINK_EncodeDecimal_tooShort(void)
{
    int64_t mantissa = 0;
    int8_t exponent = 0;
    
    const uint8_t expectedOut[] = {0x00, 0x00};
    uint8_t out[sizeof(expectedOut)];

    TEST_ASSERT_EQUAL(0U, BLINK_EncodeDecimal(mantissa, exponent, out, sizeof(out)-1));
}

void test_BLINK_EncodeDecimal_wayTooShort(void)
{
    int64_t mantissa = 0;
    int8_t exponent = 0;
    
    const uint8_t expectedOut[] = {0x00, 0x00};
    uint8_t out[sizeof(expectedOut)];

    TEST_ASSERT_EQUAL(0U, BLINK_EncodeDecimal(mantissa, exponent, out, sizeof(out)-2));
}

