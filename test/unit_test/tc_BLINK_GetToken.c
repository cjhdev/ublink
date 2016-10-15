/**
 * @example tc_BLINK_GetToken.c
 *
 * */

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
    enum blink_token expected = TOK_NAME;

    TEST_ASSERT_EQUAL(expected, BLINK_GetToken(input, sizeof(input), &read, &value, NULL));
    TEST_ASSERT_EQUAL(strlen(input), read);
    TEST_ASSERT_EQUAL(strlen(input), value.literal.len);
    TEST_ASSERT_EQUAL_MEMORY(input, value.literal.ptr, value.literal.len);
}

void test_BLINK_GetToken_name_leadingWS(void)
{
    size_t read;
    const char input[] = " test";
    union blink_token_value value;
    enum blink_token expected = TOK_NAME;

    TEST_ASSERT_EQUAL(expected, BLINK_GetToken(input, sizeof(input), &read, &value, NULL));
    TEST_ASSERT_EQUAL(strlen(" test"), read);
    TEST_ASSERT_EQUAL(strlen("test"), value.literal.len);
    TEST_ASSERT_EQUAL_MEMORY("test", value.literal.ptr, value.literal.len);
}

void test_BLINK_GetToken_name_leadingWS_trailingWS(void)
{
    size_t read;
    const char input[] = " test ";
    union blink_token_value value;
    enum blink_token expected = TOK_NAME;

    TEST_ASSERT_EQUAL(expected, BLINK_GetToken(input, sizeof(input), &read, &value, NULL));
    TEST_ASSERT_EQUAL(strlen(" test"), read);
    TEST_ASSERT_EQUAL(strlen("test"), value.literal.len);
    TEST_ASSERT_EQUAL_MEMORY("test", value.literal.ptr, value.literal.len);
}

void test_BLINK_GetToken_double_name(void)
{
    size_t read;
    size_t readAgain;
    const char input[] = "test again";
    union blink_token_value value;
    enum blink_token expected = TOK_NAME;

    TEST_ASSERT_EQUAL(expected, BLINK_GetToken(input, sizeof(input), &read, &value, NULL));

    TEST_ASSERT_EQUAL(strlen("test"), read);
    TEST_ASSERT_EQUAL(strlen("test"), value.literal.len);
    TEST_ASSERT_EQUAL_MEMORY("test", value.literal.ptr, value.literal.len);

    TEST_ASSERT_EQUAL(expected, BLINK_GetToken(&input[read], sizeof(input) - read, &readAgain, &value, NULL));

    TEST_ASSERT_EQUAL(strlen(" again"), readAgain);
    TEST_ASSERT_EQUAL(strlen("again"), value.literal.len);
    TEST_ASSERT_EQUAL_MEMORY("again", value.literal.ptr, value.literal.len);
}

void test_BLINK_GetToken_escaped_name(void)
{
    size_t read;
    const char input[] = "\\u8";
    union blink_token_value value;
    enum blink_token expected = TOK_NAME;

    TEST_ASSERT_EQUAL(expected, BLINK_GetToken(input, sizeof(input), &read, &value, NULL));
    TEST_ASSERT_EQUAL(strlen(input), read);
    TEST_ASSERT_EQUAL(strlen(input)-1, value.literal.len);
    TEST_ASSERT_EQUAL_MEMORY(&input[1], value.literal.ptr, value.literal.len);
}

void test_BLINK_GetToken_namespace(void)
{
    size_t read;
    const char input[] = "namespace";
    union blink_token_value value;
    enum blink_token expected = TOK_NAMESPACE;

    TEST_ASSERT_EQUAL(expected, BLINK_GetToken(input, sizeof(input), &read, &value, NULL));
    TEST_ASSERT_EQUAL(strlen(input), read);
}

void test_BLINK_GetToken_u8(void)
{
    size_t read;
    const char input[] = "u8";
    union blink_token_value value;
    enum blink_token expected = TOK_U8;

    TEST_ASSERT_EQUAL(expected, BLINK_GetToken(input, sizeof(input), &read, &value, NULL));
    TEST_ASSERT_EQUAL(strlen(input), read);
}

