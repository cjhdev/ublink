/**
 * @example tc_blink_lexer_gettoken.c
 *
 * */

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>

#include "cmocka.h"
#include "blink_lexer.h"
#include "blink_stream.h"
#include <string.h>

static char buffer[50U];    

static void test_BLINK_Lexer_getToken_name(void **user)
{
    const char input[] = "test";
    
    union blink_token_value value;
    enum blink_token expected = TOK_NAME;
    struct blink_stream stream;
    (void)BLINK_Stream_initBufferReadOnly(&stream, (const uint8_t *)input, sizeof(input));

    assert_int_equal(expected, BLINK_Lexer_getToken(&stream, buffer, sizeof(buffer), &value, NULL));
    assert_int_equal(strlen(input), value.literal.len);
    assert_memory_equal(input, value.literal.ptr, value.literal.len);
}

static void test_BLINK_Lexer_getToken_name_leadingWS(void **user)
{
    const char input[] = " test";
    union blink_token_value value;
    enum blink_token expected = TOK_NAME;
    struct blink_stream stream;
    (void)BLINK_Stream_initBufferReadOnly(&stream, (const uint8_t *)input, sizeof(input));

    assert_int_equal(expected, BLINK_Lexer_getToken(&stream, buffer, sizeof(buffer), &value, NULL));
    assert_int_equal(strlen("test"), value.literal.len);
    assert_memory_equal("test", value.literal.ptr, value.literal.len);
}

static void test_BLINK_Lexer_getToken_name_leadingWS_trailingWS(void **user)
{
    const char input[] = " test ";
    union blink_token_value value;
    enum blink_token expected = TOK_NAME;
    struct blink_stream stream;
    (void)BLINK_Stream_initBufferReadOnly(&stream, (const uint8_t *)input, sizeof(input));

    assert_int_equal(expected, BLINK_Lexer_getToken(&stream, buffer, sizeof(buffer), &value, NULL));
    assert_int_equal(strlen("test"), value.literal.len);
    assert_memory_equal("test", value.literal.ptr, value.literal.len);
}

static void test_BLINK_Lexer_getToken_double_name(void **user)
{
    const char input[] = "test again";
    union blink_token_value value;
    enum blink_token expected = TOK_NAME;
    struct blink_stream stream;
    (void)BLINK_Stream_initBufferReadOnly(&stream, (const uint8_t *)input, sizeof(input));

    assert_int_equal(expected, BLINK_Lexer_getToken(&stream, buffer, sizeof(buffer), &value, NULL));

    assert_int_equal(strlen("test"), value.literal.len);
    assert_memory_equal("test", value.literal.ptr, value.literal.len);

    assert_int_equal(expected, BLINK_Lexer_getToken(&stream, buffer, sizeof(buffer), &value, NULL));

    assert_int_equal(strlen("again"), value.literal.len);
    assert_memory_equal("again", value.literal.ptr, value.literal.len);
}

static void test_BLINK_Lexer_getToken_escaped_name(void **user)
{
    const char input[] = "\\u8";
    union blink_token_value value;
    enum blink_token expected = TOK_NAME;
    struct blink_stream stream;
    (void)BLINK_Stream_initBufferReadOnly(&stream, (const uint8_t *)input, sizeof(input));

    assert_int_equal(expected, BLINK_Lexer_getToken(&stream, buffer, sizeof(buffer), &value, NULL));
    assert_int_equal(strlen(input)-1, value.literal.len);
    assert_memory_equal(&input[1], value.literal.ptr, value.literal.len);
}

static void test_BLINK_Lexer_getToken_name_tooBig(void **user)
{
    const char input[] =
        "a123456789"
        "0123456789"
        "0123456789"
        "0123456789"
        "0123456789"
        "0";
        
    union blink_token_value value;
    enum blink_token expected = TOK_ENOMEM;
    struct blink_stream stream;
    (void)BLINK_Stream_initBufferReadOnly(&stream, (const uint8_t *)input, sizeof(input));

    assert_int_equal(expected, BLINK_Lexer_getToken(&stream, buffer, sizeof(buffer), &value, NULL));    
}

