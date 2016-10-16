/**
 * @example tc_BLINK_Decode.c
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

void test_BLINK_DecodeU8(void)
{
    const uint8_t in[] = {0x00};
    uint8_t out;
    bool isNull = true;
    uint8_t expectedOut = 0x00;

    TEST_ASSERT_EQUAL(sizeof(in), BLINK_DecodeU8(in, sizeof(in), &out, &isNull));
    TEST_ASSERT_EQUAL(expectedOut, out);
    TEST_ASSERT_FALSE(isNull);
}

void test_BLINK_DecodeU8_max(void)
{
    const uint8_t in[] = {0x00};
    uint8_t out;
    bool isNull = true;
    uint8_t expectedOut = UINT8_MAX;

    TEST_ASSERT_EQUAL(sizeof(in), BLINK_DecodeU8(in, sizeof(in), &out, &isNull));
    TEST_ASSERT_EQUAL(expectedOut, out);
    TEST_ASSERT_FALSE(isNull);
}

void test_BLINK_DecodeU16(void)
{
    const uint8_t in[] = {0x00};
    uint16_t out;
    bool isNull = true;
    uint16_t expectedOut = 0x00;

    TEST_ASSERT_EQUAL(sizeof(in), BLINK_DecodeU16(in, sizeof(in), &out, &isNull));
    TEST_ASSERT_EQUAL(expectedOut, out);
    TEST_ASSERT_FALSE(isNull);
}

void test_BLINK_DecodeU16_max(void)
{
    const uint8_t in[] = {0xc2,0xff,0xff};
    uint16_t out;
    bool isNull = true;
    uint16_t expectedOut = UINT16_MAX;

    TEST_ASSERT_EQUAL(sizeof(in), BLINK_DecodeU16(in, sizeof(in), &out, &isNull));
    TEST_ASSERT_EQUAL(expectedOut, out);
    TEST_ASSERT_FALSE(isNull);
}

void test_BLINK_DecodeU32(void)
{
    const uint8_t in[] = {0x00};
    uint32_t out;
    bool isNull = true;
    uint32_t expectedOut = 0x00;

    TEST_ASSERT_EQUAL(sizeof(in), BLINK_DecodeU32(in, sizeof(in), &out, &isNull));
    TEST_ASSERT_EQUAL(expectedOut, out);
    TEST_ASSERT_FALSE(isNull);
}

void test_BLINK_DecodeU32_max(void)
{
    const uint8_t in[] = {0xc4,0xff,0xff,0xff,0xff};
    uint32_t out;
    bool isNull = true;
    uint32_t expectedOut = UINT32_MAX;

    TEST_ASSERT_EQUAL(sizeof(in), BLINK_DecodeU32(in, sizeof(in), &out, &isNull));
    TEST_ASSERT_EQUAL(expectedOut, out);
    TEST_ASSERT_FALSE(isNull);
}

void test_BLINK_DecodeU64(void)
{
    const uint8_t in[] = {0x00};
    uint64_t out;
    bool isNull = true;
    uint64_t expectedOut = 0x00;

    TEST_ASSERT_EQUAL(sizeof(in), BLINK_DecodeU64(in, sizeof(in), &out, &isNull));
    TEST_ASSERT_EQUAL(expectedOut, out);
    TEST_ASSERT_FALSE(isNull);
}

void test_BLINK_DecodeU64_max(void)
{
    const uint8_t in[] = {0xc8, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    uint64_t out;
    bool isNull = true;
    uint64_t expectedOut = UINT64_MAX;

    TEST_ASSERT_EQUAL(sizeof(in), BLINK_DecodeU64(in, sizeof(in), &out, &isNull));
    TEST_ASSERT_EQUAL(expectedOut, out);
    TEST_ASSERT_FALSE(isNull);
}

void test_BLINK_DecodeI8(void)
{
    const uint8_t in[] = {0x00};
    int8_t out;
    bool isNull = true;
    int8_t expectedOut = 0x00;

    TEST_ASSERT_EQUAL(sizeof(in), BLINK_DecodeI8(in, sizeof(in), &out, &isNull));
    TEST_ASSERT_EQUAL(expectedOut, out);
    TEST_ASSERT_FALSE(isNull);
}

void test_BLINK_DecodeI8_min(void)
{
    const uint8_t in[] = {0x00};
    int8_t out;
    bool isNull = true;
    int8_t expectedOut = INT8_MIN;

    TEST_ASSERT_EQUAL(sizeof(in), BLINK_DecodeI8(in, sizeof(in), &out, &isNull));
    TEST_ASSERT_EQUAL(expectedOut, out);
    TEST_ASSERT_FALSE(isNull);
}

void test_BLINK_DecodeI8_max(void)
{
    const uint8_t in[] = {0x00};
    int8_t out;
    bool isNull = true;
    int8_t expectedOut = INT8_MAX;

    TEST_ASSERT_EQUAL(sizeof(in), BLINK_DecodeI8(in, sizeof(in), &out, &isNull));
    TEST_ASSERT_EQUAL(expectedOut, out);
    TEST_ASSERT_FALSE(isNull);
}

void test_BLINK_DecodeI16(void)
{
    const uint8_t in[] = {0x00};
    int16_t out;
    bool isNull = true;
    int16_t expectedOut = 0x00;

    TEST_ASSERT_EQUAL(sizeof(in), BLINK_DecodeI16(in, sizeof(in), &out, &isNull));
    TEST_ASSERT_EQUAL(expectedOut, out);
    TEST_ASSERT_FALSE(isNull);
}

void test_BLINK_DecodeI16_min(void)
{
    const uint8_t in[] = {0xc2, 0x00, 0x80};
    int16_t out;
    bool isNull = true;
    int16_t expectedOut = INT16_MIN;

    TEST_ASSERT_EQUAL(sizeof(in), BLINK_DecodeI16(in, sizeof(in), &out, &isNull));
    TEST_ASSERT_EQUAL(expectedOut, out);
    TEST_ASSERT_FALSE(isNull);
}
void test_BLINK_DecodeI16_max(void)
{
    const uint8_t in[] = {0xc3, 0xff, 0x7f};
    int16_t out;
    bool isNull = true;
    int16_t expectedOut = INT16_MAX;

    TEST_ASSERT_EQUAL(sizeof(in), BLINK_DecodeI16(in, sizeof(in), &out, &isNull));
    TEST_ASSERT_EQUAL(expectedOut, out);
    TEST_ASSERT_FALSE(isNull);
}

void test_BLINK_DecodeI32(void)
{
    const uint8_t in[] = {0x00};
    int32_t out;
    bool isNull = true;
    int32_t expectedOut = 0x00;

    TEST_ASSERT_EQUAL(sizeof(in), BLINK_DecodeI32(in, sizeof(in), &out, &isNull));
    TEST_ASSERT_EQUAL(expectedOut, out);
    TEST_ASSERT_FALSE(isNull);
}    

void test_BLINK_DecodeI32_min(void)
{
    const uint8_t in[] = {0xc4, 0x00, 0x00, 0x00, 0x00};
    int32_t out;
    bool isNull = true;
    int32_t expectedOut = INT32_MIN;

    TEST_ASSERT_EQUAL(sizeof(in), BLINK_DecodeI32(in, sizeof(in), &out, &isNull));
    TEST_ASSERT_EQUAL(expectedOut, out);
    TEST_ASSERT_FALSE(isNull);
}

void test_BLINK_DecodeI32_max(void)
{
    const uint8_t in[] = {0xc4, 0xff,0xff,0xff,0xff};
    int32_t out;
    bool isNull = true;
    int32_t expectedOut = INT32_MAX;

    TEST_ASSERT_EQUAL(sizeof(in), BLINK_DecodeI32(in, sizeof(in), &out, &isNull));
    TEST_ASSERT_EQUAL(expectedOut, out);
    TEST_ASSERT_FALSE(isNull);
}

void test_BLINK_DecodeI64(void)
{
    const uint8_t in[] = {0x00};
    int64_t out;
    bool isNull = true;
    int64_t expectedOut = 0x00;

    TEST_ASSERT_EQUAL(sizeof(in), BLINK_DecodeI64(in, sizeof(in), &out, &isNull));
    TEST_ASSERT_EQUAL(expectedOut, out);
    TEST_ASSERT_FALSE(isNull);
}

void test_BLINK_DecodeI64_min(void)
{
    const uint8_t in[] = {0xc8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    int64_t out;
    bool isNull = true;
    int64_t expectedOut = INT64_MIN;

    TEST_ASSERT_EQUAL(sizeof(in), BLINK_DecodeI64(in, sizeof(in), &out, &isNull));
    TEST_ASSERT_EQUAL(expectedOut, out);
    TEST_ASSERT_FALSE(isNull);
}

void test_BLINK_DecodeI64_max(void)
{
    const uint8_t in[] = {0xc8, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    int64_t out;
    bool isNull = true;
    int64_t expectedOut = INT64_MAX;

    TEST_ASSERT_EQUAL(sizeof(in), BLINK_DecodeI64(in, sizeof(in), &out, &isNull));
    TEST_ASSERT_EQUAL(expectedOut, out);
    TEST_ASSERT_FALSE(isNull);
}

void test_BLINK_DecodeF64(void)
{
    const uint8_t in[] = {0x00};
    double out;
    bool isNull = true;
    double expectedOut = 0;

    TEST_ASSERT_EQUAL(sizeof(in), BLINK_DecodeF64(in, sizeof(in), &out, &isNull));
    TEST_ASSERT_EQUAL(expectedOut, out);
    TEST_ASSERT_FALSE(isNull);
}

void test_BLINK_DecodeBool(void)
{
    const uint8_t in[] = {0x00};
    bool out;
    bool isNull = true;
    bool expectedOut = false;

    TEST_ASSERT_EQUAL(sizeof(in), BLINK_DecodeBool(in, sizeof(in), &out, &isNull));
    TEST_ASSERT_EQUAL(expectedOut, out);
    TEST_ASSERT_FALSE(isNull);
}

void test_BLINK_DecodeString(void)
{
    const uint8_t in[] = {0x05,'h','e','l','l','o'};
    const uint8_t *out;
    uint32_t outLen;
    bool isNull = true;
    const uint8_t expectedOut[] = {'h','e','l','l','o'};
    uint32_t expectedOutLen = sizeof(expectedOut);

    TEST_ASSERT_EQUAL(sizeof(in), BLINK_DecodeString(in, sizeof(in), &out, &outLen, &isNull));
    TEST_ASSERT_EQUAL(expectedOutLen, outLen);
    TEST_ASSERT_EQUAL_MEMORY(expectedOut, out, outLen);
    TEST_ASSERT_FALSE(isNull);
}

void test_BLINK_DecodeString_tooShort(void)
{
    const uint8_t in[] = {0x05,'h','e','l','l'};
    const uint8_t *out;
    uint32_t outLen;
    bool isNull = true;

    TEST_ASSERT_EQUAL(0, BLINK_DecodeString(in, sizeof(in), &out, &outLen, &isNull));    
}

void test_BLINK_DecodeBinary(void)
{
    const uint8_t in[] = {0x05,'h','e','l','l','o'};
    const uint8_t *out;
    uint32_t outLen;
    bool isNull = true;
    const uint8_t expectedOut[] = {'h','e','l','l','o'};
    uint32_t expectedOutLen = sizeof(expectedOut);

    TEST_ASSERT_EQUAL(sizeof(in), BLINK_DecodeBinary(in, sizeof(in), &out, &outLen, &isNull));
    TEST_ASSERT_EQUAL(expectedOutLen, outLen);
    TEST_ASSERT_EQUAL_MEMORY(expectedOut, out, outLen);
    TEST_ASSERT_FALSE(isNull);
}

void test_BLINK_DecodeBinary_tooShort(void)
{
    const uint8_t in[] = {0x05,'h','e','l','l'};
    const uint8_t *out;
    uint32_t outLen;
    bool isNull = true;

    TEST_ASSERT_EQUAL(0, BLINK_DecodeBinary(in, sizeof(in), &out, &outLen, &isNull));    
}

void test_BLINK_DecodeFixed(void)
{
    const uint8_t in[] = {'h','e','l','l','o'};    
    const uint8_t *out;            
    const uint8_t expectedOut[] = {'h','e','l','l','o'};    

    TEST_ASSERT_EQUAL(sizeof(in), BLINK_DecodeFixed(in, sizeof(in), &out, sizeof(expectedOut)));    
    TEST_ASSERT_EQUAL_MEMORY(expectedOut, out, sizeof(expectedOut));
}

void test_BLINK_DecodeFixed_tooShort(void)
{
    const uint8_t in[] = {'h','e','l','l'};     
    const uint8_t *out;        
    const uint8_t expectedOut[] = {'h','e','l','l','o'};    

    TEST_ASSERT_EQUAL(0, BLINK_DecodeFixed(in, sizeof(in), &out, sizeof(expectedOut)));        
}

void test_BLINK_DecodeOptionalFixed_present(void)
{
    const uint8_t in[] = {0x01, 'h','e','l','l','o'};    
    const uint8_t *out;        
    bool isNull = true;
    const uint8_t expectedOut[] = {'h','e','l','l','o'};    

    TEST_ASSERT_EQUAL(sizeof(in), BLINK_DecodeOptionalFixed(in, sizeof(in), &out, sizeof(expectedOut), &isNull));    
    TEST_ASSERT_EQUAL_MEMORY(expectedOut, out, sizeof(expectedOut));
    TEST_ASSERT_FALSE(isNull);
}

void test_BLINK_DecodeOptionalFixed_null(void)
{
    const uint8_t in[] = {0xc0};    
    const uint8_t *out;        
    bool isNull = true;
    const uint8_t expectedOut[] = {'h','e','l','l','o'};    

    TEST_ASSERT_EQUAL(sizeof(in), BLINK_DecodeOptionalFixed(in, sizeof(in), &out, sizeof(expectedOut), &isNull));        
    TEST_ASSERT_TRUE(isNull);
}

void test_BLINK_DecodeOptionalFixed_tooShort(void)
{
    const uint8_t in[] = {0x01, 'h','e','l','l'};     
    const uint8_t *out;        
    bool isNull = true;
    const uint8_t expectedOut[] = {'h','e','l','l','o'};    

    TEST_ASSERT_EQUAL(0, BLINK_DecodeOptionalFixed(in, sizeof(in), &out, sizeof(expectedOut), &isNull));        
}

void test_BLINK_DecodeOptionalFixed_invalidPresence(void)
{
    const uint8_t in[] = {0x02};    
    const uint8_t *out;        
    bool isNull = true;
    const uint8_t expectedOut[] = {'h','e','l','l','o'};    

    TEST_ASSERT_EQUAL(0, BLINK_DecodeOptionalFixed(in, sizeof(in), &out, sizeof(expectedOut), &isNull));        
}

void test_BLINK_Decimal(void)
{
    const uint8_t in[] = {0x00, 0x00};    
    int64_t mantissa;
    int8_t exponent;        
    bool isNull = true;
    int64_t expectedMantissa = 0;
    int8_t expectedExponent = 0;

    TEST_ASSERT_EQUAL(sizeof(in), BLINK_DecodeDecimal(in, sizeof(in), &mantissa, &exponent, &isNull));        
    TEST_ASSERT_EQUAL(expectedMantissa, mantissa);        
    TEST_ASSERT_EQUAL(expectedExponent, exponent);        
}

void test_BLINK_Decimal_nullMantissa(void)
{
    const uint8_t in[] = {0x00, 0xc0};    
    int64_t mantissa;
    int8_t exponent;        
    bool isNull = true;
    
    TEST_ASSERT_EQUAL(0, BLINK_DecodeDecimal(in, sizeof(in), &mantissa, &exponent, &isNull));        
}

