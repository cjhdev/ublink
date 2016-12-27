/**
 * @example __FILE__
 *
 * */

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>

#include "cmocka.h"
#include "blink_lexer.h"
#include <string.h>

void test_BLINK_Lexer_getToken_name(void **user)
{
    size_t read;
    const char input[] = "test";
    union blink_token_value value;
    enum blink_token expected = TOK_NAME;

    assert_int_equal(expected, BLINK_Lexer_getToken(input, sizeof(input), &read, &value, NULL));
    assert_int_equal(strlen(input), read);
    assert_int_equal(strlen(input), value.literal.len);
    assert_memory_equal(input, value.literal.ptr, value.literal.len);
}

void test_BLINK_Lexer_getToken_name_leadingWS(void **user)
{
    size_t read;
    const char input[] = " test";
    union blink_token_value value;
    enum blink_token expected = TOK_NAME;

    assert_int_equal(expected, BLINK_Lexer_getToken(input, sizeof(input), &read, &value, NULL));
    assert_int_equal(strlen(" test"), read);
    assert_int_equal(strlen("test"), value.literal.len);
    assert_memory_equal("test", value.literal.ptr, value.literal.len);
}

void test_BLINK_Lexer_getToken_name_leadingWS_trailingWS(void **user)
{
    size_t read;
    const char input[] = " test ";
    union blink_token_value value;
    enum blink_token expected = TOK_NAME;

    assert_int_equal(expected, BLINK_Lexer_getToken(input, sizeof(input), &read, &value, NULL));
    assert_int_equal(strlen(" test"), read);
    assert_int_equal(strlen("test"), value.literal.len);
    assert_memory_equal("test", value.literal.ptr, value.literal.len);
}

void test_BLINK_Lexer_getToken_double_name(void **user)
{
    size_t read;
    size_t readAgain;
    const char input[] = "test again";
    union blink_token_value value;
    enum blink_token expected = TOK_NAME;

    assert_int_equal(expected, BLINK_Lexer_getToken(input, sizeof(input), &read, &value, NULL));

    assert_int_equal(strlen("test"), read);
    assert_int_equal(strlen("test"), value.literal.len);
    assert_memory_equal("test", value.literal.ptr, value.literal.len);

    assert_int_equal(expected, BLINK_Lexer_getToken(&input[read], sizeof(input) - read, &readAgain, &value, NULL));

    assert_int_equal(strlen(" again"), readAgain);
    assert_int_equal(strlen("again"), value.literal.len);
    assert_memory_equal("again", value.literal.ptr, value.literal.len);
}

void test_BLINK_Lexer_getToken_escaped_name(void **user)
{
    size_t read;
    const char input[] = "\\u8";
    union blink_token_value value;
    enum blink_token expected = TOK_NAME;

    assert_int_equal(expected, BLINK_Lexer_getToken(input, sizeof(input), &read, &value, NULL));
    assert_int_equal(strlen(input), read);
    assert_int_equal(strlen(input)-1, value.literal.len);
    assert_memory_equal(&input[1], value.literal.ptr, value.literal.len);
}

void test_BLINK_Lexer_getToken_namespace(void **user)
{
    size_t read;
    const char input[] = "namespace";
    union blink_token_value value;
    enum blink_token expected = TOK_NAMESPACE;

    assert_int_equal(expected, BLINK_Lexer_getToken(input, sizeof(input), &read, &value, NULL));
    assert_int_equal(strlen(input), read);
}

void test_BLINK_Lexer_getToken_u8(void **user)
{
    size_t read;
    const char input[] = "u8";
    union blink_token_value value;
    enum blink_token expected = TOK_U8;

    assert_int_equal(expected, BLINK_Lexer_getToken(input, sizeof(input), &read, &value, NULL));
    assert_int_equal(strlen(input), read);
}

void test_BLINK_Lexer_getToken_u16(void **user)
{
    size_t read;
    const char input[] = "u16";
    union blink_token_value value;
    enum blink_token expected = TOK_U16;

    assert_int_equal(expected, BLINK_Lexer_getToken(input, sizeof(input), &read, &value, NULL));
    assert_int_equal(strlen(input), read);
}