static void test_BLINK_Lexer_getToken_cname(void **user)
{
    const char input[] = "u8:u";
    union blink_token_value value;
    enum blink_token expected = TOK_CNAME;
    struct blink_stream stream;
    (void)BLINK_Stream_initBufferReadOnly(&stream, (const uint8_t *)input, sizeof(input));

    assert_int_equal(expected, BLINK_Lexer_getToken(&stream, buffer, sizeof(buffer), &value, NULL));
    assert_int_equal(strlen(input), value.literal.len);
    assert_memory_equal(input, value.literal.ptr, value.literal.len);
}

static void test_BLINK_Lexer_getToken_cname_tooBig_beforeColon(void **user)
{
    const char input[] =
        "a123456789"
        "0123456789"
        "0123456789"
        "0123456789"
        "0123456789"
        "0";
        
    union blink_token_value value;
    enum blink_token expected = TOK_ENOMEM;
    struct blink_stream stream;
    (void)BLINK_Stream_initBufferReadOnly(&stream, (const uint8_t *)input, sizeof(input));

    assert_int_equal(expected, BLINK_Lexer_getToken(&stream, buffer, sizeof(buffer), &value, NULL));    
}

static void test_BLINK_Lexer_getToken_cname_tooBig_afterColon(void **user)
{
    const char input[] =
        "a:123456789"
        "0123456789"
        "0123456789"
        "0123456789"
        "0123456789"
        "0";
        
    union blink_token_value value;
    enum blink_token expected = TOK_ENOMEM;
    struct blink_stream stream;
    (void)BLINK_Stream_initBufferReadOnly(&stream, (const uint8_t *)input, sizeof(input));

    assert_int_equal(expected, BLINK_Lexer_getToken(&stream, buffer, sizeof(buffer), &value, NULL));    
}

static void test_BLINK_Lexer_getToken_namespace(void **user)
{
    const char input[] = "namespace";
    union blink_token_value value;
    enum blink_token expected = TOK_NAMESPACE;
    struct blink_stream stream;
    (void)BLINK_Stream_initBufferReadOnly(&stream, (const uint8_t *)input, sizeof(input));

    assert_int_equal(expected, BLINK_Lexer_getToken(&stream, buffer, sizeof(buffer), &value, NULL));
}

static void test_BLINK_Lexer_getToken_u8(void **user)
{
    const char input[] = "u8";
    union blink_token_value value;
    enum blink_token expected = TOK_U8;
    struct blink_stream stream;
    (void)BLINK_Stream_initBufferReadOnly(&stream, (const uint8_t *)input, sizeof(input));

    assert_int_equal(expected, BLINK_Lexer_getToken(&stream, buffer, sizeof(buffer), &value, NULL));
}

static void test_BLINK_Lexer_getToken_u16(void **user)
{
    const char input[] = "u16";
    union blink_token_value value;
    enum blink_token expected = TOK_U16;
    struct blink_stream stream;
    (void)BLINK_Stream_initBufferReadOnly(&stream, (const uint8_t *)input, sizeof(input));

    assert_int_equal(expected, BLINK_Lexer_getToken(&stream, buffer, sizeof(buffer), &value, NULL));
}

static void test_BLINK_Lexer_getToken_u32(void **user)
{
    const char input[] = "u32";
    union blink_token_value value;
    enum blink_token expected = TOK_U32;
    struct blink_stream stream;
    (void)BLINK_Stream_initBufferReadOnly(&stream, (const uint8_t *)input, sizeof(input));

    assert_int_equal(expected, BLINK_Lexer_getToken(&stream, buffer, sizeof(buffer), &value, NULL));
}

static void test_BLINK_Lexer_getToken_u64(void **user)
{
    const char input[] = "u64";
    union blink_token_value value;
    enum blink_token expected = TOK_U64;
    struct blink_stream stream;
    (void)BLINK_Stream_initBufferReadOnly(&stream, (const uint8_t *)input, sizeof(input));

    assert_int_equal(expected, BLINK_Lexer_getToken(&stream, buffer, sizeof(buffer), &value, NULL));
}

static void test_BLINK_Lexer_getToken_i8(void **user)
{
    const char input[] = "i8";
    union blink_token_value value;
    enum blink_token expected = TOK_I8;
    struct blink_stream stream;
    (void)BLINK_Stream_initBufferReadOnly(&stream, (const uint8_t *)input, sizeof(input));

    assert_int_equal(expected, BLINK_Lexer_getToken(&stream, buffer, sizeof(buffer), &value, NULL));
}

