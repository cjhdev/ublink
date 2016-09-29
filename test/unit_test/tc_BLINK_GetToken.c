#include "unity.h"
#include "blink_lexer.h"
#include <string.h>

void setUp(void)
{    
}

void tearDown(void)
{
}

void test_BLINK_GetToken_name(void)
{
    size_t read;
    const char input[] = "test";
    union blink_token_value value;
    enum blink_token expected = T_NAME;

    TEST_ASSERT_EQUAL(expected, BLINK_GetToken(input, sizeof(input), &read, &value));
    TEST_ASSERT_EQUAL(strlen(input), read);
    TEST_ASSERT_EQUAL(strlen(input), value.literal.len);
    TEST_ASSERT_EQUAL_MEMORY(input, value.literal.ptr, value.literal.len);
}

void test_BLINK_GetToken_escaped_name(void)
{
    size_t read;
    const char input[] = "\\u8";
    union blink_token_value value;
    enum blink_token expected = T_NAME;

    TEST_ASSERT_EQUAL(expected, BLINK_GetToken(input, sizeof(input), &read, &value));
    TEST_ASSERT_EQUAL(strlen(input), read);
    TEST_ASSERT_EQUAL(strlen(input)-1, value.literal.len);
    TEST_ASSERT_EQUAL_MEMORY(&input[1], value.literal.ptr, value.literal.len);

    
}

void test_BLINK_GetToken_namespace(void)
{
    size_t read;
    const char input[] = "namespace";
    union blink_token_value value;
    enum blink_token expected = T_NAMESPACE;

    TEST_ASSERT_EQUAL(expected, BLINK_GetToken(input, sizeof(input), &read, &value));
    TEST_ASSERT_EQUAL(strlen(input), read);
}

void test_BLINK_GetToken_u8(void)
{
    size_t read;
    const char input[] = "u8";
    union blink_token_value value;
    enum blink_token expected = T_U8;

    TEST_ASSERT_EQUAL(expected, BLINK_GetToken(input, sizeof(input), &read, &value));
    TEST_ASSERT_EQUAL(strlen(input), read);
}

void test_BLINK_GetToken_u16(void)
{
    size_t read;
    const char input[] = "u16";
    union blink_token_value value;
    enum blink_token expected = T_U16;

    TEST_ASSERT_EQUAL(expected, BLINK_GetToken(input, sizeof(input), &read, &value));
    TEST_ASSERT_EQUAL(strlen(input), read);
}

void test_BLINK_GetToken_u32(void)
{
    size_t read;
    const char input[] = "u32";
    union blink_token_value value;
    enum blink_token expected = T_U32;

    TEST_ASSERT_EQUAL(expected, BLINK_GetToken(input, sizeof(input), &read, &value));
    TEST_ASSERT_EQUAL(strlen(input), read);
}

void test_BLINK_GetToken_u64(void)
{
    size_t read;
    const char input[] = "u64";
    union blink_token_value value;
    enum blink_token expected = T_U64;

    TEST_ASSERT_EQUAL(expected, BLINK_GetToken(input, sizeof(input), &read, &value));
    TEST_ASSERT_EQUAL(strlen(input), read);
}

void test_BLINK_GetToken_i8(void)
{
    size_t read;
    const char input[] = "i8";
    union blink_token_value value;
    enum blink_token expected = T_I8;

    TEST_ASSERT_EQUAL(expected, BLINK_GetToken(input, sizeof(input), &read, &value));
    TEST_ASSERT_EQUAL(strlen(input), read);
}

void test_BLINK_GetToken_i16(void)
{
    size_t read;
    const char input[] = "i16";
    union blink_token_value value;
    enum blink_token expected = T_I16;

    TEST_ASSERT_EQUAL(expected, BLINK_GetToken(input, sizeof(input), &read, &value));
    TEST_ASSERT_EQUAL(strlen(input), read);
}

void test_BLINK_GetToken_i32(void)
{
    size_t read;
    const char input[] = "i32";
    union blink_token_value value;
    enum blink_token expected = T_I32;

    TEST_ASSERT_EQUAL(expected, BLINK_GetToken(input, sizeof(input), &read, &value));
    TEST_ASSERT_EQUAL(strlen(input), read);
}

void test_BLINK_GetToken_i64(void)
{
    size_t read;
    const char input[] = "i64";
    union blink_token_value value;
    enum blink_token expected = T_I64;

    TEST_ASSERT_EQUAL(expected, BLINK_GetToken(input, sizeof(input), &read, &value));
    TEST_ASSERT_EQUAL(strlen(input), read);
}