void test_BLINK_Lexer_getToken_u32(void **user)
{
    size_t read;
    const char input[] = "u32";
    union blink_token_value value;
    enum blink_token expected = TOK_U32;

    assert_int_equal(expected, BLINK_Lexer_getToken(input, sizeof(input), &read, &value, NULL));
    assert_int_equal(strlen(input), read);
}

void test_BLINK_Lexer_getToken_u64(void **user)
{
    size_t read;
    const char input[] = "u64";
    union blink_token_value value;
    enum blink_token expected = TOK_U64;

    assert_int_equal(expected, BLINK_Lexer_getToken(input, sizeof(input), &read, &value, NULL));
    assert_int_equal(strlen(input), read);
}

void test_BLINK_Lexer_getToken_i8(void **user)
{
    size_t read;
    const char input[] = "i8";
    union blink_token_value value;
    enum blink_token expected = TOK_I8;

    assert_int_equal(expected, BLINK_Lexer_getToken(input, sizeof(input), &read, &value, NULL));
    assert_int_equal(strlen(input), read);
}

void test_BLINK_Lexer_getToken_i16(void **user)
{
    size_t read;
    const char input[] = "i16";
    union blink_token_value value;
    enum blink_token expected = TOK_I16;

    assert_int_equal(expected, BLINK_Lexer_getToken(input, sizeof(input), &read, &value, NULL));
    assert_int_equal(strlen(input), read);
}

void test_BLINK_Lexer_getToken_i32(void **user)
{
    size_t read;
    const char input[] = "i32";
    union blink_token_value value;
    enum blink_token expected = TOK_I32;

    assert_int_equal(expected, BLINK_Lexer_getToken(input, sizeof(input), &read, &value, NULL));
    assert_int_equal(strlen(input), read);
}

void test_BLINK_Lexer_getToken_i64(void **user)
{
    size_t read;
    const char input[] = "i64";
    union blink_token_value value;
    enum blink_token expected = TOK_I64;

    assert_int_equal(expected, BLINK_Lexer_getToken(input, sizeof(input), &read, &value, NULL));
    assert_int_equal(strlen(input), read);
}

void test_BLINK_Lexer_getToken_f64(void **user)
{
    size_t read;
    const char input[] = "f64";
    union blink_token_value value;
    enum blink_token expected = TOK_F64;

    assert_int_equal(expected, BLINK_Lexer_getToken(input, sizeof(input), &read, &value, NULL));
    assert_int_equal(strlen(input), read);
}

void test_BLINK_Lexer_getToken_string(void **user)
{
    size_t read;
    const char input[] = "string";
    union blink_token_value value;
    enum blink_token expected = TOK_STRING;

    assert_int_equal(expected, BLINK_Lexer_getToken(input, sizeof(input), &read, &value, NULL));
    assert_int_equal(strlen(input), read);
}

void test_BLINK_Lexer_getToken_binary(void **user)
{
    size_t read;
    const char input[] = "binary";
    union blink_token_value value;
    enum blink_token expected = TOK_BINARY;

    assert_int_equal(expected, BLINK_Lexer_getToken(input, sizeof(input), &read, &value, NULL));
    assert_int_equal(strlen(input), read);
}

void test_BLINK_Lexer_getToken_fixed(void **user)
{
    size_t read;
    const char input[] = "fixed";
    union blink_token_value value;
    enum blink_token expected = TOK_FIXED;

    assert_int_equal(expected, BLINK_Lexer_getToken(input, sizeof(input), &read, &value, NULL));
    assert_int_equal(strlen(input), read);
}

void test_BLINK_Lexer_getToken_decimal(void **user)
{
    size_t read;
    const char input[] = "decimal";
    union blink_token_value value;
    enum blink_token expected = TOK_DECIMAL;

    assert_int_equal(expected, BLINK_Lexer_getToken(input, sizeof(input), &read, &value, NULL));
    assert_int_equal(strlen(input), read);
}