static void test_BLINK_Lexer_getToken_i16(void **user)
{
    const char input[] = "i16";
    union blink_token_value value;
    enum blink_token expected = TOK_I16;
    struct blink_stream stream;
    (void)BLINK_Stream_initBufferReadOnly(&stream, (const uint8_t *)input, sizeof(input));

    assert_int_equal(expected, BLINK_Lexer_getToken(&stream, buffer, sizeof(buffer), &value, NULL));
}

static void test_BLINK_Lexer_getToken_i32(void **user)
{
    const char input[] = "i32";
    union blink_token_value value;
    enum blink_token expected = TOK_I32;
    struct blink_stream stream;
    (void)BLINK_Stream_initBufferReadOnly(&stream, (const uint8_t *)input, sizeof(input));

    assert_int_equal(expected, BLINK_Lexer_getToken(&stream, buffer, sizeof(buffer), &value, NULL));
}

static void test_BLINK_Lexer_getToken_i64(void **user)
{
    const char input[] = "i64";
    union blink_token_value value;
    enum blink_token expected = TOK_I64;
    struct blink_stream stream;
    (void)BLINK_Stream_initBufferReadOnly(&stream, (const uint8_t *)input, sizeof(input));

    assert_int_equal(expected, BLINK_Lexer_getToken(&stream, buffer, sizeof(buffer), &value, NULL));
}

static void test_BLINK_Lexer_getToken_f64(void **user)
{
    const char input[] = "f64";
    union blink_token_value value;
    enum blink_token expected = TOK_F64;
    struct blink_stream stream;
    (void)BLINK_Stream_initBufferReadOnly(&stream, (const uint8_t *)input, sizeof(input));

    assert_int_equal(expected, BLINK_Lexer_getToken(&stream, buffer, sizeof(buffer), &value, NULL));
}

static void test_BLINK_Lexer_getToken_string(void **user)
{
    const char input[] = "string";
    union blink_token_value value;
    enum blink_token expected = TOK_STRING;
    struct blink_stream stream;
    (void)BLINK_Stream_initBufferReadOnly(&stream, (const uint8_t *)input, sizeof(input));

    assert_int_equal(expected, BLINK_Lexer_getToken(&stream, buffer, sizeof(buffer), &value, NULL));
}

static void test_BLINK_Lexer_getToken_binary(void **user)
{
    const char input[] = "binary";
    union blink_token_value value;
    enum blink_token expected = TOK_BINARY;
    struct blink_stream stream;
    (void)BLINK_Stream_initBufferReadOnly(&stream, (const uint8_t *)input, sizeof(input));

    assert_int_equal(expected, BLINK_Lexer_getToken(&stream, buffer, sizeof(buffer), &value, NULL));
}

static void test_BLINK_Lexer_getToken_fixed(void **user)
{
    const char input[] = "fixed";
    union blink_token_value value;
    enum blink_token expected = TOK_FIXED;
    struct blink_stream stream;
    (void)BLINK_Stream_initBufferReadOnly(&stream, (const uint8_t *)input, sizeof(input));

    assert_int_equal(expected, BLINK_Lexer_getToken(&stream, buffer, sizeof(buffer), &value, NULL));
}

static void test_BLINK_Lexer_getToken_decimal(void **user)
{
    const char input[] = "decimal";
    union blink_token_value value;
    enum blink_token expected = TOK_DECIMAL;
    struct blink_stream stream;
    (void)BLINK_Stream_initBufferReadOnly(&stream, (const uint8_t *)input, sizeof(input));

    assert_int_equal(expected, BLINK_Lexer_getToken(&stream, buffer, sizeof(buffer), &value, NULL));
}

static void test_BLINK_Lexer_getToken_date(void **user)
{
    const char input[] = "date";
    union blink_token_value value;
    enum blink_token expected = TOK_DATE;
    struct blink_stream stream;
    (void)BLINK_Stream_initBufferReadOnly(&stream, (const uint8_t *)input, sizeof(input));

    assert_int_equal(expected, BLINK_Lexer_getToken(&stream, buffer, sizeof(buffer), &value, NULL));
}

