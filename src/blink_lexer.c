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

/* structs ************************************************************/

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
static bool isName(const char *in, size_t inLen, size_t *read, const char **out, size_t *outLen);
static bool isCName(const char *in, size_t inLen, size_t *read, const char **out, size_t *outLen);
static bool isUnsignedNumber(const char *in, size_t inLen, size_t *read, uint64_t *out);
static bool isSignedNumber(const char *in, size_t inLen, size_t *read, int64_t *out);
static bool isHexNumber(const char *in, size_t inLen, size_t *read, uint64_t *out);
static bool stringToToken(const char *in, size_t inLen, size_t *read, enum blink_token *token);
static bool isLiteral(const char *in, size_t inLen, size_t *read, const char **out, size_t *outLen);

/* functions **********************************************************/

const char *BLINK_TokenToString(enum blink_token token, size_t *len)
{
    const char *retval = NULL;    
    size_t i;
    size_t l = 0U;

    switch(token){
    case TOK_NAME:
        retval = "<name>";
        l = sizeof("<name>")-1U;
        break;    
    case TOK_CNAME:
        retval = "<cname>";
        l = sizeof("<cname>")-1U;
        break;    
    case TOK_EOF:
        retval = "<eof>";
        l = sizeof("<eof>")-1U;
        break;    
    case TOK_UINT:
        retval = "<uint>";
        l = sizeof("<uint>")-1U;
        break;    
    case TOK_INT:
        retval = "<int>";
        l = sizeof("<int>")-1U;
        break;    
    case TOK_LITERAL:
        retval = "<literal>";
        l = sizeof("<literal>")-1U;
        break;    
    default:
    
        for(i = 0U; i < (sizeof(tokenTable)/sizeof(*tokenTable)); i++){

            if(tokenTable[i].token == token){

                retval = tokenTable[i].s;
                l = tokenTable[i].size;
                break;
            }
        }
        break;
    }

    if(len != NULL){

        *len = l;
    }

    return retval;
}

enum blink_token BLINK_GetToken(const char *in, size_t inLen, size_t *read, union blink_token_value *value, struct blink_token_location *location)
{
    size_t pos = 0U;
    enum blink_token retval = TOK_EOF;
    size_t r;

    /* skip whitespace */
    while(pos < inLen){

        if(!isSeparator(in[pos])){
            break;
        }
        
        pos++;
    }

    /* skip comment */
    if(in[pos] == '#'){

        while(pos < inLen){

            if(in[pos] == '\n'){
                pos++;
                break;
            }

            pos++;
        }
    }

    *read = pos;

    if(pos < inLen){

        if(in[pos] == '\0'){

            retval = TOK_EOF;
        }
        else if(isCName(&in[pos], inLen - pos, &r, &value->literal.ptr, &value->literal.len)){

            *read += r;
            retval = TOK_CNAME;
        }
        else if(stringToToken(&in[pos], inLen - pos, &r, &retval)){

            *read += r;
        }
        else if(isLiteral(&in[pos], inLen - pos, &r, &value->literal.ptr, &value->literal.len)){

            *read += r;
            retval = TOK_LITERAL;                
        }
        else if(isName(&in[pos], inLen - pos, &r, &value->literal.ptr, &value->literal.len)){

            *read += r;
            retval = TOK_NAME;
        }
        else if(isHexNumber(&in[pos], inLen - pos, &r, &value->number)){

            *read += r;
            retval = TOK_UINT;
        }
        else if(isUnsignedNumber(&in[pos], inLen - pos, read, &value->number)){
            
            *read += r;
            retval = TOK_UINT;
        }
        else if(isSignedNumber(&in[pos], inLen - pos, read, &value->signedNumber)){

            *read += r;
            retval = TOK_INT;
        }
        else{

            retval = TOK_UNKNOWN;
        }
    }

