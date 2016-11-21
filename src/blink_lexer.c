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
static bool isNum(const char *in, size_t inLen, size_t *read, uint64_t *out);
static bool isHexNum(const char *in, size_t inLen, size_t *read, uint64_t *out);
static bool stringToToken(const char *in, size_t inLen, size_t *read, enum blink_token *token);
static bool isLiteral(const char *in, size_t inLen, size_t *read, const char **out, size_t *outLen);

/* functions **********************************************************/

const char *BLINK_TokenToString(enum blink_token token, size_t *len)
{
    const char *retval = NULL;    
    size_t i;

    *len = 0U;

    switch(token){
    case TOK_NAME:
        retval = "<name>";
        *len = sizeof("<name>")-1U;
        break;    
    case TOK_CNAME:
        retval = "<cname>";
        *len = sizeof("<cname>")-1U;
        break;    
    case TOK_EOF:
        retval = "<eof>";
        *len = sizeof("<eof>")-1U;
        break;    
    case TOK_NUMBER:
        retval = "<number>";
        *len = sizeof("<number>")-1U;
        break;    
    case TOK_LITERAL:
        retval = "<literal>";
        *len = sizeof("<literal>")-1U;
        break;    
    default:
    
        for(i = 0U; i < (sizeof(tokenTable)/sizeof(*tokenTable)); i++){

            if(tokenTable[i].token == token){

                retval = tokenTable[i].s;
                *len = tokenTable[i].size;
                break;
            }
        }
        break;
    }

    return retval;
}

/*lint -e(9018) argument value is a union where the relevant field is determined by the function return value */
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
        else{

            if(isLiteral(&in[pos], inLen - pos, &r, &value->literal.ptr, &value->literal.len)){

                *read += r;
                retval = TOK_LITERAL;                
            }
            else if(isName(&in[pos], inLen - pos, &r, &value->literal.ptr, &value->literal.len)){

                *read += r;
                retval = TOK_NAME;
            }
            else if(isHexNum(&in[pos], inLen - pos, &r, &value->number) || isNum(&in[pos], inLen - pos, read, &value->number)){

                *read += r;
                retval = TOK_NUMBER;
            }
            else{

                retval = TOK_UNKNOWN;
            }
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
    BLINK_ASSERT(out != NULL);
    BLINK_ASSERT(outLen != NULL);

    bool retval = false;
    char c;
    enum {
        EXIT,
        START,
        FIRST_CHAR,
        NAME
    } state = START;

    *read = 0U;

    while(*read < inLen){

        c = in[*read];
        
        switch(state){
        case START:

            if(c == '\\'){

                state = FIRST_CHAR;                
            }
            else if(isFirstNameChar(c)){

                *out = &in[*read];
                *outLen = 1U;
                state = NAME;
            }
            else{

                state = EXIT;
            }         
            break;

        case FIRST_CHAR:
            
            if(isFirstNameChar(c)){
                
                *out = &in[*read];
                *outLen = 1U;
                state = NAME;
            }
            else{

                state = EXIT;
            }
            break;

        case NAME:

            if(!isNameChar(c)){

                retval = true;                
                state = EXIT;
            }
            else{

                (*outLen)++;
            }
            break;

        case EXIT:
        default:
            break;
        }

        if(state == EXIT){

            break;
        }

        (*read)++;
    }

    return retval;
}

static bool isCName(const char *in, size_t inLen, size_t *read, const char **out, size_t *outLen)
{
    BLINK_ASSERT(in != NULL)
    BLINK_ASSERT(read != NULL)
    BLINK_ASSERT(out != NULL);
    BLINK_ASSERT(outLen != NULL);

    char c;
    enum {
        START,
        FIRST,
        SECOND,
        EXIT,
    } state = START;

    bool retval = false;
    *read = 0U;

    while(*read < inLen){

        c = in[*read];
        
        switch(state){
        case START:

            if(isFirstNameChar(c)){

                *out = &in[*read];
                *outLen = 1U;
                state = FIRST;
            }   
            else{

                state = EXIT;
            }
            break;
            
        case FIRST:

            if(isNameChar(c)){

                (*outLen)++;
            }
            else if(c == ':'){

                (*outLen)++;
                state = SECOND;
            }
            else{
             
                state = EXIT;
            }
            break;

        case SECOND:

            if(isNameChar(c)){

                (*outLen)++;
            }
            else if(*out[*outLen] == ':'){

                state = EXIT;
            }
            else{
                
                retval = true;                
                state = EXIT;
            }
            break;

        case EXIT:
        default:
            break;
        }

        if(state == EXIT){

            break;
        }

        (*read)++;
    }
    

    return retval;
}
    