static void test_BLINK_Lexer_getToken_millitime(void **user)
{
    const char input[] = "millitime";
    union blink_token_value value;
    enum blink_token expected = TOK_MILLI_TIME;
    struct blink_stream stream;
    (void)BLINK_Stream_initBufferReadOnly(&stream, (const uint8_t *)input, sizeof(input));

    assert_int_equal(expected, BLINK_Lexer_getToken(&stream, buffer, sizeof(buffer), &value, NULL));
}

static void test_BLINK_Lexer_getToken_nanotime(void **user)
{
    const char input[] = "nanotime";
    union blink_token_value value;
    enum blink_token expected = TOK_NANO_TIME;
    struct blink_stream stream;
    (void)BLINK_Stream_initBufferReadOnly(&stream, (const uint8_t *)input, sizeof(input));

    assert_int_equal(expected, BLINK_Lexer_getToken(&stream, buffer, sizeof(buffer), &value, NULL));
}

static void test_BLINK_Lexer_getToken_timeOfDayMilli(void **user)
{
    const char input[] = "timeOfDayMilli";
    union blink_token_value value;
    enum blink_token expected = TOK_TIME_OF_DAY_MILLI;
    struct blink_stream stream;
    (void)BLINK_Stream_initBufferReadOnly(&stream, (const uint8_t *)input, sizeof(input));

    assert_int_equal(expected, BLINK_Lexer_getToken(&stream, buffer, sizeof(buffer), &value, NULL));
}

static void test_BLINK_Lexer_getToken_timeOfDayNano(void **user)
{
    const char input[] = "timeOfDayNano";
    union blink_token_value value;
    enum blink_token expected = TOK_TIME_OF_DAY_NANO;
    struct blink_stream stream;
    (void)BLINK_Stream_initBufferReadOnly(&stream, (const uint8_t *)input, sizeof(input));

    assert_int_equal(expected, BLINK_Lexer_getToken(&stream, buffer, sizeof(buffer), &value, NULL));
}

static void test_BLINK_Lexer_getToken_schema(void **user)
{
    const char input[] = "schema";
    union blink_token_value value;
    enum blink_token expected = TOK_SCHEMA;
    struct blink_stream stream;
    (void)BLINK_Stream_initBufferReadOnly(&stream, (const uint8_t *)input, sizeof(input));

    assert_int_equal(expected, BLINK_Lexer_getToken(&stream, buffer, sizeof(buffer), &value, NULL));
}

static void test_BLINK_Lexer_getToken_type(void **user)
{
    const char input[] = "type";
    union blink_token_value value;
    enum blink_token expected = TOK_TYPE;
    struct blink_stream stream;
    (void)BLINK_Stream_initBufferReadOnly(&stream, (const uint8_t *)input, sizeof(input));

    assert_int_equal(expected, BLINK_Lexer_getToken(&stream, buffer, sizeof(buffer), &value, NULL));
}

static void test_BLINK_Lexer_getToken_rarrow(void **user)
{
    const char input[] = "->";
    union blink_token_value value;
    enum blink_token expected = TOK_RARROW;
    struct blink_stream stream;
    (void)BLINK_Stream_initBufferReadOnly(&stream, (const uint8_t *)input, sizeof(input));

    assert_int_equal(expected, BLINK_Lexer_getToken(&stream, buffer, sizeof(buffer), &value, NULL));
}

static void test_BLINK_Lexer_getToken_larrow(void **user)
{
    const char input[] = "<-";
    union blink_token_value value;
    enum blink_token expected = TOK_LARROW;
    struct blink_stream stream;
    (void)BLINK_Stream_initBufferReadOnly(&stream, (const uint8_t *)input, sizeof(input));

    assert_int_equal(expected, BLINK_Lexer_getToken(&stream, buffer, sizeof(buffer), &value, NULL));
}

static void test_BLINK_Lexer_getToken_bool(void **user)
{
    const char input[] = "bool";
    union blink_token_value value;
    enum blink_token expected = TOK_BOOL;
    struct blink_stream stream;
    (void)BLINK_Stream_initBufferReadOnly(&stream, (const uint8_t *)input, sizeof(input));

    assert_int_equal(expected, BLINK_Lexer_getToken(&stream, buffer, sizeof(buffer), &value, NULL));
    
}

