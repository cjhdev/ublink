/**
 * @example __FILE__
 *
 * */

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>

#include "cmocka.h"
#include "blink_compact.h"

void test_BLINK_Compact_decodeU8(void **user)
{
    const uint8_t in[] = {0x00};
    uint8_t out;
    bool isNull = true;
    uint8_t expectedOut = 0x00;

    assert_int_equal(sizeof(in), BLINK_Compact_decodeU8(in, sizeof(in), &out, &isNull));
    assert_int_equal(expectedOut, out);
    assert_false(isNull);
}

void test_BLINK_Compact_decodeU8_max(void **user)
{
    const uint8_t in[] = {0xbf, 0x03};
    uint8_t out;
    bool isNull = true;
    uint8_t expectedOut = UINT8_MAX;

    assert_int_equal(sizeof(in), BLINK_Compact_decodeU8(in, sizeof(in), &out, &isNull));
    assert_int_equal(expectedOut, out);
    assert_false(isNull);
}

void test_BLINK_Compact_decodeU16(void **user)
{
    const uint8_t in[] = {0x00};
    uint16_t out;
    bool isNull = true;
    uint16_t expectedOut = 0x00;

    assert_int_equal(sizeof(in), BLINK_Compact_decodeU16(in, sizeof(in), &out, &isNull));
    assert_int_equal(expectedOut, out);
    assert_false(isNull);
}

void test_BLINK_Compact_decodeU16_max(void **user)
{
    const uint8_t in[] = {0xc2,0xff,0xff};
    uint16_t out;
    bool isNull = true;
    uint16_t expectedOut = UINT16_MAX;

    assert_int_equal(sizeof(in), BLINK_Compact_decodeU16(in, sizeof(in), &out, &isNull));
    assert_int_equal(expectedOut, out);
    assert_false(isNull);
}

void test_BLINK_Compact_decodeU32(void **user)
{
    const uint8_t in[] = {0x00};
    uint32_t out;
    bool isNull = true;
    uint32_t expectedOut = 0x00;

    assert_int_equal(sizeof(in), BLINK_Compact_decodeU32(in, sizeof(in), &out, &isNull));
    assert_int_equal(expectedOut, out);
    assert_false(isNull);
}

void test_BLINK_Compact_decodeU32_max(void **user)
{
    const uint8_t in[] = {0xc4,0xff,0xff,0xff,0xff};
    uint32_t out;
    bool isNull = true;
    uint32_t expectedOut = UINT32_MAX;

    assert_int_equal(sizeof(in), BLINK_Compact_decodeU32(in, sizeof(in), &out, &isNull));
    assert_int_equal(expectedOut, out);
    assert_false(isNull);
}

void test_BLINK_Compact_decodeU64(void **user)
{
    const uint8_t in[] = {0x00};
    uint64_t out;
    bool isNull = true;
    uint64_t expectedOut = 0x00;

    assert_int_equal(sizeof(in), BLINK_Compact_decodeU64(in, sizeof(in), &out, &isNull));
    assert_int_equal(expectedOut, out);
    assert_false(isNull);
}