static bool isNum(const char *in, size_t inLen, size_t *read, uint64_t *out)
{
    BLINK_ASSERT(in != NULL)
    BLINK_ASSERT(read != NULL)
    BLINK_ASSERT(out != NULL)

    char c;
    bool retval = false;
    *read = 0U;
    uint8_t digits = 0;
    uint8_t digit;

    enum {
        START,
        NUMBER,
        EXIT
    } state = START;
    
    while(*read < inLen){

        c = in[*read];

        switch(state){
        case START:
            
            if(isInteger(c, &digit)){

                *out = (uint64_t)digit;
                state = NUMBER;
                retval = true;
                digits++;
            }
            else{

                state = EXIT;
            }
            
            break;

        case NUMBER:
        
            if(isInteger(c, &digit)){

                if(digits <= 20){

                    *out *= 10;                
                    *out += digit;
                    digits++;                    
                }
                else{

                    BLINK_ERROR("too many digits for a 64bit number")
                    retval = false;
                    state = EXIT;
                }
            }
            else if((c == 'x') && (digits == 1) && (in[(*read)-1] == '0')){

                retval = false;
                state = EXIT;
            }
            else{
                
                state = EXIT;
            }
            break;
            
        case EXIT:
        default:
            break;
        }

        if(state == EXIT){

            break;
        }

        (*read)++;
    }

    return retval;
}

static bool isHexNum(const char *in, size_t inLen, size_t *read, uint64_t *out)
{
    BLINK_ASSERT(in != NULL)
    BLINK_ASSERT(read != NULL)
    BLINK_ASSERT(out != NULL)

    char c;
    bool retval = false;
    *read = 0U;
    uint8_t digits = 0;
    uint8_t digit;
    *out = 0U;

    enum {
        START,
        PRE,
        NUMBER,
        EXIT
    }state = START;
    
    while(*read < inLen){

        c = in[*read];

        switch(state){
        case START:
            
            if(c == '0'){

                state = PRE;
            }
            else{

                state = EXIT;
            }            
            break;
            
        case PRE:

            if(c == 'x'){

                state = NUMBER;
            }
            else{

                state = EXIT;
            }
            break;
        
        case NUMBER:

            if(isHexInteger(c, &digit)){

                if(digits < 16U){

                    *out <<= 4;
                    *out |= digit;
                    digits++;
                    retval = true;                
                }
                else{

                    state = EXIT;
                    retval = false;
                    BLINK_ERROR("too many digits for 64 bit number")
                }
            }
            else{

                state = EXIT;                
            }
            break;
            
        case EXIT:
        default:
            break;
        }

        if(state == EXIT){

            break;
        }

        (*read)++;
    }

    return retval;
}

static bool isLiteral(const char *in, size_t inLen, size_t *read, const char **out, size_t *outLen)
{
    BLINK_ASSERT(in != NULL)
    BLINK_ASSERT(read != NULL)
    BLINK_ASSERT(out != NULL);
    BLINK_ASSERT(outLen != NULL);

    bool retval = false;
    char c;
    char mark;
    enum {
        EXIT,
        START,
        LITERAL
    } state = START;

    *read = 0U;

    while(*read < inLen){

        c = in[*read];
        
        switch(state){
        case START:

            if((c == '"') || (c == '\'')){

                mark = c;
                state = LITERAL;
                *out = &in[1U];
                *outLen = 0U;
            }
            else{

                state = EXIT;
            }
            break;                

        case LITERAL:

            if(c == '\n'){

                state = EXIT;
            }
            else if(c == mark){

                retval = true;
                state = EXIT;
            }
            else{
        
                (*outLen)++;
            }
            break;

        case EXIT:
        default:
            break;
        }

        (*read)++;

        if(state == EXIT){

            break;
        }
    }

    return retval;
}