static void test_BLINK_Lexer_getToken_star(void **user)
{
    
    const char input[] = "*";
    union blink_token_value value;
    enum blink_token expected = TOK_STAR;
    struct blink_stream stream;
    (void)BLINK_Stream_initBufferReadOnly(&stream, (const uint8_t *)input, sizeof(input));

    assert_int_equal(expected, BLINK_Lexer_getToken(&stream, buffer, sizeof(buffer), &value, NULL));
    
}

static void test_BLINK_Lexer_getToken_equal(void **user)
{
    
    const char input[] = "=";
    union blink_token_value value;
    enum blink_token expected = TOK_EQUAL;
    struct blink_stream stream;
    (void)BLINK_Stream_initBufferReadOnly(&stream, (const uint8_t *)input, sizeof(input));

    assert_int_equal(expected, BLINK_Lexer_getToken(&stream, buffer, sizeof(buffer), &value, NULL));
    
}

static void test_BLINK_Lexer_getToken_period(void **user)
{
    
    const char input[] = ".";
    union blink_token_value value;
    enum blink_token expected = TOK_PERIOD;
    struct blink_stream stream;
    (void)BLINK_Stream_initBufferReadOnly(&stream, (const uint8_t *)input, sizeof(input));

    assert_int_equal(expected, BLINK_Lexer_getToken(&stream, buffer, sizeof(buffer), &value, NULL));
    
}

static void test_BLINK_Lexer_getToken_comma(void **user)
{
    
    const char input[] = ",";
    union blink_token_value value;
    enum blink_token expected = TOK_COMMA;
    struct blink_stream stream;
    (void)BLINK_Stream_initBufferReadOnly(&stream, (const uint8_t *)input, sizeof(input));

    assert_int_equal(expected, BLINK_Lexer_getToken(&stream, buffer, sizeof(buffer), &value, NULL));
    
}

static void test_BLINK_Lexer_getToken_lbracket(void **user)
{
    
    const char input[] = "[";
    union blink_token_value value;
    enum blink_token expected = TOK_LBRACKET;
    struct blink_stream stream;
    (void)BLINK_Stream_initBufferReadOnly(&stream, (const uint8_t *)input, sizeof(input));

    assert_int_equal(expected, BLINK_Lexer_getToken(&stream, buffer, sizeof(buffer), &value, NULL));
    
}

static void test_BLINK_Lexer_getToken_rbracket(void **user)
{
    
    const char input[] = "]";
    union blink_token_value value;
    enum blink_token expected = TOK_RBRACKET;
    struct blink_stream stream;
    (void)BLINK_Stream_initBufferReadOnly(&stream, (const uint8_t *)input, sizeof(input));

    assert_int_equal(expected, BLINK_Lexer_getToken(&stream, buffer, sizeof(buffer), &value, NULL));
    
}

static void test_BLINK_Lexer_getToken_lparen(void **user)
{
    
    const char input[] = "(";
    union blink_token_value value;
    enum blink_token expected = TOK_LPAREN;
    struct blink_stream stream;
    (void)BLINK_Stream_initBufferReadOnly(&stream, (const uint8_t *)input, sizeof(input));

    assert_int_equal(expected, BLINK_Lexer_getToken(&stream, buffer, sizeof(buffer), &value, NULL));
    
}

static void test_BLINK_Lexer_getToken_rparen(void **user)
{
    
    const char input[] = ")";
    union blink_token_value value;
    enum blink_token expected = TOK_RPAREN;
    struct blink_stream stream;
    (void)BLINK_Stream_initBufferReadOnly(&stream, (const uint8_t *)input, sizeof(input));

    assert_int_equal(expected, BLINK_Lexer_getToken(&stream, buffer, sizeof(buffer), &value, NULL));
    
}

static void test_BLINK_Lexer_getToken_colon(void **user)
{
    
    const char input[] = ":";
    union blink_token_value value;
    enum blink_token expected = TOK_COLON;
    struct blink_stream stream;
    (void)BLINK_Stream_initBufferReadOnly(&stream, (const uint8_t *)input, sizeof(input));

    assert_int_equal(expected, BLINK_Lexer_getToken(&stream, buffer, sizeof(buffer), &value, NULL));
    
}

