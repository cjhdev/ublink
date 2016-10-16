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

#include "blink_compact_form.h"
#include "blink_debug.h"

/* functions **********************************************************/

uint8_t BLINK_EncodeVLCNull(uint8_t *out, uint32_t outMax)
{
    uint8_t retval = 0U;

    if(outMax > 0){

        *out = 0xC0U;
        retval = 1U;
    }

    return retval;
}

uint8_t BLINK_GetVLCSizeUnsigned(uint64_t value)
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

uint8_t BLINK_GetVLCSizeSigned(int64_t value)
{
    uint8_t retval;
    uint8_t i;

    if(value < 0){

        if(value >= -64){

            retval = 1U;
        }
        else if(value >= -8192){

            retval = 2U;
        }
        else{

            int64_t min = -32768;
            retval = 3U;

            for(i=0; i < 6; i++){

                if((min << (i*8)) <= value){

                    break;
                }
                else{

                    retval++;
                }
            }
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

uint8_t BLINK_EncodeVLC(uint64_t in, bool isSigned, uint8_t *out, uint32_t outMax)
{
    uint8_t bytes = (isSigned) ? BLINK_GetVLCSizeSigned((int64_t)in) : BLINK_GetVLCSizeUnsigned(in);
    uint8_t retval = 0U;
    uint8_t i;

    if(outMax >= bytes){

        if(bytes == 1){

            *out = (uint8_t)(in & 0x7f);
        }
        else if(bytes == 2){

            out[0] = 0x80 | (uint8_t)(in & 0x3f);
            out[1] = (uint8_t)(in >> 6);   
        }
        else{
            
            out[0] = 0xC0U | (bytes-1);
            for(i=1; i < bytes; i++){

                out[i] = (uint8_t)(in >> ((i-1)*8));
            }            
        }
        retval = bytes;
    }

    return retval;
}

uint8_t BLINK_DecodeVLC(const uint8_t *in, uint32_t inLen, bool isSigned, uint64_t *out, bool *isNull)
{
    uint8_t retval = 0U;
    uint8_t bytes;
    uint8_t i;

    *isNull = false;

    if(inLen > 0){

        if(*in < 0xc0){

            if(*in < 0x80){

                if(isSigned && ((*in & 0x40) == 0x40)){

                    *out = 0xffffffffffffffc0;                           
                }
                else{
                    
                    *out = 0x0;
                }
                *out |= (uint64_t)(*in & 0x7f);
                retval = 1U;  
            }
            else{

                if(inLen >= 2){
                    
                    if(isSigned && ((in[1] & 0x80) == 0x80)){

                        *out = 0xffffffffffffff00;                               
                    }
                    else{
                        
                        *out = 0x0;
                    }
                    *out |= (uint64_t)in[1];
                    *out <<= 6;
                    *out |= (uint64_t)(in[0] & 0x3fU);
                    retval = 2U;
                }
            }
        }
        else if(*in == 0xc0){

            *isNull = true;
            retval = 1U;
        }
        else{

            bytes = *in & 0x3fU;

            if(inLen >= (1U + bytes)){

                if(bytes <= 8U){

                    if(isSigned && ((in[bytes] & 0x80U) == 0x80U)){
                        
                        *out = 0xffffffffffff00U | in[bytes];
                    }
                    else{

                        *out = in[bytes];
                    }

                    for(i=bytes-1; i != 0U; i--){

                        *out <<= 8;
                        *out |= in[i];                        
                    }

                    retval = bytes + 1U;                    
                }
            }
        }
    }

    return retval;
}

uint8_t BLINK_DecodeBool(const uint8_t *in, uint32_t inLen, bool *out, bool *isNull)
{
    uint64_t number;
    uint8_t retval;
    retval = BLINK_DecodeVLC(in, inLen, false, &number, isNull);
    if((retval > 0U) && (*isNull == false)){

        if(number <= UINT8_MAX){
            
            if((number == 0x00) || (number == 0x01)){

                *out = (number == 0x00) ? false : true;
            }
            else{

                ERROR("W11: boolean must be 0x00 or 0x01")
                retval = 0U;                
            }
        }
        else{

            ERROR("W3: out of range")
            retval = 0U;
        }
    }

    return retval;
}

uint8_t BLINK_DecodeU8(const uint8_t *in, uint32_t inLen, uint8_t *out, bool *isNull)
{
    uint64_t number;
    uint8_t retval;
    retval = BLINK_DecodeVLC(in, inLen, false, &number, isNull);
    if((retval > 0U) && (*isNull == false)){

        if(number <= UINT8_MAX){
            
            *out = (uint8_t)number;
        }
        else{

            ERROR("W3: out of range")
            retval = 0U;
        }
    }

    return retval;
}

uint8_t BLINK_DecodeU16(const uint8_t *in, uint32_t inLen, uint16_t *out, bool *isNull)
{
    uint64_t number;
    uint8_t retval;
    retval = BLINK_DecodeVLC(in, inLen, false, &number, isNull);
    if((retval > 0U) && (*isNull == false)){

        if(number <= UINT16_MAX){

            *out = (uint16_t)number;
        }
        else{

            ERROR("W3: out of range")
            retval = 0U;
        }
    }

    return retval;
}

uint8_t BLINK_DecodeU32(const uint8_t *in, uint32_t inLen, uint32_t *out, bool *isNull)
{
    uint64_t number;
    uint8_t retval;
    retval = BLINK_DecodeVLC(in, inLen, false, &number, isNull);
    if((retval > 0U) && (*isNull == false)){

        if(number <= UINT32_MAX){

            *out = (uint32_t)number;
        }
        else{

            ERROR("W3: out of range")
            retval = 0U;
        }
    }

    return retval;
}

uint8_t BLINK_DecodeU64(const uint8_t *in, uint32_t inLen, uint64_t *out, bool *isNull)
{
    return BLINK_DecodeVLC(in, inLen, false, out, isNull);
}

uint8_t BLINK_DecodeI8(const uint8_t *in, uint32_t inLen, int8_t *out, bool *isNull)
{
    int64_t number;
    uint8_t retval;
    retval = BLINK_DecodeVLC(in, inLen, true, (uint64_t *)&number, isNull);
    if((retval > 0U) && (*isNull == false)){

        if((number <= INT8_MAX) && (number >= INT8_MIN)){
            
            *out = (int8_t)number;
        }
        else{

            ERROR("W3: out of range")
            retval = 0U;
        }
    }

    return retval;
}

uint8_t BLINK_DecodeI16(const uint8_t *in, uint32_t inLen, int16_t *out, bool *isNull)
{
    int64_t number;
    uint8_t retval;
    retval = BLINK_DecodeVLC(in, inLen, true, (uint64_t *)&number, isNull);
    if((retval > 0U) && (*isNull == false)){

        if((number <= INT16_MAX) && (number >= INT16_MIN)){

            *out = (int16_t)number;
        }
        else{

            ERROR("W3: out of range")
            retval = 0U;
        }
    }

    return retval;
}

uint8_t BLINK_DecodeI32(const uint8_t *in, uint32_t inLen, int32_t *out, bool *isNull)
{
    int64_t number;
    uint8_t retval;
    retval = BLINK_DecodeVLC(in, inLen, true, (uint64_t *)&number, isNull);
    if((retval > 0U) && (*isNull == false)){

        if((number <= INT32_MAX) && (number >= INT32_MIN)){

            *out = (int32_t)number;
        }
        else{

            ERROR("W3: out of range")
            retval = 0U;
        }
    }

    return retval;
}

uint8_t BLINK_DecodeI64(const uint8_t *in, uint32_t inLen, int64_t *out, bool *isNull)
{
    return BLINK_DecodeVLC(in, inLen, true, (uint64_t *)out, isNull);
}

uint32_t BLINK_DecodeBinary(const uint8_t *in, uint32_t inLen, const uint8_t **out, uint32_t *outLen, bool *isNull)
{
    uint64_t size;
    uint32_t retval;
    retval = BLINK_DecodeVLC(in, inLen, false, &size, isNull);
    if((retval > 0U) && (*isNull == false)){

        if(size <= UINT32_MAX){

            if((inLen - retval) >= size){
    
                *out = &in[retval];
                *outLen = (uint32_t)size;
                retval += (uint32_t)size;
            }
            else{

                ERROR("S1: EOF")
                retval = 0U;
            }
        }
        else{

            retval = 0U;
        }
    }

    return retval;
}

uint32_t BLINK_DecodeString(const uint8_t *in, uint32_t inLen, const uint8_t **out, uint32_t *outLen, bool *isNull)
{
    return BLINK_DecodeBinary(in, inLen, out, outLen, isNull);
}

uint32_t BLINK_DecodeFixed(const uint8_t *in, uint32_t inLen, const uint8_t **out, uint32_t size)
{
    uint32_t retval = 0U;

    if(inLen >= size){

        *out = in;
        retval = size;
    }
    else{

        ERROR("S1: EOF")
    }

    return retval;
}    

uint32_t BLINK_DecodeOptionalFixed(const uint8_t *in, uint32_t inLen, const uint8_t **out, uint32_t size, bool *isNull)
{
    uint8_t present;
    uint32_t retval;

    retval = BLINK_DecodeU8(in, inLen, &present, isNull);

    if((retval > 0U) && (*isNull == false)){

        if(present == 0x01U){

            if((inLen - retval) >= size){
    
                *out = &in[retval];
                retval += (uint32_t)size;
            }
            else{

                ERROR("S1: EOF")
                retval = 0U;
            }
        }
        else{

            ERROR("W9: presence flag must be 0xc0 or 0x01")
            retval = 0U;
        }
    }

    return retval;
}

uint8_t BLINK_DecodeDecimal(const uint8_t *in, uint32_t inLen, int64_t *mantissa, int8_t *exponent, bool *isNull)
{
    uint8_t retval;
    uint8_t pos;
    
    retval = BLINK_DecodeI8(in, inLen, exponent, isNull);

    if((retval > 0U) && (*isNull == false)){

        pos = retval;

        retval = BLINK_DecodeI64(&in[pos], inLen - pos, mantissa, isNull);

        if(retval > 0){

            if(*isNull){

                ERROR("mantissa cannot be NULL")
                retval = 0U;
            }
            else{

                retval += pos;
            }        
        }        
    }

    return retval;
}

uint8_t BLINK_DecodeF64(const uint8_t *in, uint32_t inLen, double *out, bool *isNull)
{
    uint8_t retval;
    uint64_t result;

    retval = BLINK_DecodeU64(in, inLen, &result, isNull);

    if((retval > 0U) && (*isNull == false)){

        *out = (double)result;        
    }

    return retval;
}

