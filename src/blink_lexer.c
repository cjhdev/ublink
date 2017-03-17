/* Copyright (c) 2016 Cameron Harper
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * */

/* includes ***********************************************************/

#include <string.h>
#include <stdbool.h>

#include "blink_debug.h"
#include "blink_lexer.h"
#include "blink_stream.h"

/* types **************************************************************/

struct token_table {
    const char *s;
    size_t size;
    enum blink_token token;    
};

/* static variables ***************************************************/

static const struct token_table tokenTable[] = {
    {"*", sizeof("*")-1U, TOK_STAR},    
    {"=", sizeof("=")-1U, TOK_EQUAL},
    {".", sizeof(".")-1U, TOK_PERIOD},
    {",", sizeof(",")-1U, TOK_COMMA},    
    {"(", sizeof("(")-1U, TOK_LPAREN},    
    {")", sizeof(")")-1U, TOK_RPAREN},    
    {"[", sizeof("[")-1U, TOK_LBRACKET},    
    {"]", sizeof("]")-1U, TOK_RBRACKET},    
    {":", sizeof(":")-1U, TOK_COLON},
    {"/", sizeof("/")-1U, TOK_SLASH},    
    {"?", sizeof("?")-1U, TOK_QUESTION},    
    {"@", sizeof("@")-1U, TOK_AT},    
    {"|", sizeof("|")-1U, TOK_BAR},    
    {"->", sizeof("->")-1U, TOK_RARROW},
    {"<-", sizeof("<-")-1U, TOK_LARROW},
    {"i8", sizeof("i8")-1U, TOK_I8},
    {"i16", sizeof("i16")-1U, TOK_I16},
    {"i32", sizeof("i32")-1U, TOK_I32},
    {"i64", sizeof("i64")-1U, TOK_I64},
    {"u8", sizeof("u8")-1U, TOK_U8},
    {"u16", sizeof("u16")-1U, TOK_U16},
    {"u32", sizeof("u32")-1U, TOK_U32},
    {"u64", sizeof("u64")-1U, TOK_U64},
    {"f64", sizeof("f64")-1U, TOK_F64},
    {"decimal", sizeof("decimal")-1U, TOK_DECIMAL},
    {"date", sizeof("date")-1U, TOK_DATE},
    {"timeOfDayMilli", sizeof("timeOfDayMilli")-1U, TOK_TIME_OF_DAY_MILLI},
    {"timeOfDayNano", sizeof("timeOfDayNano")-1U, TOK_TIME_OF_DAY_NANO},
    {"nanotime", sizeof("nanotime")-1U, TOK_NANO_TIME},
    {"millitime", sizeof("millitime")-1U, TOK_MILLI_TIME},
    {"bool", sizeof("bool")-1U, TOK_BOOL},
    {"string", sizeof("string")-1U, TOK_STRING},
    {"binary", sizeof("binary")-1U, TOK_BINARY},
    {"object", sizeof("object")-1U, TOK_OBJECT},
    {"namespace", sizeof("namespace")-1U, TOK_NAMESPACE},
    {"type", sizeof("type")-1U, TOK_TYPE},
    {"schema", sizeof("schema")-1U, TOK_SCHEMA},
    {"fixed", sizeof("fixed")-1U, TOK_FIXED}        
};

/* static prototypes **************************************************/

static bool isSeparator(char c);
static bool isInteger(char c, uint8_t *out);
static bool isHexInteger(char c, uint8_t *out);
static bool isNameChar(char c);
static bool isFirstNameChar(char c);

static bool isName(blink_stream_t in, bool *enomem, char *out, size_t outMax, size_t *outLen);
static bool isCName(blink_stream_t in, bool *enomem, char *out, size_t outMax, size_t *outLen);
static bool isUnsignedNumber(blink_stream_t in, uint64_t *out);
static bool isSignedNumber(blink_stream_t in, int64_t *out);
static bool isHexNumber(blink_stream_t in, uint64_t *out);
static bool stringToToken(blink_stream_t in, enum blink_token *token);
static bool isLiteral(blink_stream_t in, bool *enomem, char *out, size_t outMax, size_t *outLen);

/* functions **********************************************************/