static void test_BLINK_Lexer_getToken_slash(void **user)
{
    
    const char input[] = "/";
    union blink_token_value value;
    enum blink_token expected = TOK_SLASH;
    struct blink_stream stream;
    (void)BLINK_Stream_initBufferReadOnly(&stream, (const uint8_t *)input, sizeof(input));

    assert_int_equal(expected, BLINK_Lexer_getToken(&stream, buffer, sizeof(buffer), &value, NULL));
    
}

static void test_BLINK_Lexer_getToken_question(void **user)
{
    
    const char input[] = "?";
    union blink_token_value value;
    enum blink_token expected = TOK_QUESTION;
    struct blink_stream stream;
    (void)BLINK_Stream_initBufferReadOnly(&stream, (const uint8_t *)input, sizeof(input));

    assert_int_equal(expected, BLINK_Lexer_getToken(&stream, buffer, sizeof(buffer), &value, NULL));
    
}

static void test_BLINK_Lexer_getToken_at(void **user)
{
    
    const char input[] = "@";
    union blink_token_value value;
    enum blink_token expected = TOK_AT;
    struct blink_stream stream;
    (void)BLINK_Stream_initBufferReadOnly(&stream, (const uint8_t *)input, sizeof(input));

    assert_int_equal(expected, BLINK_Lexer_getToken(&stream, buffer, sizeof(buffer), &value, NULL));
    
}

static void test_BLINK_Lexer_getToken_bar(void **user)
{
    
    const char input[] = "|";
    union blink_token_value value;
    enum blink_token expected = TOK_BAR;
    struct blink_stream stream;
    (void)BLINK_Stream_initBufferReadOnly(&stream, (const uint8_t *)input, sizeof(input));

    assert_int_equal(expected, BLINK_Lexer_getToken(&stream, buffer, sizeof(buffer), &value, NULL));
    
}

static void test_BLINK_Lexer_getToken_unsigned_number(void **user)
{
    
    const char input[] = "42";
    union blink_token_value value;
    enum blink_token expected = TOK_UINT;
    struct blink_stream stream;
    (void)BLINK_Stream_initBufferReadOnly(&stream, (const uint8_t *)input, sizeof(input));

    assert_int_equal(expected, BLINK_Lexer_getToken(&stream, buffer, sizeof(buffer), &value, NULL));
    
    assert_int_equal(42, value.number);
}

static void test_BLINK_Lexer_getToken_signed_number(void **user)
{
    
    const char input[] = "-42";
    union blink_token_value value;
    enum blink_token expected = TOK_INT;
    struct blink_stream stream;
    (void)BLINK_Stream_initBufferReadOnly(&stream, (const uint8_t *)input, sizeof(input));

    assert_int_equal(expected, BLINK_Lexer_getToken(&stream, buffer, sizeof(buffer), &value, NULL));
    
    assert_int_equal(-42, value.signedNumber);
}

static void test_BLINK_Lexer_getToken_hex_number(void **user)
{
    
    const char input[] = "0x2A";
    union blink_token_value value;
    enum blink_token expected = TOK_UINT;
    struct blink_stream stream;
    (void)BLINK_Stream_initBufferReadOnly(&stream, (const uint8_t *)input, sizeof(input));

    assert_int_equal(expected, BLINK_Lexer_getToken(&stream, buffer, sizeof(buffer), &value, NULL));
    
    assert_int_equal(42, value.number);
}

static void test_BLINK_Lexer_getToken_hex_number_too_big(void **user)
{
    
    const char input[] = "0x10000000000000000";
    union blink_token_value value;
    enum blink_token expected = TOK_UNKNOWN;
    struct blink_stream stream;
    (void)BLINK_Stream_initBufferReadOnly(&stream, (const uint8_t *)input, sizeof(input));

    assert_int_equal(expected, BLINK_Lexer_getToken(&stream, buffer, sizeof(buffer), &value, NULL));
}

static void test_BLINK_Lexer_getToken_comment_eof(void **user)
{
    
    const char input[] = "# this is a comment";
    union blink_token_value value;
    enum blink_token expected = TOK_EOF;
    struct blink_stream stream;
    (void)BLINK_Stream_initBufferReadOnly(&stream, (const uint8_t *)input, sizeof(input));

    assert_int_equal(expected, BLINK_Lexer_getToken(&stream, buffer, sizeof(buffer), &value, NULL));
}