void test_BLINK_GetToken_u16(void)
{
    size_t read;
    const char input[] = "u16";
    union blink_token_value value;
    enum blink_token expected = TOK_U16;

    TEST_ASSERT_EQUAL(expected, BLINK_GetToken(input, sizeof(input), &read, &value, NULL));
    TEST_ASSERT_EQUAL(strlen(input), read);
}

void test_BLINK_GetToken_u32(void)
{
    size_t read;
    const char input[] = "u32";
    union blink_token_value value;
    enum blink_token expected = TOK_U32;

    TEST_ASSERT_EQUAL(expected, BLINK_GetToken(input, sizeof(input), &read, &value, NULL));
    TEST_ASSERT_EQUAL(strlen(input), read);
}

void test_BLINK_GetToken_u64(void)
{
    size_t read;
    const char input[] = "u64";
    union blink_token_value value;
    enum blink_token expected = TOK_U64;

    TEST_ASSERT_EQUAL(expected, BLINK_GetToken(input, sizeof(input), &read, &value, NULL));
    TEST_ASSERT_EQUAL(strlen(input), read);
}

void test_BLINK_GetToken_i8(void)
{
    size_t read;
    const char input[] = "i8";
    union blink_token_value value;
    enum blink_token expected = TOK_I8;

    TEST_ASSERT_EQUAL(expected, BLINK_GetToken(input, sizeof(input), &read, &value, NULL));
    TEST_ASSERT_EQUAL(strlen(input), read);
}

void test_BLINK_GetToken_i16(void)
{
    size_t read;
    const char input[] = "i16";
    union blink_token_value value;
    enum blink_token expected = TOK_I16;

    TEST_ASSERT_EQUAL(expected, BLINK_GetToken(input, sizeof(input), &read, &value, NULL));
    TEST_ASSERT_EQUAL(strlen(input), read);
}

void test_BLINK_GetToken_i32(void)
{
    size_t read;
    const char input[] = "i32";
    union blink_token_value value;
    enum blink_token expected = TOK_I32;

    TEST_ASSERT_EQUAL(expected, BLINK_GetToken(input, sizeof(input), &read, &value, NULL));
    TEST_ASSERT_EQUAL(strlen(input), read);
}

void test_BLINK_GetToken_i64(void)
{
    size_t read;
    const char input[] = "i64";
    union blink_token_value value;
    enum blink_token expected = TOK_I64;

    TEST_ASSERT_EQUAL(expected, BLINK_GetToken(input, sizeof(input), &read, &value, NULL));
    TEST_ASSERT_EQUAL(strlen(input), read);
}

void test_BLINK_GetToken_f64(void)
{
    size_t read;
    const char input[] = "f64";
    union blink_token_value value;
    enum blink_token expected = TOK_F64;

    TEST_ASSERT_EQUAL(expected, BLINK_GetToken(input, sizeof(input), &read, &value, NULL));
    TEST_ASSERT_EQUAL(strlen(input), read);
}

void test_BLINK_GetToken_string(void)
{
    size_t read;
    const char input[] = "string";
    union blink_token_value value;
    enum blink_token expected = TOK_STRING;

    TEST_ASSERT_EQUAL(expected, BLINK_GetToken(input, sizeof(input), &read, &value, NULL));
    TEST_ASSERT_EQUAL(strlen(input), read);
}

void test_BLINK_GetToken_binary(void)
{
    size_t read;
    const char input[] = "binary";
    union blink_token_value value;
    enum blink_token expected = TOK_BINARY;

    TEST_ASSERT_EQUAL(expected, BLINK_GetToken(input, sizeof(input), &read, &value, NULL));
    TEST_ASSERT_EQUAL(strlen(input), read);
}

void test_BLINK_GetToken_fixed(void)
{
    size_t read;
    const char input[] = "fixed";
    union blink_token_value value;
    enum blink_token expected = TOK_FIXED;

    TEST_ASSERT_EQUAL(expected, BLINK_GetToken(input, sizeof(input), &read, &value, NULL));
    TEST_ASSERT_EQUAL(strlen(input), read);
}

void test_BLINK_GetToken_decimal(void)
{
    size_t read;
    const char input[] = "decimal";
    union blink_token_value value;
    enum blink_token expected = TOK_DECIMAL;

    TEST_ASSERT_EQUAL(expected, BLINK_GetToken(input, sizeof(input), &read, &value, NULL));
    TEST_ASSERT_EQUAL(strlen(input), read);
}