    return retval;        
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

static bool stringToToken(const char *in, size_t inLen, size_t *read, enum blink_token *token)
{
    BLINK_ASSERT(in != NULL)
    BLINK_ASSERT(read != NULL)

    size_t i;
    bool retval = false;
    
    *read = 0U;

    for(i=0; i < (sizeof(tokenTable)/sizeof(*tokenTable)); i++){

        if(tokenTable[i].size <= inLen){

            if(memcmp(tokenTable[i].s, in, tokenTable[i].size) == 0){

                *read = tokenTable[i].size;
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

static bool isName(const char *in, size_t inLen, size_t *read, const char **out, size_t *outLen)
{
    BLINK_ASSERT(in != NULL)
    BLINK_ASSERT(read != NULL)
    BLINK_ASSERT(out != NULL)
    BLINK_ASSERT(outLen != NULL)

    bool retval = false;
    *read = 0U;

    if((*read < inLen) && (isFirstNameChar(*in) || (*in == '\\'))){

        bool escape = (*in == '\\') ? true : false;

        *out = in;
        *outLen = 1U;

        (*read)++;

        if(escape){

            if((*read < inLen) && isFirstNameChar(in[*read])){

                *out = &in[*read];
                retval = true;
                (*read)++;
            }
        }
        else{

            retval = true;
        }

        while(retval && (*read < inLen)){

            if(isNameChar(in[*read])){

                (*read)++;
                (*outLen)++;
            }
            else{

                break;
            }
        }
    }

    return retval;
}

static bool isCName(const char *in, size_t inLen, size_t *read, const char **out, size_t *outLen)
{
    BLINK_ASSERT(in != NULL)
    BLINK_ASSERT(read != NULL)
    BLINK_ASSERT(out != NULL)
    BLINK_ASSERT(outLen != NULL)

    bool retval = false;
    *read = 0U;

    if((*read < inLen) && isFirstNameChar(*in)){
        
        *out = in;
        *outLen = 1U;
        (*read)++;

        while(*read < inLen){

            if(isNameChar(in[*read])){

                (*read)++;
                (*outLen)++;
            }
            else if(in[*read] == ':'){

                (*read)++;
                (*outLen)++;

                if((*read < inLen) && isNameChar(in[*read])){

                    (*read)++;
                    (*outLen)++;

                    retval = true;
                    
                    while((*read < inLen) && isNameChar(in[*read])){

                        (*read)++;
                        (*outLen)++;
                    }
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
    
static bool isUnsignedNumber(const char *in, size_t inLen, size_t *read, uint64_t *out)
{
    BLINK_ASSERT(in != NULL)
    BLINK_ASSERT(read != NULL)
    BLINK_ASSERT(out != NULL)

    bool retval = false;
    *read = 0U;
    uint8_t digit = 0U;

    /*lint -e9007 side effect: digit may be initialised */
    if((*read < inLen) && isInteger(*in, &digit)){

        retval = true;
        *out = (uint64_t)digit;
        (*read)++;

        while(retval && (*read < inLen)){
            
            if(isInteger(in[*read], &digit)){

                /* todo: overflow protect */
                *out *= 10;                
                *out += digit;
                (*read)++;
                
            }
            else{

                break;
            } 
        }
    }
    
    return retval;
}

static bool isSignedNumber(const char *in, size_t inLen, size_t *read, int64_t *out)
{
    BLINK_ASSERT(in != NULL)
    BLINK_ASSERT(read != NULL)
    BLINK_ASSERT(out != NULL)

    bool retval = false;
    *read = 0U;
    uint8_t digit = 0U;
    
    if((*read < inLen) && ((*in == '-') || isInteger(*in, &digit))){

        bool negative = (*in == '-') ? true : false;

        (*read)++;

        if(negative){

            if((*read < inLen) && isInteger(in[*read], &digit)){

                *out = (int64_t)digit;
                (*read)++;
                retval = true;
            }                
        }
        else{

            *out = (int64_t)digit;
            retval = true;
        }            

        while(retval && (*read < inLen)){
            
            if(isInteger(in[*read], &digit)){
                
                /* todo: overflow protect */
                *out *= 10;                
                *out += digit;
                (*read)++;                               
            }
            else{

                break;
            } 
        }

        if(retval){

            *out = 0 - *out;
        }
    }    

    return retval;
}

static bool isHexNumber(const char *in, size_t inLen, size_t *read, uint64_t *out)
{
    BLINK_ASSERT(in != NULL)
    BLINK_ASSERT(read != NULL)
    BLINK_ASSERT(out != NULL)

    bool retval = false;
    *read = 0U;
    uint8_t digits = 1U;
    uint8_t digit = 0U;
    
    if((*read < inLen) && (*in == '0')){

        (*read)++;

        if((*read < inLen) && (in[*read] == 'x')){

            (*read)++;    

            /*lint -e9007 side effect: digit may be initialised */
            if((*read < inLen) && isHexInteger(in[*read], &digit)){

                (*read)++;

                *out = (uint64_t)digit;

                retval = true;

                while(retval && (*read < inLen)){

                    if(isHexInteger(in[*read], &digit)){

                        (*read)++;

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

    return retval;
}

static bool isLiteral(const char *in, size_t inLen, size_t *read, const char **out, size_t *outLen)
{
    BLINK_ASSERT(in != NULL)
    BLINK_ASSERT(read != NULL)
    BLINK_ASSERT(out != NULL)
    BLINK_ASSERT(outLen != NULL)

    bool retval = false;
    char mark;
    
    *read = 0U;

    if((*read < inLen) && ((*in == '"') || (*in == '\''))){

        mark = *in;
        (*read)++;
        *out = &in[*read];
        *outLen = 0U;

        while(*read < inLen){

            char c = in[*read];
            (*read)++;

            if(c == mark){
                
                retval = true;
                break;
            }
            else if(c == '\n'){

                break;
            }
            else{

                (*outLen)++;
            }
        }
    }

    return retval;
}