const char *BLINK_Lexer_tokenToString(enum blink_token token)
{
    const char *retval = NULL;    
    size_t i;

    switch(token){
    case TOK_NAME:
        retval = "<name>";
        break;    
    case TOK_CNAME:
        retval = "<cname>";
        break;    
    case TOK_EOF:
        retval = "<eof>";
        break;    
    case TOK_UINT:
        retval = "<uint>";
        break;    
    case TOK_INT:
        retval = "<int>";
        break;    
    case TOK_LITERAL:
        retval = "<literal>";
        break;
    case TOK_UNKNOWN:
        retval = "<unknown>";
        break;
    case TOK_ENOMEM:
        retval = "<enomem>";
        break;
    default:
    
        for(i = 0U; i < (sizeof(tokenTable)/sizeof(*tokenTable)); i++){

            if(tokenTable[i].token == token){

                retval = tokenTable[i].s;
                break;
            }
        }
        break;
    }

    BLINK_ASSERT(retval != NULL)

    return retval;
}

enum blink_token BLINK_Lexer_getToken(blink_stream_t in, char *buffer, size_t max, union blink_token_value *value, struct blink_token_location *location)
{
    BLINK_ASSERT(in != NULL)
    BLINK_ASSERT(buffer != NULL)
    BLINK_ASSERT(value != NULL)
    
    enum blink_token retval = TOK_EOF;
    char c;
    bool enomem = false;
    
    (void)memset(value, 0, sizeof(*value));

    /* skip whitespace */
    while(BLINK_Stream_peek(in, &c)){

        if(!isSeparator(c)){
            break;
        }

        if(location != NULL){

            if(c == '\n'){

                location->row++;
                location->col = 0U;            
            }
            else{

                location->col++;            
            }
        }
            
        (void)BLINK_Stream_seekCur(in, (int32_t)sizeof(c));
    }

    /* skip comment */
    if(BLINK_Stream_peek(in, &c) && (c == '#')){

        while(BLINK_Stream_peek(in, &c)){

            (void)BLINK_Stream_seekCur(in, (int32_t)sizeof(c));

            if(c == '\n'){

                if(location != NULL){

                    location->row++;
                    location->col = 0U;
                }                    
                break;
            }
        }
    }

    size_t pos = BLINK_Stream_tell(in);

    if(!BLINK_Stream_read(in, &c, sizeof(c)) || (c == '\0')){

        return TOK_EOF;
    }

    (void)BLINK_Stream_seekSet(in, (uint32_t)pos);

    if(isCName(in, &enomem, buffer, max, &value->literal.len)){

        value->literal.ptr = buffer;
        
        return TOK_CNAME;
    }

    if(enomem){

        return TOK_ENOMEM;
    }

    (void)BLINK_Stream_seekSet(in, (uint32_t)pos);
        
    if(stringToToken(in, &retval)){

        return retval;
    }

    (void)BLINK_Stream_seekSet(in, (uint32_t)pos);

    if(isLiteral(in, &enomem, buffer, max, &value->literal.len)){

        value->literal.ptr = buffer;

        return TOK_LITERAL;
    }

    if(enomem){

        return TOK_ENOMEM;
    }
                
    (void)BLINK_Stream_seekSet(in, (uint32_t)pos);

    if(isName(in, &enomem, buffer, max, &value->literal.len)){

        value->literal.ptr = buffer;

        return TOK_NAME;
    }

    if(enomem){

        return TOK_ENOMEM;
    }
                    
    (void)BLINK_Stream_seekSet(in, (uint32_t)pos);
                        
    if(isHexNumber(in, &value->number)){

        return TOK_UINT;                            
    }
                        
    (void)BLINK_Stream_seekSet(in, (uint32_t)pos);

    if(isUnsignedNumber(in, &value->number)){

        return TOK_UINT;
    }

    (void)BLINK_Stream_seekSet(in, (uint32_t)pos);
              
    if(isSignedNumber(in, &value->signedNumber)){

        return TOK_INT;
    }
              
    return TOK_UNKNOWN;        
}

/* static functions ***************************************************/

static bool isSeparator(char c)
{
    bool retval = false;
    
    switch(c){
    case ' ':
    case '\n':
    case '\t':
    case '\r':
        retval = true;
        break;
    default:
        /* not a separator */
        break;
    }

    return retval;
}