void test_BLINK_GetToken_date(void)
{
    size_t read;
    const char input[] = "date";
    union blink_token_value value;
    enum blink_token expected = TOK_DATE;

    TEST_ASSERT_EQUAL(expected, BLINK_GetToken(input, sizeof(input), &read, &value, NULL));
    TEST_ASSERT_EQUAL(strlen(input), read);
}

void test_BLINK_GetToken_millitime(void)
{
    size_t read;
    const char input[] = "millitime";
    union blink_token_value value;
    enum blink_token expected = TOK_MILLI_TIME;

    TEST_ASSERT_EQUAL(expected, BLINK_GetToken(input, sizeof(input), &read, &value, NULL));
    TEST_ASSERT_EQUAL(strlen(input), read);
}

void test_BLINK_GetToken_nanotime(void)
{
    size_t read;
    const char input[] = "nanotime";
    union blink_token_value value;
    enum blink_token expected = TOK_NANO_TIME;

    TEST_ASSERT_EQUAL(expected, BLINK_GetToken(input, sizeof(input), &read, &value, NULL));
    TEST_ASSERT_EQUAL(strlen(input), read);
}

void test_BLINK_GetToken_timeOfDayMilli(void)
{
    size_t read;
    const char input[] = "timeOfDayMilli";
    union blink_token_value value;
    enum blink_token expected = TOK_TIME_OF_DAY_MILLI;

    TEST_ASSERT_EQUAL(expected, BLINK_GetToken(input, sizeof(input), &read, &value, NULL));
    TEST_ASSERT_EQUAL(strlen(input), read);
}

void test_BLINK_GetToken_timeOfDayNano(void)
{
    size_t read;
    const char input[] = "timeOfDayNano";
    union blink_token_value value;
    enum blink_token expected = TOK_TIME_OF_DAY_NANO;

    TEST_ASSERT_EQUAL(expected, BLINK_GetToken(input, sizeof(input), &read, &value, NULL));
    TEST_ASSERT_EQUAL(strlen(input), read);
}

void test_BLINK_GetToken_schema(void)
{
    size_t read;
    const char input[] = "schema";
    union blink_token_value value;
    enum blink_token expected = TOK_SCHEMA;

    TEST_ASSERT_EQUAL(expected, BLINK_GetToken(input, sizeof(input), &read, &value, NULL));
    TEST_ASSERT_EQUAL(strlen(input), read);
}

void test_BLINK_GetToken_type(void)
{
    size_t read;
    const char input[] = "type";
    union blink_token_value value;
    enum blink_token expected = TOK_TYPE;

    TEST_ASSERT_EQUAL(expected, BLINK_GetToken(input, sizeof(input), &read, &value, NULL));
    TEST_ASSERT_EQUAL(strlen(input), read);
}

void test_BLINK_GetToken_rarrow(void)
{
    size_t read;
    const char input[] = "->";
    union blink_token_value value;
    enum blink_token expected = TOK_RARROW;

    TEST_ASSERT_EQUAL(expected, BLINK_GetToken(input, sizeof(input), &read, &value, NULL));
    TEST_ASSERT_EQUAL(strlen(input), read);
}

void test_BLINK_GetToken_larrow(void)
{
    size_t read;
    const char input[] = "<-";
    union blink_token_value value;
    enum blink_token expected = TOK_LARROW;

    TEST_ASSERT_EQUAL(expected, BLINK_GetToken(input, sizeof(input), &read, &value, NULL));
    TEST_ASSERT_EQUAL(strlen(input), read);
}

void test_BLINK_GetToken_bool(void)
{
    size_t read;
    const char input[] = "bool";
    union blink_token_value value;
    enum blink_token expected = TOK_BOOL;

    TEST_ASSERT_EQUAL(expected, BLINK_GetToken(input, sizeof(input), &read, &value, NULL));
    TEST_ASSERT_EQUAL(strlen(input), read);
}

void test_BLINK_GetToken_star(void)
{
    size_t read;
    const char input[] = "*";
    union blink_token_value value;
    enum blink_token expected = TOK_STAR;

    TEST_ASSERT_EQUAL(expected, BLINK_GetToken(input, sizeof(input), &read, &value, NULL));
    TEST_ASSERT_EQUAL(strlen(input), read);
}

