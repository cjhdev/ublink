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
 * 
 * */

/* includes ***********************************************************/

#include "blink_compact.h"
#include "blink_debug.h"
#include "blink_stream.h"

#include <string.h>

/* static function prototypes *****************************************/

static bool encodeVLC(uint64_t in, bool isSigned, blink_stream_t out);
static bool decodeVLC(blink_stream_t in, bool isSigned, uint64_t *out, bool *isNull);

/* functions **********************************************************/

bool BLINK_Compact_encodeNull(blink_stream_t out)
{
    uint8_t in = 0xc0U;
    return BLINK_Stream_write(out, &in, sizeof(in));
}

bool BLINK_Compact_encodePresent(blink_stream_t out)
{
    uint8_t in = 0x01U;
    return BLINK_Stream_write(out, &in, sizeof(in));
}

bool BLINK_Compact_decodeBool(blink_stream_t in, bool *out, bool *isNull)
{
    BLINK_ASSERT(out != NULL)

    uint64_t number;
    bool retval = false;
    
    if(decodeVLC(in, false, &number, isNull)){

        if(*isNull){

            retval = true;
        }
        else{        

            if(number <= (uint64_t)UINT8_MAX){
                
                if((number == 0x00U) || (number == 0x01U)){

                    *out = (number == 0x00U) ? false : true;
                    retval = true;
                }
                else{

                    BLINK_ERROR("W11: boolean must be 0x00 or 0x01")              
                }
            }
            else{

                BLINK_ERROR("W3: out of range")
            }
        }
    }

    return retval;
}

bool BLINK_Compact_decodeU8(blink_stream_t in, uint8_t *out, bool *isNull)
{
    BLINK_ASSERT(out != NULL)

    uint64_t number;
    bool retval = false;

    if(decodeVLC(in, false, &number, isNull)){

        if(*isNull){

            retval = true;
        }
        else{
        
            if(number <= (uint64_t)UINT8_MAX){
                
                *out = (uint8_t)number;
                retval = true;
            }
            else{

                BLINK_ERROR("W3: out of range")
            }
        }
    }

    return retval;
}

bool BLINK_Compact_decodeU16(blink_stream_t in, uint16_t *out, bool *isNull)
{
    BLINK_ASSERT(out != NULL)

    uint64_t number;
    bool retval = false;

    if(decodeVLC(in, false, &number, isNull)){

        if(*isNull){

            retval = true;
        }
        else{

            if(number <= (uint64_t)UINT16_MAX){

                *out = (uint16_t)number;
                retval = true;
            }
            else{

                BLINK_ERROR("W3: out of range")
            }
        }
    }

    return retval;
}

bool BLINK_Compact_decodeU32(blink_stream_t in, uint32_t *out, bool *isNull)
{
    BLINK_ASSERT(out != NULL)

    uint64_t number;
    bool retval = false;

    if(decodeVLC(in, false, &number, isNull)){

        if(*isNull){

            retval = true;
        }
        else{

            if(number <= UINT32_MAX){

                *out = (uint32_t)number;
                retval = true;
            }
            else{

                BLINK_ERROR("W3: out of range")
            }
        }
    }

    return retval;
}

bool BLINK_Compact_decodeU64(blink_stream_t in, uint64_t *out, bool *isNull)
{
    return decodeVLC(in, false, out, isNull);
}

bool BLINK_Compact_decodeI8(blink_stream_t in, int8_t *out, bool *isNull)
{
    BLINK_ASSERT(out != NULL)
    
    int64_t number;
    bool retval = false;

    if(decodeVLC(in, true, (uint64_t *)&number, isNull)){

        if(*isNull){

            retval = true;
        }
        else{

            if((number <= INT8_MAX) && (number >= INT8_MIN)){
                
                *out = (int8_t)number;
                retval = true;
            }
            else{

                BLINK_ERROR("W3: out of range")
            }
        }
    }

    return retval;
}

bool BLINK_Compact_decodeI16(blink_stream_t in, int16_t *out, bool *isNull)
{
    BLINK_ASSERT(out != NULL)
    
    int64_t number;
    bool retval = false;

    if(decodeVLC(in, true, (uint64_t *)&number, isNull)){

        if(*isNull){

            retval = true;
        }
        else{

            if((number <= INT16_MAX) && (number >= INT16_MIN)){

                *out = (int16_t)number;
                retval = true;
            }
            else{

                BLINK_ERROR("W3: out of range")
            }
        }
    }

    return retval;
}