static bool stringToToken(blink_stream_t in, enum blink_token *token)
{
    BLINK_ASSERT(in != NULL)
    BLINK_ASSERT(token != NULL)

    size_t i;
    char buf[20U];
    bool retval = false;
    size_t pos = BLINK_Stream_tell(in);

    for(i=0; i < (sizeof(tokenTable)/sizeof(*tokenTable)); i++){

        (void)BLINK_Stream_seekSet(in, (uint32_t)pos);

        BLINK_ASSERT(sizeof(buf) > tokenTable[i].size)

        if(BLINK_Stream_read(in, buf, tokenTable[i].size)){

            if(memcmp(tokenTable[i].s, buf, tokenTable[i].size) == 0){

                *token = tokenTable[i].token;
                retval = true;
                break;
            }
        }
    }

    return retval;
}

static bool isInteger(char c, uint8_t *out)
{
    BLINK_ASSERT(out != NULL)
    
    bool retval = false;

    if((c >= '0') && (c <= '9')){

        *out = (uint8_t)((((int)c) - '0') & 0xff);
        retval = true;
    }

    return retval;
}

static bool isHexInteger(char c, uint8_t *out)
{
    BLINK_ASSERT(out != NULL)
    
    bool retval = true;
    
    if((c >= '0') && (c <= '9')){

        *out = (uint8_t)(c - '0');
    }
    else if((c >= 'a') && (c <= 'f')){

        *out = (uint8_t)(c - 'a' + 10);
    }
    else if((c >= 'A') && (c <= 'F')){
        
        *out = (uint8_t)(c - 'A' + 10);        
    }
    else{
        
        retval = false;
    }

    return retval;
}