void test_BLINK_Compact_decodeU64_max(void **user)
{
    const uint8_t in[] = {0xc8, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    uint64_t out;
    bool isNull = true;
    uint64_t expectedOut = UINT64_MAX;

    assert_int_equal(sizeof(in), BLINK_Compact_decodeU64(in, sizeof(in), &out, &isNull));
    assert_int_equal(expectedOut, out);
    assert_false(isNull);
}

void test_BLINK_Compact_decodeI8(void **user)
{
    const uint8_t in[] = {0x00};
    int8_t out;
    bool isNull = true;
    int8_t expectedOut = 0x00;

    assert_int_equal(sizeof(in), BLINK_Compact_decodeI8(in, sizeof(in), &out, &isNull));
    assert_int_equal(expectedOut, out);
    assert_false(isNull);
}

void test_BLINK_Compact_decodeI8_min(void **user)
{
    const uint8_t in[] = {0x80, 0xfe};
    int8_t out;
    bool isNull = true;
    int8_t expectedOut = INT8_MIN;

    assert_int_equal(sizeof(in), BLINK_Compact_decodeI8(in, sizeof(in), &out, &isNull));
    assert_int_equal(expectedOut, out);
    assert_false(isNull);
}

void test_BLINK_Compact_decodeI8_max(void **user)
{
    const uint8_t in[] = {0xbf,0x01};
    int8_t out;
    bool isNull = true;
    int8_t expectedOut = INT8_MAX;

    assert_int_equal(sizeof(in), BLINK_Compact_decodeI8(in, sizeof(in), &out, &isNull));
    assert_int_equal(expectedOut, out);
    assert_false(isNull);
}

void test_BLINK_Compact_decodeI16(void **user)
{
    const uint8_t in[] = {0x00};
    int16_t out;
    bool isNull = true;
    int16_t expectedOut = 0x00;

    assert_int_equal(sizeof(in), BLINK_Compact_decodeI16(in, sizeof(in), &out, &isNull));
    assert_int_equal(expectedOut, out);
    assert_false(isNull);
}

void test_BLINK_Compact_decodeI16_min(void **user)
{
    const uint8_t in[] = {0xc2, 0x00, 0x80};
    int16_t out;
    bool isNull = true;
    int16_t expectedOut = INT16_MIN;

    assert_int_equal(sizeof(in), BLINK_Compact_decodeI16(in, sizeof(in), &out, &isNull));
    assert_int_equal(expectedOut, out);
    assert_false(isNull);
}
void test_BLINK_Compact_decodeI16_max(void **user)
{
    const uint8_t in[] = {0xc2, 0xff, 0x7f};
    int16_t out;
    bool isNull = true;
    int16_t expectedOut = INT16_MAX;

    assert_int_equal(sizeof(in), BLINK_Compact_decodeI16(in, sizeof(in), &out, &isNull));
    assert_int_equal(expectedOut, out);
    assert_false(isNull);
}

void test_BLINK_Compact_decodeI32(void **user)
{
    const uint8_t in[] = {0x00};
    int32_t out;
    bool isNull = true;
    int32_t expectedOut = 0x00;

    assert_int_equal(sizeof(in), BLINK_Compact_decodeI32(in, sizeof(in), &out, &isNull));
    assert_int_equal(expectedOut, out);
    assert_false(isNull);
}    

void test_BLINK_Compact_decodeI32_min(void **user)
{
    const uint8_t in[] = {0xc4, 0x00, 0x00, 0x00, 0x80};
    int32_t out;
    bool isNull = true;
    int32_t expectedOut = INT32_MIN;

    assert_int_equal(sizeof(in), BLINK_Compact_decodeI32(in, sizeof(in), &out, &isNull));
    assert_int_equal(expectedOut, out);
    assert_false(isNull);
}

void test_BLINK_Compact_decodeI32_max(void **user)
{
    const uint8_t in[] = {0xc4, 0xff,0xff,0xff,0x7f};
    int32_t out;
    bool isNull = true;
    int32_t expectedOut = INT32_MAX;

    assert_int_equal(sizeof(in), BLINK_Compact_decodeI32(in, sizeof(in), &out, &isNull));
    assert_int_equal(expectedOut, out);
    assert_false(isNull);
}

void test_BLINK_Compact_decodeI64(void **user)
{
    const uint8_t in[] = {0x00};
    int64_t out;
    bool isNull = true;
    int64_t expectedOut = 0x00;

    assert_int_equal(sizeof(in), BLINK_Compact_decodeI64(in, sizeof(in), &out, &isNull));
    assert_int_equal(expectedOut, out);
    assert_false(isNull);
}

void test_BLINK_Compact_decodeI64_min(void **user)
{
    const uint8_t in[] = {0xc8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80};
    int64_t out;
    bool isNull = true;
    int64_t expectedOut = INT64_MIN;

    assert_int_equal(sizeof(in), BLINK_Compact_decodeI64(in, sizeof(in), &out, &isNull));
    assert_int_equal(expectedOut, out);
    assert_false(isNull);
}

void test_BLINK_Compact_decodeI64_max(void **user)
{
    const uint8_t in[] = {0xc8, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f};
    int64_t out;
    bool isNull = true;
    int64_t expectedOut = INT64_MAX;

    assert_int_equal(sizeof(in), BLINK_Compact_decodeI64(in, sizeof(in), &out, &isNull));
    assert_int_equal(expectedOut, out);
    assert_false(isNull);
}

void test_BLINK_Compact_decodeF64(void **user)
{
    const uint8_t in[] = {0x00};
    double out;
    bool isNull = true;
    double expectedOut = 0;

    assert_int_equal(sizeof(in), BLINK_Compact_decodeF64(in, sizeof(in), &out, &isNull));
    assert_int_equal(expectedOut, out);
    assert_false(isNull);
}

void test_BLINK_Compact_decodeBool(void **user)
{
    const uint8_t in[] = {0x00};
    bool out;
    bool isNull = true;
    bool expectedOut = false;

    assert_int_equal(sizeof(in), BLINK_Compact_decodeBool(in, sizeof(in), &out, &isNull));
    assert_int_equal(expectedOut, out);
    assert_false(isNull);
}

void test_BLINK_Compact_decodeString(void **user)
{
    const uint8_t in[] = {0x05,'h','e','l','l','o'};
    const uint8_t *out;
    uint32_t outLen;
    bool isNull = true;
    const uint8_t expectedOut[] = {'h','e','l','l','o'};
    uint32_t expectedOutLen = sizeof(expectedOut);

    assert_int_equal(sizeof(in), BLINK_Compact_decodeString(in, sizeof(in), &out, &outLen, &isNull));
    assert_int_equal(expectedOutLen, outLen);
    assert_memory_equal(expectedOut, out, outLen);
    assert_false(isNull);
}

void test_BLINK_Compact_decodeString_tooShort(void **user)
{
    const uint8_t in[] = {0x05,'h','e','l','l'};
    const uint8_t *out;
    uint32_t outLen;
    bool isNull = true;

    assert_int_equal(0, BLINK_Compact_decodeString(in, sizeof(in), &out, &outLen, &isNull));    
}

void test_BLINK_Compact_decodeBinary(void **user)
{
    const uint8_t in[] = {0x05,'h','e','l','l','o'};
    const uint8_t *out;
    uint32_t outLen;
    bool isNull = true;
    const uint8_t expectedOut[] = {'h','e','l','l','o'};
    uint32_t expectedOutLen = sizeof(expectedOut);

    assert_int_equal(sizeof(in), BLINK_Compact_decodeBinary(in, sizeof(in), &out, &outLen, &isNull));
    assert_int_equal(expectedOutLen, outLen);
    assert_memory_equal(expectedOut, out, outLen);
    assert_false(isNull);
}

void test_BLINK_Compact_decodeBinary_tooShort(void **user)
{
    const uint8_t in[] = {0x05,'h','e','l','l'};
    const uint8_t *out;
    uint32_t outLen;
    bool isNull = true;

    assert_int_equal(0, BLINK_Compact_decodeBinary(in, sizeof(in), &out, &outLen, &isNull));    
}

void test_BLINK_Compact_decodeBinary_null(void **user)
{
    const uint8_t in[] = {0xc0};
    const uint8_t *out;
    uint32_t outLen;
    bool isNull = true;

    assert_int_equal(sizeof(in), BLINK_Compact_decodeBinary(in, sizeof(in), &out, &outLen, &isNull));    
    assert_true(isNull);    
}

void test_BLINK_Compact_decodeFixed(void **user)
{
    const uint8_t in[] = {'h','e','l','l','o'};    
    const uint8_t *out;            
    const uint8_t expectedOut[] = {'h','e','l','l','o'};    

    assert_int_equal(sizeof(in), BLINK_Compact_decodeFixed(in, sizeof(in), &out, sizeof(expectedOut)));    
    assert_memory_equal(expectedOut, out, sizeof(expectedOut));
}

void test_BLINK_Compact_decodeFixed_tooShort(void **user)
{
    const uint8_t in[] = {'h','e','l','l'};     
    const uint8_t *out;        
    const uint8_t expectedOut[] = {'h','e','l','l','o'};    

    assert_int_equal(0, BLINK_Compact_decodeFixed(in, sizeof(in), &out, sizeof(expectedOut)));        
}

void test_BLINK_Compact_decodeOptionalFixed_present(void **user)
{
    const uint8_t in[] = {0x01, 'h','e','l','l','o'};    
    const uint8_t *out;        
    bool isNull = true;
    const uint8_t expectedOut[] = {'h','e','l','l','o'};    

    assert_int_equal(sizeof(in), BLINK_Compact_decodeOptionalFixed(in, sizeof(in), &out, sizeof(expectedOut), &isNull));    
    assert_memory_equal(expectedOut, out, sizeof(expectedOut));
    assert_false(isNull);
}

void test_BLINK_Compact_decodeOptionalFixed_null(void **user)
{
    const uint8_t in[] = {0xc0};    
    const uint8_t *out;        
    bool isNull = true;
    const uint8_t expectedOut[] = {'h','e','l','l','o'};    

    assert_int_equal(sizeof(in), BLINK_Compact_decodeOptionalFixed(in, sizeof(in), &out, sizeof(expectedOut), &isNull));        
    assert_true(isNull);
}

void test_BLINK_Compact_decodeOptionalFixed_tooShort(void **user)
{
    const uint8_t in[] = {0x01, 'h','e','l','l'};     
    const uint8_t *out;        
    bool isNull = true;
    const uint8_t expectedOut[] = {'h','e','l','l','o'};    

    assert_int_equal(0, BLINK_Compact_decodeOptionalFixed(in, sizeof(in), &out, sizeof(expectedOut), &isNull));        
}

void test_BLINK_Compact_decodeOptionalFixed_invalidPresence(void **user)
{
    const uint8_t in[] = {0x02};    
    const uint8_t *out;        
    bool isNull = true;
    const uint8_t expectedOut[] = {'h','e','l','l','o'};    

    assert_int_equal(0, BLINK_Compact_decodeOptionalFixed(in, sizeof(in), &out, sizeof(expectedOut), &isNull));        
}

void test_BLINK_Compact_Decimal(void **user)
{
    const uint8_t in[] = {0x00, 0x00};    
    int64_t mantissa;
    int8_t exponent;        
    bool isNull = true;
    int64_t expectedMantissa = 0;
    int8_t expectedExponent = 0;

    assert_int_equal(sizeof(in), BLINK_Compact_decodeDecimal(in, sizeof(in), &mantissa, &exponent, &isNull));        
    assert_int_equal(expectedMantissa, mantissa);        
    assert_int_equal(expectedExponent, exponent);        
}

void test_BLINK_Compact_Decimal_nullMantissa(void **user)
{
    const uint8_t in[] = {0x00, 0xc0};    
    int64_t mantissa;
    int8_t exponent;        
    bool isNull = true;
    
    assert_int_equal(0, BLINK_Compact_decodeDecimal(in, sizeof(in), &mantissa, &exponent, &isNull));        
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_BLINK_Compact_decodeU8),
        cmocka_unit_test(test_BLINK_Compact_decodeU8_max),
        cmocka_unit_test(test_BLINK_Compact_decodeU16),
        cmocka_unit_test(test_BLINK_Compact_decodeU16_max),
        cmocka_unit_test(test_BLINK_Compact_decodeU32),
        cmocka_unit_test(test_BLINK_Compact_decodeU32_max),
        cmocka_unit_test(test_BLINK_Compact_decodeU64),
        cmocka_unit_test(test_BLINK_Compact_decodeU64_max),
        cmocka_unit_test(test_BLINK_Compact_decodeI8),
        cmocka_unit_test(test_BLINK_Compact_decodeI8_min),
        cmocka_unit_test(test_BLINK_Compact_decodeI8_max),
        cmocka_unit_test(test_BLINK_Compact_decodeI16),
        cmocka_unit_test(test_BLINK_Compact_decodeI16_min),
        cmocka_unit_test(test_BLINK_Compact_decodeI16_max),
        cmocka_unit_test(test_BLINK_Compact_decodeI32),
        cmocka_unit_test(test_BLINK_Compact_decodeI32_min),
        cmocka_unit_test(test_BLINK_Compact_decodeI32_max),
        cmocka_unit_test(test_BLINK_Compact_decodeI64),
        cmocka_unit_test(test_BLINK_Compact_decodeI64_min),
        cmocka_unit_test(test_BLINK_Compact_decodeI64_max),
        cmocka_unit_test(test_BLINK_Compact_decodeF64),
        cmocka_unit_test(test_BLINK_Compact_decodeBool),
        cmocka_unit_test(test_BLINK_Compact_decodeString),
        cmocka_unit_test(test_BLINK_Compact_decodeString_tooShort),
        cmocka_unit_test(test_BLINK_Compact_decodeBinary),
        cmocka_unit_test(test_BLINK_Compact_decodeBinary_tooShort),
        cmocka_unit_test(test_BLINK_Compact_decodeBinary_null),
        cmocka_unit_test(test_BLINK_Compact_decodeFixed),
        cmocka_unit_test(test_BLINK_Compact_decodeFixed_tooShort),
        cmocka_unit_test(test_BLINK_Compact_decodeOptionalFixed_present),
        cmocka_unit_test(test_BLINK_Compact_decodeOptionalFixed_null),
        cmocka_unit_test(test_BLINK_Compact_decodeOptionalFixed_tooShort),
        cmocka_unit_test(test_BLINK_Compact_decodeOptionalFixed_invalidPresence),
        cmocka_unit_test(test_BLINK_Compact_Decimal),
        cmocka_unit_test(test_BLINK_Compact_Decimal_nullMantissa)
    };
    
    return cmocka_run_group_tests(tests, NULL, NULL);
}