void test_BLINK_GetToken_equal(void)
{
    size_t read;
    const char input[] = "=";
    union blink_token_value value;
    enum blink_token expected = TOK_EQUAL;

    TEST_ASSERT_EQUAL(expected, BLINK_GetToken(input, sizeof(input), &read, &value, NULL));
    TEST_ASSERT_EQUAL(strlen(input), read);
}

void test_BLINK_GetToken_period(void)
{
    size_t read;
    const char input[] = ".";
    union blink_token_value value;
    enum blink_token expected = TOK_PERIOD;

    TEST_ASSERT_EQUAL(expected, BLINK_GetToken(input, sizeof(input), &read, &value, NULL));
    TEST_ASSERT_EQUAL(strlen(input), read);
}

void test_BLINK_GetToken_comma(void)
{
    size_t read;
    const char input[] = ",";
    union blink_token_value value;
    enum blink_token expected = TOK_COMMA;

    TEST_ASSERT_EQUAL(expected, BLINK_GetToken(input, sizeof(input), &read, &value, NULL));
    TEST_ASSERT_EQUAL(strlen(input), read);
}

void test_BLINK_GetToken_lbracket(void)
{
    size_t read;
    const char input[] = "[";
    union blink_token_value value;
    enum blink_token expected = TOK_LBRACKET;

    TEST_ASSERT_EQUAL(expected, BLINK_GetToken(input, sizeof(input), &read, &value, NULL));
    TEST_ASSERT_EQUAL(strlen(input), read);
}

void test_BLINK_GetToken_rbracket(void)
{
    size_t read;
    const char input[] = "]";
    union blink_token_value value;
    enum blink_token expected = TOK_RBRACKET;

    TEST_ASSERT_EQUAL(expected, BLINK_GetToken(input, sizeof(input), &read, &value, NULL));
    TEST_ASSERT_EQUAL(strlen(input), read);
}

void test_BLINK_GetToken_lparen(void)
{
    size_t read;
    const char input[] = "(";
    union blink_token_value value;
    enum blink_token expected = TOK_LPAREN;

    TEST_ASSERT_EQUAL(expected, BLINK_GetToken(input, sizeof(input), &read, &value, NULL));
    TEST_ASSERT_EQUAL(strlen(input), read);
}

void test_BLINK_GetToken_rparen(void)
{
    size_t read;
    const char input[] = ")";
    union blink_token_value value;
    enum blink_token expected = TOK_RPAREN;

    TEST_ASSERT_EQUAL(expected, BLINK_GetToken(input, sizeof(input), &read, &value, NULL));
    TEST_ASSERT_EQUAL(strlen(input), read);
}

void test_BLINK_GetToken_colon(void)
{
    size_t read;
    const char input[] = ":";
    union blink_token_value value;
    enum blink_token expected = TOK_COLON;

    TEST_ASSERT_EQUAL(expected, BLINK_GetToken(input, sizeof(input), &read, &value, NULL));
    TEST_ASSERT_EQUAL(strlen(input), read);
}

void test_BLINK_GetToken_slash(void)
{
    size_t read;
    const char input[] = "/";
    union blink_token_value value;
    enum blink_token expected = TOK_SLASH;

    TEST_ASSERT_EQUAL(expected, BLINK_GetToken(input, sizeof(input), &read, &value, NULL));
    TEST_ASSERT_EQUAL(strlen(input), read);
}

void test_BLINK_GetToken_question(void)
{
    size_t read;
    const char input[] = "?";
    union blink_token_value value;
    enum blink_token expected = TOK_QUESTION;

    TEST_ASSERT_EQUAL(expected, BLINK_GetToken(input, sizeof(input), &read, &value, NULL));
    TEST_ASSERT_EQUAL(strlen(input), read);
}

void test_BLINK_GetToken_at(void)
{
    size_t read;
    const char input[] = "@";
    union blink_token_value value;
    enum blink_token expected = TOK_AT;

    TEST_ASSERT_EQUAL(expected, BLINK_GetToken(input, sizeof(input), &read, &value, NULL));
    TEST_ASSERT_EQUAL(strlen(input), read);
}