void test_BLINK_GetToken_f64(void)
{
    size_t read;
    const char input[] = "f64";
    union blink_token_value value;
    enum blink_token expected = T_F64;

    TEST_ASSERT_EQUAL(expected, BLINK_GetToken(input, sizeof(input), &read, &value));
    TEST_ASSERT_EQUAL(strlen(input), read);
}

void test_BLINK_GetToken_string(void)
{
    size_t read;
    const char input[] = "string";
    union blink_token_value value;
    enum blink_token expected = T_STRING;

    TEST_ASSERT_EQUAL(expected, BLINK_GetToken(input, sizeof(input), &read, &value));
    TEST_ASSERT_EQUAL(strlen(input), read);
}

void test_BLINK_GetToken_binary(void)
{
    size_t read;
    const char input[] = "binary";
    union blink_token_value value;
    enum blink_token expected = T_BINARY;

    TEST_ASSERT_EQUAL(expected, BLINK_GetToken(input, sizeof(input), &read, &value));
    TEST_ASSERT_EQUAL(strlen(input), read);
}

void test_BLINK_GetToken_fixed(void)
{
    size_t read;
    const char input[] = "fixed";
    union blink_token_value value;
    enum blink_token expected = T_FIXED;

    TEST_ASSERT_EQUAL(expected, BLINK_GetToken(input, sizeof(input), &read, &value));
    TEST_ASSERT_EQUAL(strlen(input), read);
}

void test_BLINK_GetToken_decimal(void)
{
    size_t read;
    const char input[] = "decimal";
    union blink_token_value value;
    enum blink_token expected = T_DECIMAL;

    TEST_ASSERT_EQUAL(expected, BLINK_GetToken(input, sizeof(input), &read, &value));
    TEST_ASSERT_EQUAL(strlen(input), read);
}

void test_BLINK_GetToken_date(void)
{
    size_t read;
    const char input[] = "date";
    union blink_token_value value;
    enum blink_token expected = T_DATE;

    TEST_ASSERT_EQUAL(expected, BLINK_GetToken(input, sizeof(input), &read, &value));
    TEST_ASSERT_EQUAL(strlen(input), read);
}

void test_BLINK_GetToken_millitime(void)
{
    size_t read;
    const char input[] = "millitime";
    union blink_token_value value;
    enum blink_token expected = T_MILLI_TIME;

    TEST_ASSERT_EQUAL(expected, BLINK_GetToken(input, sizeof(input), &read, &value));
    TEST_ASSERT_EQUAL(strlen(input), read);
}

void test_BLINK_GetToken_nanotime(void)
{
    size_t read;
    const char input[] = "nanotime";
    union blink_token_value value;
    enum blink_token expected = T_NANO_TIME;

    TEST_ASSERT_EQUAL(expected, BLINK_GetToken(input, sizeof(input), &read, &value));
    TEST_ASSERT_EQUAL(strlen(input), read);
}

void test_BLINK_GetToken_timeOfDayMilli(void)
{
    size_t read;
    const char input[] = "timeOfDayMilli";
    union blink_token_value value;
    enum blink_token expected = T_TIME_OF_DAY_MILLI;

    TEST_ASSERT_EQUAL(expected, BLINK_GetToken(input, sizeof(input), &read, &value));
    TEST_ASSERT_EQUAL(strlen(input), read);
}

void test_BLINK_GetToken_timeOfDayNano(void)
{
    size_t read;
    const char input[] = "timeOfDayNano";
    union blink_token_value value;
    enum blink_token expected = T_TIME_OF_DAY_NANO;

    TEST_ASSERT_EQUAL(expected, BLINK_GetToken(input, sizeof(input), &read, &value));
    TEST_ASSERT_EQUAL(strlen(input), read);
}

void test_BLINK_GetToken_schema(void)
{
    size_t read;
    const char input[] = "schema";
    union blink_token_value value;
    enum blink_token expected = T_SCHEMA;

    TEST_ASSERT_EQUAL(expected, BLINK_GetToken(input, sizeof(input), &read, &value));
    TEST_ASSERT_EQUAL(strlen(input), read);
}

void test_BLINK_GetToken_type(void)
{
    size_t read;
    const char input[] = "type";
    union blink_token_value value;
    enum blink_token expected = T_TYPE;

    TEST_ASSERT_EQUAL(expected, BLINK_GetToken(input, sizeof(input), &read, &value));
    TEST_ASSERT_EQUAL(strlen(input), read);
}