void test_BLINK_Lexer_getToken_date(void **user)
{
    size_t read;
    const char input[] = "date";
    union blink_token_value value;
    enum blink_token expected = TOK_DATE;

    assert_int_equal(expected, BLINK_Lexer_getToken(input, sizeof(input), &read, &value, NULL));
    assert_int_equal(strlen(input), read);
}

void test_BLINK_Lexer_getToken_millitime(void **user)
{
    size_t read;
    const char input[] = "millitime";
    union blink_token_value value;
    enum blink_token expected = TOK_MILLI_TIME;

    assert_int_equal(expected, BLINK_Lexer_getToken(input, sizeof(input), &read, &value, NULL));
    assert_int_equal(strlen(input), read);
}

void test_BLINK_Lexer_getToken_nanotime(void **user)
{
    size_t read;
    const char input[] = "nanotime";
    union blink_token_value value;
    enum blink_token expected = TOK_NANO_TIME;

    assert_int_equal(expected, BLINK_Lexer_getToken(input, sizeof(input), &read, &value, NULL));
    assert_int_equal(strlen(input), read);
}

void test_BLINK_Lexer_getToken_timeOfDayMilli(void **user)
{
    size_t read;
    const char input[] = "timeOfDayMilli";
    union blink_token_value value;
    enum blink_token expected = TOK_TIME_OF_DAY_MILLI;

    assert_int_equal(expected, BLINK_Lexer_getToken(input, sizeof(input), &read, &value, NULL));
    assert_int_equal(strlen(input), read);
}

void test_BLINK_Lexer_getToken_timeOfDayNano(void **user)
{
    size_t read;
    const char input[] = "timeOfDayNano";
    union blink_token_value value;
    enum blink_token expected = TOK_TIME_OF_DAY_NANO;

    assert_int_equal(expected, BLINK_Lexer_getToken(input, sizeof(input), &read, &value, NULL));
    assert_int_equal(strlen(input), read);
}

void test_BLINK_Lexer_getToken_schema(void **user)
{
    size_t read;
    const char input[] = "schema";
    union blink_token_value value;
    enum blink_token expected = TOK_SCHEMA;

    assert_int_equal(expected, BLINK_Lexer_getToken(input, sizeof(input), &read, &value, NULL));
    assert_int_equal(strlen(input), read);
}

void test_BLINK_Lexer_getToken_type(void **user)
{
    size_t read;
    const char input[] = "type";
    union blink_token_value value;
    enum blink_token expected = TOK_TYPE;

    assert_int_equal(expected, BLINK_Lexer_getToken(input, sizeof(input), &read, &value, NULL));
    assert_int_equal(strlen(input), read);
}

void test_BLINK_Lexer_getToken_rarrow(void **user)
{
    size_t read;
    const char input[] = "->";
    union blink_token_value value;
    enum blink_token expected = TOK_RARROW;

    assert_int_equal(expected, BLINK_Lexer_getToken(input, sizeof(input), &read, &value, NULL));
    assert_int_equal(strlen(input), read);
}

void test_BLINK_Lexer_getToken_larrow(void **user)
{
    size_t read;
    const char input[] = "<-";
    union blink_token_value value;
    enum blink_token expected = TOK_LARROW;

    assert_int_equal(expected, BLINK_Lexer_getToken(input, sizeof(input), &read, &value, NULL));
    assert_int_equal(strlen(input), read);
}

void test_BLINK_Lexer_getToken_bool(void **user)
{
    size_t read;
    const char input[] = "bool";
    union blink_token_value value;
    enum blink_token expected = TOK_BOOL;

    assert_int_equal(expected, BLINK_Lexer_getToken(input, sizeof(input), &read, &value, NULL));
    assert_int_equal(strlen(input), read);
}

void test_BLINK_Lexer_getToken_star(void **user)
{
    size_t read;
    const char input[] = "*";
    union blink_token_value value;
    enum blink_token expected = TOK_STAR;

    assert_int_equal(expected, BLINK_Lexer_getToken(input, sizeof(input), &read, &value, NULL));
    assert_int_equal(strlen(input), read);
}

void test_BLINK_Lexer_getToken_equal(void **user)
{
    size_t read;
    const char input[] = "=";
    union blink_token_value value;
    enum blink_token expected = TOK_EQUAL;

    assert_int_equal(expected, BLINK_Lexer_getToken(input, sizeof(input), &read, &value, NULL));
    assert_int_equal(strlen(input), read);
}

