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
#include <ctype.h>
#include <stdbool.h>

#include "blink_debug.h"
#include "blink_lexer.h"

/* structs ************************************************************/

struct token_table {
    const char *s;
    size_t size;
    enum blink_token type;
};

/* static prototypes **************************************************/

static bool isInteger(char c, uint8_t *out);
static bool isHexInteger(char c, uint8_t *out);
static bool isNameChar(char c);
static bool isFirstNameChar(char c);
static bool isName(const char *in, size_t inLen, size_t *read, const char **out, size_t *outLen);
static bool isCName(const char *in, size_t inLen, size_t *read, const char **out, size_t *outLen);
static bool isNum(const char *in, size_t inLen, size_t *read, uint64_t *out);
static bool isHexNum(const char *in, size_t inLen, size_t *read, uint64_t *out);

/* functions **********************************************************/

/*lint -e(9018) advisory */
enum blink_token BLINK_GetToken(const char *in, size_t inLen, size_t *read, union blink_token_value *value)
{
    size_t pos = 0U;
    enum blink_token retval = T_EOF;
    size_t i;
    size_t r;

    const static struct token_table tt[] = {
        {"->", sizeof("->")-1U, T_RARROW},
        {"<-", sizeof("<-")-1U, T_LARROW},
        {"i8", sizeof("i8")-1U, T_I8},
        {"i16", sizeof("i16")-1U, T_I16},
        {"i32", sizeof("i32")-1U, T_I32},
        {"i64", sizeof("i64")-1U, T_I64},
        {"u8", sizeof("u8")-1U, T_U8},
        {"u16", sizeof("u16")-1U, T_U16},
        {"u32", sizeof("u32")-1U, T_U32},
        {"u64", sizeof("u64")-1U, T_U64},
        {"f64", sizeof("f64")-1U, T_F64},
        {"decimal", sizeof("decimal")-1U, T_DECIMAL},
        {"date", sizeof("date")-1U, T_DATE},
        {"timeOfDayMilli", sizeof("timeOfDayMilli")-1U, T_TIME_OF_DAY_MILLI},
        {"timeOfDayNano", sizeof("timeOfDayNano")-1U, T_TIME_OF_DAY_NANO},
        {"nanotime", sizeof("nanotime")-1U, T_NANO_TIME},
        {"millitime", sizeof("millitime")-1U, T_MILLI_TIME},
        {"bool", sizeof("bool")-1U, T_BOOL},
        {"string", sizeof("string")-1U, T_STRING},
        {"binary", sizeof("binary")-1U, T_BINARY},
        {"object", sizeof("object")-1U, T_OBJECT},
        {"namespace", sizeof("namespace")-1U, T_NAMESPACE},
        {"type", sizeof("type")-1U, T_TYPE},
        {"schema", sizeof("schema")-1U, T_SCHEMA},
        {"fixed", sizeof("fixed")-1U, T_FIXED}        
    };

    /* skip whitespace */
    while(pos < inLen){

        if(!isspace((int)in[pos])){
            break;
        }
        pos++;
    }

    *read = pos;

    if(pos < inLen){

        (*read)++;

        switch(in[pos]){
        case '*':
            retval = T_STAR;
            break;
        case '=':
            retval = T_EQUAL;
            break;
        case '.':
            retval = T_PERIOD;
            break;
        case ',':
            retval = T_COMMA;
            break;        
        case '(':
            retval = T_LPAREN;
            break;    
        case ')':
            retval = T_RPAREN;
            break;    
        case '[':
            retval = T_LBRACKET;
            break;    
        case ']':
            retval = T_RBRACKET;
            break;        
        case ':':
            retval = T_COLON;
            break;    
        case '/':
            retval = T_SLASH;
            break;    
        case '?':
            retval = T_QUESTION;
            break;    
        case '@':
            retval = T_AT;
            break;
        case '|':
            retval = T_BAR;
            break;
        default:

            (*read)--;

            if(isCName(&in[pos], inLen - pos, &r, &value->literal.ptr, &value->literal.len)){

                *read += r;
                retval = T_CNAME;
            }
            else{

                for(i=0; i < (sizeof(tt)/sizeof(*tt)); i++){
                    if(tt[i].size <= (inLen - pos)){

                        if(!memcmp(tt[i].s, &in[pos], tt[i].size)){

                            *read += tt[i].size;
                            retval = tt[i].type;
                            break;
                        }
                    }
                }

                if(i == (sizeof(tt)/sizeof(*tt))){

                    if(isName(&in[pos], inLen - pos, &r, &value->literal.ptr, &value->literal.len)){

                        *read += r;
                        retval = T_NAME;
                    }
                    else if(isHexNum(&in[pos], inLen - pos, &r, &value->number) || isNum(&in[pos], inLen - pos, read, &value->number)){

                        *read += r;
                        retval = T_NUMBER;
                    }
                    else{

                        retval = T_UNKNOWN;
                    }
                }
            }
            break;
        }
    }

    return retval;        
}

/* static functions ***************************************************/

static bool isInteger(char c, uint8_t *out)
{
    ASSERT(out != NULL)
    
    bool retval = false;

    if((c >= '0') && (c <= '9')){

        *out = (uint8_t)(c - '0');
        retval = true;
    }

    return retval;
}

static bool isHexInteger(char c, uint8_t *out)
{
    ASSERT(out != NULL)
    
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
    ASSERT(in != NULL)
    ASSERT(read != NULL)
    ASSERT(out != NULL);
    ASSERT(outLen != NULL);

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
    ASSERT(in != NULL)
    ASSERT(read != NULL)
    ASSERT(out != NULL);
    ASSERT(outLen != NULL);

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
    ASSERT(in != NULL)
    ASSERT(read != NULL)
    ASSERT(out != NULL)

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

                    ERROR("too many digits for a 64bit number")
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
    ASSERT(in != NULL)
    ASSERT(read != NULL)
    ASSERT(out != NULL)

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
                    ERROR("too many digits for 64 bit number")
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