bool BLINK_Compact_decodeI32(blink_stream_t in, int32_t *out, bool *isNull)
{
    BLINK_ASSERT(out != NULL)
    
    int64_t number;
    bool retval = false;

    if(decodeVLC(in, true, (uint64_t *)&number, isNull)){

        if(*isNull){

            retval = true;
        }
        else{

            if((number <= INT32_MAX) && (number >= INT32_MIN)){

                *out = (int32_t)number;
                retval = true;
            }
            else{

                BLINK_ERROR("W3: out of range")
            }
        }
    }

    return retval;
}

bool BLINK_Compact_decodeI64(blink_stream_t in, int64_t *out, bool *isNull)
{
    BLINK_ASSERT(out != NULL)
    
    return decodeVLC(in, true, (uint64_t *)out, isNull);
}

bool BLINK_Compact_decodeDecimal(blink_stream_t in, int64_t *mantissa, int8_t *exponent, bool *isNull)
{
    BLINK_ASSERT(mantissa != NULL)
    BLINK_ASSERT(exponent != NULL)

    bool retval = false;
    
    if(BLINK_Compact_decodeI8(in, exponent, isNull)){

        if(*isNull){

            retval = true;
        }
        else{

            if(BLINK_Compact_decodeI64(in, mantissa, isNull)){

                if(*isNull){

                    BLINK_ERROR("mantissa cannot be NULL")
                }
                else{

                    retval = true;
                }        
            }
        }
    }

    return retval;
}

bool BLINK_Compact_decodeF64(blink_stream_t in, double *out, bool *isNull)
{
    BLINK_ASSERT(out != NULL)

    bool retval = false;
    uint64_t result;

    if(BLINK_Compact_decodeU64(in, &result, isNull)){

        retval = true;

        if(*isNull == false){

            *out = (double)result;        
        }
    }

    return retval;
}

bool BLINK_Compact_decodePresent(blink_stream_t in, bool *out)
{
    BLINK_ASSERT(out != NULL)

    bool isNull;
    uint64_t value;
    bool retval = false;
    
    if(decodeVLC(in, false, &value, &isNull)){

        if(isNull || (value == 0x01U)){

            *out = (isNull) ? false : true;
            retval = true;
        }
        else{

            BLINK_ERROR("W9: presence flag must be 0xc0 or 0x01")
        }
    }

    return retval;
}

bool BLINK_Compact_encodeBool(bool in, blink_stream_t out)
{
    return encodeVLC((in ? 0x01U : 0x00U), false, out);
}

bool BLINK_Compact_encodeU8(uint8_t in, blink_stream_t out)
{
    return encodeVLC((uint64_t)in, false, out);
}

bool BLINK_Compact_encodeU16(uint16_t in, blink_stream_t out)
{
    return encodeVLC((uint64_t)in, false, out);
}

bool BLINK_Compact_encodeU32(uint32_t in, blink_stream_t out)
{
    return encodeVLC((uint64_t)in, false, out);
}

bool BLINK_Compact_encodeU64(uint64_t in, blink_stream_t out)
{
    return encodeVLC(in, false, out);
}

bool BLINK_Compact_encodeI8(int8_t in, blink_stream_t out)
{
    return encodeVLC((uint64_t)in, true, out);
}

bool BLINK_Compact_encodeI16(int16_t in, blink_stream_t out)
{
    return encodeVLC((uint64_t)in, true, out);
}

bool BLINK_Compact_encodeI32(int32_t in, blink_stream_t out)
{
    return encodeVLC((uint64_t)in, true, out);
}

bool BLINK_Compact_encodeI64(int64_t in, blink_stream_t out)
{
    return encodeVLC((uint64_t)in, true, out);
}

bool BLINK_Compact_encodeF64(double in, blink_stream_t out)
{
    uint64_t *value = (uint64_t *)&in;  /*lint !e740 !e9087 double cast to uint64_t */
    
    return encodeVLC(*value, false, out);
}

bool BLINK_Compact_encodeDecimal(int64_t mantissa, int8_t exponent, blink_stream_t out)
{
    bool retval = false;
    
    if(encodeVLC((uint64_t)exponent, true, out)){

        if(encodeVLC(mantissa, true, out)){

            retval = true;
        }
    }

    return retval;
}

uint8_t BLINK_Compact_sizeofUnsigned(uint64_t value)
{
    uint8_t retval;

    if(value <= 0x7fUL){
        retval = 1U;
    }
    else if(value <= 0x3fffUL){
        retval = 2U;            
    }
    else if(value <= 0xffffUL){
        retval = 3U;
    }
    else if(value <= 0xffffffUL){
        retval = 4U;
    }
    else if(value <= 0xffffffffUL){
        retval = 5U;
    }
    else if(value <= 0xffffffffffUL){
        retval = 6U;
    }
    else if(value <= 0xffffffffffffUL){
        retval = 7U;
    }
    else if(value <= 0xffffffffffffffUL){
        retval = 8U;
    }
    else{
        retval = 9U;
    }

    return retval;
}