void test_BLINK_Lexer_getToken_period(void **user)
{
    size_t read;
    const char input[] = ".";
    union blink_token_value value;
    enum blink_token expected = TOK_PERIOD;

    assert_int_equal(expected, BLINK_Lexer_getToken(input, sizeof(input), &read, &value, NULL));
    assert_int_equal(strlen(input), read);
}

void test_BLINK_Lexer_getToken_comma(void **user)
{
    size_t read;
    const char input[] = ",";
    union blink_token_value value;
    enum blink_token expected = TOK_COMMA;

    assert_int_equal(expected, BLINK_Lexer_getToken(input, sizeof(input), &read, &value, NULL));
    assert_int_equal(strlen(input), read);
}

void test_BLINK_Lexer_getToken_lbracket(void **user)
{
    size_t read;
    const char input[] = "[";
    union blink_token_value value;
    enum blink_token expected = TOK_LBRACKET;

    assert_int_equal(expected, BLINK_Lexer_getToken(input, sizeof(input), &read, &value, NULL));
    assert_int_equal(strlen(input), read);
}

void test_BLINK_Lexer_getToken_rbracket(void **user)
{
    size_t read;
    const char input[] = "]";
    union blink_token_value value;
    enum blink_token expected = TOK_RBRACKET;

    assert_int_equal(expected, BLINK_Lexer_getToken(input, sizeof(input), &read, &value, NULL));
    assert_int_equal(strlen(input), read);
}

void test_BLINK_Lexer_getToken_lparen(void **user)
{
    size_t read;
    const char input[] = "(";
    union blink_token_value value;
    enum blink_token expected = TOK_LPAREN;

    assert_int_equal(expected, BLINK_Lexer_getToken(input, sizeof(input), &read, &value, NULL));
    assert_int_equal(strlen(input), read);
}

void test_BLINK_Lexer_getToken_rparen(void **user)
{
    size_t read;
    const char input[] = ")";
    union blink_token_value value;
    enum blink_token expected = TOK_RPAREN;

    assert_int_equal(expected, BLINK_Lexer_getToken(input, sizeof(input), &read, &value, NULL));
    assert_int_equal(strlen(input), read);
}

void test_BLINK_Lexer_getToken_colon(void **user)
{
    size_t read;
    const char input[] = ":";
    union blink_token_value value;
    enum blink_token expected = TOK_COLON;

    assert_int_equal(expected, BLINK_Lexer_getToken(input, sizeof(input), &read, &value, NULL));
    assert_int_equal(strlen(input), read);
}

void test_BLINK_Lexer_getToken_slash(void **user)
{
    size_t read;
    const char input[] = "/";
    union blink_token_value value;
    enum blink_token expected = TOK_SLASH;

    assert_int_equal(expected, BLINK_Lexer_getToken(input, sizeof(input), &read, &value, NULL));
    assert_int_equal(strlen(input), read);
}

void test_BLINK_Lexer_getToken_question(void **user)
{
    size_t read;
    const char input[] = "?";
    union blink_token_value value;
    enum blink_token expected = TOK_QUESTION;

    assert_int_equal(expected, BLINK_Lexer_getToken(input, sizeof(input), &read, &value, NULL));
    assert_int_equal(strlen(input), read);
}

void test_BLINK_Lexer_getToken_at(void **user)
{
    size_t read;
    const char input[] = "@";
    union blink_token_value value;
    enum blink_token expected = TOK_AT;

    assert_int_equal(expected, BLINK_Lexer_getToken(input, sizeof(input), &read, &value, NULL));
    assert_int_equal(strlen(input), read);
}

void test_BLINK_Lexer_getToken_bar(void **user)
{
    size_t read;
    const char input[] = "|";
    union blink_token_value value;
    enum blink_token expected = TOK_BAR;

    assert_int_equal(expected, BLINK_Lexer_getToken(input, sizeof(input), &read, &value, NULL));
    assert_int_equal(strlen(input), read);
}