static void test_BLINK_Lexer_getToken_comment_u8(void **user)
{
    
    const char input[] = "# this is a comment\nu8";
    union blink_token_value value;
    enum blink_token expected = TOK_U8;
    struct blink_stream stream;
    (void)BLINK_Stream_initBufferReadOnly(&stream, (const uint8_t *)input, sizeof(input));

    assert_int_equal(expected, BLINK_Lexer_getToken(&stream, buffer, sizeof(buffer), &value, NULL));
}

static void test_BLINK_Lexer_getToken_literal_doubleQuote(void **user)
{
    
    const char input[] = "\"this is a literal\"";
    union blink_token_value value;
    enum blink_token expected = TOK_LITERAL;
    struct blink_stream stream;
    (void)BLINK_Stream_initBufferReadOnly(&stream, (const uint8_t *)input, sizeof(input));

    assert_int_equal(expected, BLINK_Lexer_getToken(&stream, buffer, sizeof(buffer), &value, NULL));
    
    assert_int_equal(strlen("this is a literal"), value.literal.len);
    assert_memory_equal("this is a literal", value.literal.ptr, value.literal.len);
}

static void test_BLINK_Lexer_getToken_literal_singleQuote(void **user)
{
    
    const char input[] = "'this is a literal'";
    union blink_token_value value;
    enum blink_token expected = TOK_LITERAL;
    struct blink_stream stream;
    (void)BLINK_Stream_initBufferReadOnly(&stream, (const uint8_t *)input, sizeof(input));

    assert_int_equal(expected, BLINK_Lexer_getToken(&stream, buffer, sizeof(buffer), &value, NULL));
    
    assert_int_equal(strlen("this is a literal"), value.literal.len);
    assert_memory_equal("this is a literal", value.literal.ptr, value.literal.len);
}

static void test_BLINK_Lexer_getToken_literal_mixedQuote(void **user)
{
    
    const char input[] = "'this is a literal\"";
    union blink_token_value value;
    enum blink_token expected = TOK_UNKNOWN;
    struct blink_stream stream;
    (void)BLINK_Stream_initBufferReadOnly(&stream, (const uint8_t *)input, sizeof(input));

    assert_int_equal(expected, BLINK_Lexer_getToken(&stream, buffer, sizeof(buffer), &value, NULL));    
}

static void test_BLINK_Lexer_getToken_literal_newline(void **user)
{
    
    const char input[] = "'this is a\nliteral'";
    union blink_token_value value;
    enum blink_token expected = TOK_UNKNOWN;
    struct blink_stream stream;
    (void)BLINK_Stream_initBufferReadOnly(&stream, (const uint8_t *)input, sizeof(input));

    assert_int_equal(expected, BLINK_Lexer_getToken(&stream, buffer, sizeof(buffer), &value, NULL));    
}

static void test_BLINK_Lexer_getToken_literal_tooBig(void **user)
{
    
    const char input[] =
        "'0123456789"
        "0123456789"
        "0123456789"
        "0123456789"
        "0123456789"
        "0"
        ;
    union blink_token_value value;
    enum blink_token expected = TOK_ENOMEM;
    struct blink_stream stream;
    (void)BLINK_Stream_initBufferReadOnly(&stream, (const uint8_t *)input, sizeof(input));

    assert_int_equal(expected, BLINK_Lexer_getToken(&stream, buffer, sizeof(buffer), &value, NULL));    
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_BLINK_Lexer_getToken_name),
        cmocka_unit_test(test_BLINK_Lexer_getToken_name_leadingWS),
        cmocka_unit_test(test_BLINK_Lexer_getToken_name_leadingWS_trailingWS),
        cmocka_unit_test(test_BLINK_Lexer_getToken_double_name),
        cmocka_unit_test(test_BLINK_Lexer_getToken_escaped_name),
        cmocka_unit_test(test_BLINK_Lexer_getToken_name_tooBig),

        cmocka_unit_test(test_BLINK_Lexer_getToken_cname),
        cmocka_unit_test(test_BLINK_Lexer_getToken_cname_tooBig_beforeColon),
        cmocka_unit_test(test_BLINK_Lexer_getToken_cname_tooBig_afterColon),
        
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
        cmocka_unit_test(test_BLINK_Lexer_getToken_literal_newline),
        cmocka_unit_test(test_BLINK_Lexer_getToken_literal_tooBig)
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