void test_BLINK_GetToken_bar(void)
{
    size_t read;
    const char input[] = "|";
    union blink_token_value value;
    enum blink_token expected = TOK_BAR;

    TEST_ASSERT_EQUAL(expected, BLINK_GetToken(input, sizeof(input), &read, &value, NULL));
    TEST_ASSERT_EQUAL(strlen(input), read);
}

void test_BLINK_GetToken_number(void)
{
    size_t read;
    const char input[] = "42";
    union blink_token_value value;
    enum blink_token expected = TOK_NUMBER;

    TEST_ASSERT_EQUAL(expected, BLINK_GetToken(input, sizeof(input), &read, &value, NULL));
    TEST_ASSERT_EQUAL(strlen(input), read);
    TEST_ASSERT_EQUAL(42, value.number);
}

void test_BLINK_GetToken_hex_number(void)
{
    size_t read;
    const char input[] = "0x2A";
    union blink_token_value value;
    enum blink_token expected = TOK_NUMBER;

    TEST_ASSERT_EQUAL(expected, BLINK_GetToken(input, sizeof(input), &read, &value, NULL));
    TEST_ASSERT_EQUAL(strlen(input), read);
    TEST_ASSERT_EQUAL(42, value.number);
}

void test_BLINK_GetToken_hex_number_too_big(void)
{
    size_t read;
    const char input[] = "0x10000000000000000";
    union blink_token_value value;
    enum blink_token expected = TOK_UNKNOWN;

    TEST_ASSERT_EQUAL(expected, BLINK_GetToken(input, sizeof(input), &read, &value, NULL));
}

void test_BLINK_GetToken_comment_eof(void)
{
    size_t read;
    const char input[] = "# this is a comment";
    union blink_token_value value;
    enum blink_token expected = TOK_EOF;

    TEST_ASSERT_EQUAL(expected, BLINK_GetToken(input, sizeof(input), &read, &value, NULL));
}

void test_BLINK_GetToken_comment_u8(void)
{
    size_t read;
    const char input[] = "# this is a comment\nu8";
    union blink_token_value value;
    enum blink_token expected = TOK_U8;

    TEST_ASSERT_EQUAL(expected, BLINK_GetToken(input, sizeof(input), &read, &value, NULL));
}

void test_BLINK_GetToken_literal_doubleQuote(void)
{
    size_t read;
    const char input[] = "\"this is a literal\"";
    union blink_token_value value;
    enum blink_token expected = TOK_LITERAL;

    TEST_ASSERT_EQUAL(expected, BLINK_GetToken(input, sizeof(input), &read, &value, NULL));
    TEST_ASSERT_EQUAL(strlen(input), read);
    TEST_ASSERT_EQUAL(strlen("this is a literal"), value.literal.len);
    TEST_ASSERT_EQUAL_MEMORY("this is a literal", value.literal.ptr, value.literal.len);
}

void test_BLINK_GetToken_literal_singleQuote(void)
{
    size_t read;
    const char input[] = "'this is a literal'";
    union blink_token_value value;
    enum blink_token expected = TOK_LITERAL;

    TEST_ASSERT_EQUAL(expected, BLINK_GetToken(input, sizeof(input), &read, &value, NULL));
    TEST_ASSERT_EQUAL(strlen(input), read);
    TEST_ASSERT_EQUAL(strlen("this is a literal"), value.literal.len);
    TEST_ASSERT_EQUAL_MEMORY("this is a literal", value.literal.ptr, value.literal.len);
}

void test_BLINK_GetToken_literal_mixedQuote(void)
{
    size_t read;
    const char input[] = "'this is a literal\"";
    union blink_token_value value;
    enum blink_token expected = TOK_UNKNOWN;

    TEST_ASSERT_EQUAL(expected, BLINK_GetToken(input, sizeof(input), &read, &value, NULL));    
}

void test_BLINK_GetToken_literal_newline(void)
{
    size_t read;
    const char input[] = "'this is a\nliteral'";
    union blink_token_value value;
    enum blink_token expected = TOK_UNKNOWN;

    TEST_ASSERT_EQUAL(expected, BLINK_GetToken(input, sizeof(input), &read, &value, NULL));    
}