uint8_t BLINK_Compact_sizeofSigned(int64_t value)
{
    uint8_t retval;

    if(value < 0){

        if(value >= -64){
            retval = 1U;
        }
        else if(value >= -8192){
            retval = 2U;
        }
        else if(value >= -32768){
            retval = 3U;
        }
        else if(value >= -8388608){
            retval = 4U;
        }
        else if(value >= -2147483648){
            retval = 5U;
        }            
        else if(value >= -549755813888){
            retval = 6U;
        }            
        else if(value >= -140737488355328){
            retval = 7U;
        }            
        else if(value >= -36028797018963968){
            retval = 8U;
        }            
        else{
            retval = 9U;
        }
    }
    else{

        if(value <= 0x3fL){
            retval = 1U;
        }
        else if(value <= 0x1fffL){
            retval = 2U;            
        }
        else if(value <= 0x7fffL){
            retval = 3U;
        }
        else if(value <= 0x7fffffL){
            retval = 4U;
        }
        else if(value <= 0x7fffffffL){
            retval = 5U;
        }
        else if(value <= 0x7fffffffffL){
            retval = 6U;
        }
        else if(value <= 0x7fffffffffffL){
            retval = 7U;
        }
        else if(value <= 0x7fffffffffffffL){
            retval = 8U;
        }
        else{
            retval = 9U;
        }
    }

    return retval;
}

/* static functions ***************************************************/

static bool encodeVLC(uint64_t in, bool isSigned, blink_stream_t out)
{
    uint8_t buffer[9U];
    uint8_t bytes = (isSigned) ? BLINK_Compact_sizeofSigned((int64_t)in) : BLINK_Compact_sizeofUnsigned(in);
    uint8_t i;

    if(bytes == 1U){

        *buffer = (uint8_t)(in & 0x7fU);
    }
    else if(bytes == 2U){

        buffer[0] = 0x80U | (uint8_t)(in & 0x3fU);
        buffer[1] = (uint8_t)(in >> 6);   
    }
    else{
        
        buffer[0] = 0xC0U | (bytes-1U);
        for(i=1; i < bytes; i++){

            buffer[i] = (uint8_t)(in >> ((i-1U)*8U));
        }            
    }
    
    return BLINK_Stream_write(out, buffer, bytes);    
}

static bool decodeVLC(blink_stream_t in, bool isSigned, uint64_t *out, bool *isNull)
{
    BLINK_ASSERT(out != NULL)
    BLINK_ASSERT(isNull != NULL)

    uint8_t buffer[9U];
    bool retval = false;
    uint8_t bytes;
    uint8_t i;

    *isNull = false;

    if(BLINK_Stream_read(in, buffer, 1U)){

        if(buffer[0] < 0xc0U){

            if(buffer[0] < 0x80U){

                if(isSigned && ((buffer[0] & 0x40U) == 0x40U)){

                    *out = 0xffffffffffffffc0U;                           
                }
                else{
                    
                    *out = 0x0U;
                }
                *out |= (uint64_t)(buffer[0] & 0x7fU);
                retval = true;  
            }
            else{

                if(BLINK_Stream_read(in, &buffer[1], 1U)){

                    if(isSigned && ((buffer[1] & 0x80U) == 0x80U)){

                        *out = 0xffffffffffffff00U;                               
                    }
                    else{
                        
                        *out = 0x0U;
                    }
                    *out |= (uint64_t)buffer[1];
                    *out <<= 6U;
                    *out |= (uint64_t)(buffer[0] & 0x3fU);
                    retval = true;
                }
            }
        }
        else if(buffer[0] == 0xc0U){

            *isNull = true;
            retval = true;
        }
        else{

            bytes = buffer[0] & 0x3fU;

            if(bytes <= 8U){

                if(BLINK_Stream_read(in, &buffer[1], bytes)){

                    if(isSigned && ((buffer[bytes] & 0x80U) == 0x80U)){
                        
                        *out = 0xffffffffffff00U | buffer[bytes];
                    }
                    else{

                        *out = buffer[bytes];
                    }

                    for(i=bytes-1U; i != 0U; i--){

                        *out <<= 8;
                        *out |= buffer[i];                        
                    }

                    retval = true;
                }
            }
            else{

                /* VLC too large */
                BLINK_ERROR("cannot handle a VLC field larger than 8 bytes")
            }
        }
    }

    return retval;
}