void test_BLINK_Lexer_getToken_unsigned_number(void **user)
{
    size_t read;
    const char input[] = "42";
    union blink_token_value value;
    enum blink_token expected = TOK_UINT;

    assert_int_equal(expected, BLINK_Lexer_getToken(input, sizeof(input), &read, &value, NULL));
    assert_int_equal(strlen(input), read);
    assert_int_equal(42, value.number);
}

void test_BLINK_Lexer_getToken_signed_number(void **user)
{
    size_t read;
    const char input[] = "-42";
    union blink_token_value value;
    enum blink_token expected = TOK_INT;

    assert_int_equal(expected, BLINK_Lexer_getToken(input, sizeof(input), &read, &value, NULL));
    assert_int_equal(strlen(input), read);
    assert_int_equal(-42, value.signedNumber);
}

void test_BLINK_Lexer_getToken_hex_number(void **user)
{
    size_t read;
    const char input[] = "0x2A";
    union blink_token_value value;
    enum blink_token expected = TOK_UINT;

    assert_int_equal(expected, BLINK_Lexer_getToken(input, sizeof(input), &read, &value, NULL));
    assert_int_equal(strlen(input), read);
    assert_int_equal(42, value.number);
}

void test_BLINK_Lexer_getToken_hex_number_too_big(void **user)
{
    size_t read;
    const char input[] = "0x10000000000000000";
    union blink_token_value value;
    enum blink_token expected = TOK_UNKNOWN;

    assert_int_equal(expected, BLINK_Lexer_getToken(input, sizeof(input), &read, &value, NULL));
}

void test_BLINK_Lexer_getToken_comment_eof(void **user)
{
    size_t read;
    const char input[] = "# this is a comment";
    union blink_token_value value;
    enum blink_token expected = TOK_EOF;

    assert_int_equal(expected, BLINK_Lexer_getToken(input, sizeof(input), &read, &value, NULL));
}

void test_BLINK_Lexer_getToken_comment_u8(void **user)
{
    size_t read;
    const char input[] = "# this is a comment\nu8";
    union blink_token_value value;
    enum blink_token expected = TOK_U8;

    assert_int_equal(expected, BLINK_Lexer_getToken(input, sizeof(input), &read, &value, NULL));
}

void test_BLINK_Lexer_getToken_literal_doubleQuote(void **user)
{
    size_t read;
    const char input[] = "\"this is a literal\"";
    union blink_token_value value;
    enum blink_token expected = TOK_LITERAL;

    assert_int_equal(expected, BLINK_Lexer_getToken(input, sizeof(input), &read, &value, NULL));
    assert_int_equal(strlen(input), read);
    assert_int_equal(strlen("this is a literal"), value.literal.len);
    assert_memory_equal("this is a literal", value.literal.ptr, value.literal.len);
}

void test_BLINK_Lexer_getToken_literal_singleQuote(void **user)
{
    size_t read;
    const char input[] = "'this is a literal'";
    union blink_token_value value;
    enum blink_token expected = TOK_LITERAL;

    assert_int_equal(expected, BLINK_Lexer_getToken(input, sizeof(input), &read, &value, NULL));
    assert_int_equal(strlen(input), read);
    assert_int_equal(strlen("this is a literal"), value.literal.len);
    assert_memory_equal("this is a literal", value.literal.ptr, value.literal.len);
}

void test_BLINK_Lexer_getToken_literal_mixedQuote(void **user)
{
    size_t read;
    const char input[] = "'this is a literal\"";
    union blink_token_value value;
    enum blink_token expected = TOK_UNKNOWN;

    assert_int_equal(expected, BLINK_Lexer_getToken(input, sizeof(input), &read, &value, NULL));    
}