static bool isNameChar(char c)
{
    return ((c == '_') || ((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z')) || ((c >= '0') && (c <= '9')));
}

static bool isFirstNameChar(char c)
{
    return ((c == '_') || ((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z')));
}

static bool isName(blink_stream_t in, bool *enomem, char *out, size_t outMax, size_t *outLen)
{
    BLINK_ASSERT(in != NULL)    
    BLINK_ASSERT(enomem != NULL)    
    BLINK_ASSERT(out != NULL)
    BLINK_ASSERT(outLen != NULL)

    char c;
    size_t pos = 0U;
    bool retval = false;

    *enomem = false;
    
    if(BLINK_Stream_read(in, &c, sizeof(c))){

        if(isFirstNameChar(c) || (c == '\\')){

            bool escape = (c == '\\') ? true : false;

            if(escape){

                if(BLINK_Stream_read(in, &c, 1U) && isFirstNameChar(c)){

                    if(outMax > 0U){

                        out[pos] = c;
                        pos++;
                        retval = true;
                    }
                    else{

                        *enomem = true;
                    }
                }
            }
            else{

                if(outMax > 0U){

                    out[pos] = c;
                    pos++;
                    retval = true;
                }
                else{

                    *enomem = true;
                }
            }

            if(retval){

                while(BLINK_Stream_peek(in, &c)){

                    if(isNameChar(c)){

                        (void)BLINK_Stream_seekCur(in, (int32_t)sizeof(c));

                        if(pos < outMax){

                            out[pos] = c;
                            pos++;
                        }
                        else{

                            BLINK_ERROR("buffer overrun")
                            *enomem = true;
                            retval = false;
                            break;
                        }
                    }
                    else{

                        *outLen = pos;                        
                        break;
                    }
                }
            }
        }
    }
    
    return retval;
}

static bool isCName(blink_stream_t in, bool *enomem, char *out, size_t outMax, size_t *outLen)
{
    BLINK_ASSERT(in != NULL)
    BLINK_ASSERT(out != NULL)
    BLINK_ASSERT(outLen != NULL)

    char c;
    size_t pos = 0U;
    bool retval = false;

    *enomem = false;

    if(BLINK_Stream_peek(in, &c) && isFirstNameChar(c)){
        
        while(BLINK_Stream_read(in, &c, sizeof(c))){

            if(isNameChar(c)){

                if(pos < outMax){
                
                    out[pos] = c;
                    pos++;
                }
                else{

                    *enomem = true;
                    break;
                }
            }
            else if(c == ':'){

                if(pos < outMax){
            
                    out[pos] = c;
                    pos++;

                    if(BLINK_Stream_read(in, &c, sizeof(c)) && isNameChar(c)){

                        if(pos < outMax){
        
                            out[pos] = c;
                            pos++;
    
                            retval = true;
                            
                            while(BLINK_Stream_peek(in, &c) && isNameChar(c)){

                                (void)BLINK_Stream_seekCur(in, (int32_t)sizeof(c));

                                if(pos < outMax){

                                    out[pos] = c;
                                    pos++;
                                }
                                else{

                                    *enomem = true;
                                    retval = false;
                                    break;
                                }
                            }

                            *outLen = pos;
                        }
                        else{

                            *enomem = true;
                        }
                    }
                }
                else{

                    *enomem = true;
                }

                break;            
            }
            else{

                break;
            }
        }
    }

    return retval;
}
    
static bool isUnsignedNumber(blink_stream_t in, uint64_t *out)
{
    BLINK_ASSERT(in != NULL)    
    BLINK_ASSERT(out != NULL)

    bool retval = false;
    uint8_t digit = 0U;
    char c;

    if(BLINK_Stream_read(in, &c, sizeof(c))){

        if(isInteger(c, &digit)){
    
            retval = true;
            *out = (uint64_t)digit;
            
            while(retval && BLINK_Stream_peek(in, &c)){
                
                if(isInteger(c, &digit)){

                    /* todo: overflow protect */
                    *out *= 10;                
                    *out += digit;
                    (void)BLINK_Stream_seekCur(in, (int32_t)sizeof(c));
                }
                else{

                    break;
                } 
            }
        }
    }
    
    return retval;
}

static bool isSignedNumber(blink_stream_t in, int64_t *out)
{
    BLINK_ASSERT(in != NULL)
    BLINK_ASSERT(out != NULL)

    bool retval = false;    
    uint8_t digit = 0U;
    char c;
    
    if(BLINK_Stream_read(in, &c, sizeof(c))){

        if(isInteger(c, &digit) || (c == '-')){

            bool negative = (c == '-') ? true : false;

            if(negative){

                if(BLINK_Stream_peek(in, &c)){

                    if(isInteger(c, &digit)){

                        *out = (int64_t)digit;
                        (void)BLINK_Stream_seekCur(in, (int32_t)sizeof(c));
                        retval = true;
                    }
                }                
            }
            else{

                *out = (int64_t)digit;
                retval = true;
            }            

            if(retval){

                while(BLINK_Stream_peek(in, &c)){
                    
                    if(isInteger(c, &digit)){
                        
                        /* todo: overflow protect */
                        *out *= 10;                
                        *out += digit;
                        (void)BLINK_Stream_seekCur(in, (int32_t)sizeof(c));
                    }
                    else{

                        break;
                    } 
                }

                *out = 0 - *out;
            }
        }
    }    

    return retval;
}

static bool isHexNumber(blink_stream_t in, uint64_t *out)
{
    BLINK_ASSERT(in != NULL)
    BLINK_ASSERT(out != NULL)

    bool retval = false;
    uint8_t digits = 1U;
    uint8_t digit = 0U;
    char c;
    
    if(BLINK_Stream_read(in, &c, sizeof(c)) && (c == '0')){

        if(BLINK_Stream_read(in, &c, sizeof(c)) && (c == 'x')){

            if(BLINK_Stream_read(in, &c, sizeof(c))){

                if(isHexInteger(c, &digit)){

                    *out = (uint64_t)digit;

                    retval = true;

                    while(BLINK_Stream_peek(in, &c) && retval){

                        if(isHexInteger(c, &digit)){

                            (void)BLINK_Stream_seekCur(in, (int32_t)sizeof(c));

                            if(digits <= 16U){

                                *out <<= 4;
                                *out |= digit;
                                digits++;
                            }
                            else{
                                
                                retval = false;
                            }
                        }
                        else{

                            break;
                        }
                    }
                }
            }
        }
    }

    return retval;
}

static bool isLiteral(blink_stream_t in, bool *enomem, char *out, size_t outMax, size_t *outLen)
{
    BLINK_ASSERT(in != NULL)
    BLINK_ASSERT(enomem != NULL)
    BLINK_ASSERT(out != NULL)
    BLINK_ASSERT(outLen != NULL)

    bool retval = false;
    char mark;
    char c;
    size_t pos = 0U;
    
    *enomem = false;    

    if(BLINK_Stream_read(in, &c, sizeof(c)) && ((c == '"') || (c == '\''))){

        mark = c;
        
        while(BLINK_Stream_read(in, &c, sizeof(c))){

            if(c == mark){

                *outLen = pos;
                retval = true;
                break;
            }
            else if(c == '\n'){

                break;
            }
            else if(pos == outMax){

                *enomem = true;
                break;
            }
            else{

                out[pos] = c;
                pos++;
            }
        }
    }

    return retval;
}