void test_BLINK_Lexer_getToken_literal_newline(void **user)
{
    size_t read;
    const char input[] = "'this is a\nliteral'";
    union blink_token_value value;
    enum blink_token expected = TOK_UNKNOWN;

    assert_int_equal(expected, BLINK_Lexer_getToken(input, sizeof(input), &read, &value, NULL));    
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_BLINK_Lexer_getToken_name),
        cmocka_unit_test(test_BLINK_Lexer_getToken_name_leadingWS),
        cmocka_unit_test(test_BLINK_Lexer_getToken_name_leadingWS_trailingWS),
        cmocka_unit_test(test_BLINK_Lexer_getToken_double_name),
        cmocka_unit_test(test_BLINK_Lexer_getToken_escaped_name),
        cmocka_unit_test(test_BLINK_Lexer_getToken_namespace),
        cmocka_unit_test(test_BLINK_Lexer_getToken_u8),
        cmocka_unit_test(test_BLINK_Lexer_getToken_u16),
        cmocka_unit_test(test_BLINK_Lexer_getToken_u32),
        cmocka_unit_test(test_BLINK_Lexer_getToken_u64),
        cmocka_unit_test(test_BLINK_Lexer_getToken_i8),
        cmocka_unit_test(test_BLINK_Lexer_getToken_i16),
        cmocka_unit_test(test_BLINK_Lexer_getToken_i32),
        cmocka_unit_test(test_BLINK_Lexer_getToken_i64),
        cmocka_unit_test(test_BLINK_Lexer_getToken_f64),
        cmocka_unit_test(test_BLINK_Lexer_getToken_string),
        cmocka_unit_test(test_BLINK_Lexer_getToken_binary),
        cmocka_unit_test(test_BLINK_Lexer_getToken_fixed),
        cmocka_unit_test(test_BLINK_Lexer_getToken_decimal),
        cmocka_unit_test(test_BLINK_Lexer_getToken_date),
        cmocka_unit_test(test_BLINK_Lexer_getToken_millitime),
        cmocka_unit_test(test_BLINK_Lexer_getToken_nanotime),
        cmocka_unit_test(test_BLINK_Lexer_getToken_timeOfDayMilli),
        cmocka_unit_test(test_BLINK_Lexer_getToken_timeOfDayNano),
        cmocka_unit_test(test_BLINK_Lexer_getToken_schema),
        cmocka_unit_test(test_BLINK_Lexer_getToken_type),
        cmocka_unit_test(test_BLINK_Lexer_getToken_rarrow),
        cmocka_unit_test(test_BLINK_Lexer_getToken_larrow),
        cmocka_unit_test(test_BLINK_Lexer_getToken_bool),
        cmocka_unit_test(test_BLINK_Lexer_getToken_star),
        cmocka_unit_test(test_BLINK_Lexer_getToken_equal),
        cmocka_unit_test(test_BLINK_Lexer_getToken_period),
        cmocka_unit_test(test_BLINK_Lexer_getToken_comma),
        cmocka_unit_test(test_BLINK_Lexer_getToken_lbracket),
        cmocka_unit_test(test_BLINK_Lexer_getToken_rbracket),
        cmocka_unit_test(test_BLINK_Lexer_getToken_lparen),
        cmocka_unit_test(test_BLINK_Lexer_getToken_rparen),
        cmocka_unit_test(test_BLINK_Lexer_getToken_colon),
        cmocka_unit_test(test_BLINK_Lexer_getToken_slash),
        cmocka_unit_test(test_BLINK_Lexer_getToken_question),
        cmocka_unit_test(test_BLINK_Lexer_getToken_at),
        cmocka_unit_test(test_BLINK_Lexer_getToken_bar),
        cmocka_unit_test(test_BLINK_Lexer_getToken_unsigned_number),
        cmocka_unit_test(test_BLINK_Lexer_getToken_signed_number),
        cmocka_unit_test(test_BLINK_Lexer_getToken_hex_number),
        cmocka_unit_test(test_BLINK_Lexer_getToken_hex_number_too_big),
        cmocka_unit_test(test_BLINK_Lexer_getToken_comment_eof),
        cmocka_unit_test(test_BLINK_Lexer_getToken_comment_u8),
        cmocka_unit_test(test_BLINK_Lexer_getToken_literal_doubleQuote),
        cmocka_unit_test(test_BLINK_Lexer_getToken_literal_singleQuote),
        cmocka_unit_test(test_BLINK_Lexer_getToken_literal_mixedQuote),
        cmocka_unit_test(test_BLINK_Lexer_getToken_literal_newline)
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
