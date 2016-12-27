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

#include <string.h>

/* functions **********************************************************/

uint8_t BLINK_Compact_encodeVLCNull(uint8_t *out, uint32_t outMax)
{
    uint8_t retval = 0U;

    if(outMax > 0U){

        *out = 0xC0U;
        retval = 1U;
    }

    return retval;
}

uint8_t BLINK_Compact_encodePresent(uint8_t *out, uint32_t outMax)
{
    uint8_t retval = 0U;

    if(outMax > 0U){

        *out = 0x01U;
        retval = 1U;
    }

    return retval;    
}

uint8_t BLINK_Compact_getVLCSizeUnsigned(uint64_t value)
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

uint8_t BLINK_Compact_getVLCSizeSigned(int64_t value)
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

uint8_t BLINK_Compact_encodeVLC(uint64_t in, bool isSigned, uint8_t *out, uint32_t outMax)
{
    BLINK_ASSERT(out != NULL)

    uint8_t bytes = (isSigned) ? BLINK_Compact_getVLCSizeSigned((int64_t)in) : BLINK_Compact_getVLCSizeUnsigned(in);
    uint8_t retval = 0U;
    uint8_t i;

    if(outMax >= bytes){

        if(bytes == 1U){

            *out = (uint8_t)(in & 0x7fU);
        }
        else if(bytes == 2U){

            out[0] = 0x80U | (uint8_t)(in & 0x3fU);
            out[1] = (uint8_t)(in >> 6);   
        }
        else{
            
            out[0] = 0xC0U | (bytes-1U);
            for(i=1; i < bytes; i++){

                out[i] = (uint8_t)(in >> ((i-1U)*8U));
            }            
        }
        retval = bytes;
    }

    return retval;
}

uint8_t BLINK_Compact_decodeVLC(const uint8_t *in, uint32_t inLen, bool isSigned, uint64_t *out, bool *isNull)
{
    BLINK_ASSERT(in != NULL)
    BLINK_ASSERT(out != NULL)
    BLINK_ASSERT(isNull != NULL)

    uint8_t retval = 0U;
    uint8_t bytes;
    uint8_t i;

    *isNull = false;

    if(inLen > 0U){

        if(*in < 0xc0U){

            if(*in < 0x80U){

                if(isSigned && ((*in & 0x40U) == 0x40U)){

                    *out = 0xffffffffffffffc0U;                           
                }
                else{
                    
                    *out = 0x0U;
                }
                *out |= (uint64_t)(*in & 0x7fU);
                retval = 1U;  
            }
            else{

                if(inLen >= 2U){
                    
                    if(isSigned && ((in[1] & 0x80U) == 0x80U)){

                        *out = 0xffffffffffffff00U;                               
                    }
                    else{
                        
                        *out = 0x0U;
                    }
                    *out |= (uint64_t)in[1];
                    *out <<= 6U;
                    *out |= (uint64_t)(in[0] & 0x3fU);
                    retval = 2U;
                }
            }
        }
        else if(*in == 0xc0U){

            *isNull = true;
            retval = 1U;
        }
        else{

            bytes = *in & 0x3fU;

            if(inLen >= (1U + (uint32_t)bytes)){

                if(bytes <= 8U){

                    if(isSigned && ((in[bytes] & 0x80U) == 0x80U)){
                        
                        *out = 0xffffffffffff00U | in[bytes];
                    }
                    else{

                        *out = in[bytes];
                    }

                    for(i=bytes-1U; i != 0U; i--){

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

uint8_t BLINK_Compact_decodeBool(const uint8_t *in, uint32_t inLen, bool *out, bool *isNull)
{
    BLINK_ASSERT(out != NULL)

    uint64_t number;
    uint8_t retval;
    retval = BLINK_Compact_decodeVLC(in, inLen, false, &number, isNull);
    if((retval > 0U) && (*isNull == false)){

        if(number <= (uint64_t)UINT8_MAX){
            
            if((number == 0x00U) || (number == 0x01U)){

                *out = (number == 0x00U) ? false : true;
            }
            else{

                BLINK_ERROR("W11: boolean must be 0x00 or 0x01")
                retval = 0U;                
            }
        }
        else{

            BLINK_ERROR("W3: out of range")
            retval = 0U;
        }
    }

    return retval;
}

uint8_t BLINK_Compact_decodeU8(const uint8_t *in, uint32_t inLen, uint8_t *out, bool *isNull)
{
    BLINK_ASSERT(out != NULL)

    uint64_t number;
    uint8_t retval;
    retval = BLINK_Compact_decodeVLC(in, inLen, false, &number, isNull);
    if((retval > 0U) && (*isNull == false)){

        if(number <= (uint64_t)UINT8_MAX){
            
            *out = (uint8_t)number;
        }
        else{

            BLINK_ERROR("W3: out of range")
            retval = 0U;
        }
    }

    return retval;
}

uint8_t BLINK_Compact_decodeU16(const uint8_t *in, uint32_t inLen, uint16_t *out, bool *isNull)
{
    BLINK_ASSERT(out != NULL)
    
    uint64_t number;
    uint8_t retval;
    retval = BLINK_Compact_decodeVLC(in, inLen, false, &number, isNull);
    if((retval > 0U) && (*isNull == false)){

        if(number <= (uint64_t)UINT16_MAX){

            *out = (uint16_t)number;
        }
        else{

            BLINK_ERROR("W3: out of range")
            retval = 0U;
        }
    }

    return retval;
}

uint8_t BLINK_Compact_decodeU32(const uint8_t *in, uint32_t inLen, uint32_t *out, bool *isNull)
{
    BLINK_ASSERT(out != NULL)
    
    uint64_t number;
    uint8_t retval;
    retval = BLINK_Compact_decodeVLC(in, inLen, false, &number, isNull);
    if((retval > 0U) && (*isNull == false)){

        if(number <= UINT32_MAX){

            *out = (uint32_t)number;
        }
        else{

            BLINK_ERROR("W3: out of range")
            retval = 0U;
        }
    }

    return retval;
}

uint8_t BLINK_Compact_decodeU64(const uint8_t *in, uint32_t inLen, uint64_t *out, bool *isNull)
{
    return BLINK_Compact_decodeVLC(in, inLen, false, out, isNull);
}

uint8_t BLINK_Compact_decodeI8(const uint8_t *in, uint32_t inLen, int8_t *out, bool *isNull)
{
    BLINK_ASSERT(out != NULL)
    
    int64_t number;
    uint8_t retval;
    retval = BLINK_Compact_decodeVLC(in, inLen, true, (uint64_t *)&number, isNull);
    if((retval > 0U) && (*isNull == false)){

        if((number <= INT8_MAX) && (number >= INT8_MIN)){
            
            *out = (int8_t)number;
        }
        else{

            BLINK_ERROR("W3: out of range")
            retval = 0U;
        }
    }

    return retval;
}

uint8_t BLINK_Compact_decodeI16(const uint8_t *in, uint32_t inLen, int16_t *out, bool *isNull)
{
    BLINK_ASSERT(out != NULL)

    int64_t number;
    uint8_t retval;
    retval = BLINK_Compact_decodeVLC(in, inLen, true, (uint64_t *)&number, isNull);
    if((retval > 0U) && (*isNull == false)){

        if((number <= INT16_MAX) && (number >= INT16_MIN)){

            *out = (int16_t)number;
        }
        else{

            BLINK_ERROR("W3: out of range")
            retval = 0U;
        }
    }

    return retval;
}

uint8_t BLINK_Compact_decodeI32(const uint8_t *in, uint32_t inLen, int32_t *out, bool *isNull)
{
    BLINK_ASSERT(out != NULL)
    
    int64_t number;
    uint8_t retval;
    retval = BLINK_Compact_decodeVLC(in, inLen, true, (uint64_t *)&number, isNull);
    if((retval > 0U) && (*isNull == false)){

        if((number <= INT32_MAX) && (number >= INT32_MIN)){

            *out = (int32_t)number;
        }
        else{

            BLINK_ERROR("W3: out of range")
            retval = 0U;
        }
    }

    return retval;
}

uint8_t BLINK_Compact_decodeI64(const uint8_t *in, uint32_t inLen, int64_t *out, bool *isNull)
{
    BLINK_ASSERT(out != NULL)
    
    return BLINK_Compact_decodeVLC(in, inLen, true, (uint64_t *)out, isNull);
}

uint32_t BLINK_Compact_decodeBinary(const uint8_t *in, uint32_t inLen, const uint8_t **out, uint32_t *outLen, bool *isNull)
{
    BLINK_ASSERT(out != NULL)
    BLINK_ASSERT(outLen != NULL)
    
    uint64_t size;
    uint32_t retval;
    retval = BLINK_Compact_decodeVLC(in, inLen, false, &size, isNull);
    if((retval > 0U) && (*isNull == false)){

        if(size <= UINT32_MAX){

            if((inLen - retval) >= size){
    
                *out = &in[retval];
                *outLen = (uint32_t)size;
                retval += (uint32_t)size;
            }
            else{

                BLINK_ERROR("S1: EOF")
                retval = 0U;
            }
        }
        else{

            retval = 0U;
        }
    }

    return retval;
}

uint32_t BLINK_Compact_decodeString(const uint8_t *in, uint32_t inLen, const uint8_t **out, uint32_t *outLen, bool *isNull)
{
    return BLINK_Compact_decodeBinary(in, inLen, out, outLen, isNull);
}

uint32_t BLINK_Compact_decodeFixed(const uint8_t *in, uint32_t inLen, const uint8_t **out, uint32_t size)
{
    BLINK_ASSERT(out != NULL)
    
    uint32_t retval = 0U;

    if(inLen >= size){

        *out = in;
        retval = size;
    }
    else{

        BLINK_ERROR("S1: EOF")  /* EOF */
    }

    return retval;
}    

uint32_t BLINK_Compact_decodeOptionalFixed(const uint8_t *in, uint32_t inLen, const uint8_t **out, uint32_t size, bool *isNull)
{
    BLINK_ASSERT(out != NULL)

    uint8_t present;
    uint32_t retval;

    retval = BLINK_Compact_decodeU8(in, inLen, &present, isNull);

    if((retval > 0U) && (*isNull == false)){

        if(present == 0x01U){

            if((inLen - retval) >= size){
    
                *out = &in[retval];
                retval += (uint32_t)size;
            }
            else{

                BLINK_ERROR("S1: EOF")
                retval = 0U;
            }
        }
        else{

            BLINK_ERROR("W9: presence flag must be 0xc0 or 0x01")
            retval = 0U;
        }
    }

    return retval;
}

uint8_t BLINK_Compact_decodeDecimal(const uint8_t *in, uint32_t inLen, int64_t *mantissa, int8_t *exponent, bool *isNull)
{
    BLINK_ASSERT(mantissa != NULL)
    BLINK_ASSERT(exponent != NULL)

    uint8_t retval;
    uint8_t pos;
    
    retval = BLINK_Compact_decodeI8(in, inLen, exponent, isNull);

    if((retval > 0U) && (*isNull == false)){

        pos = retval;

        retval = BLINK_Compact_decodeI64(&in[pos], inLen - pos, mantissa, isNull);

        if(retval > 0U){

            if(*isNull){

                BLINK_ERROR("mantissa cannot be NULL")
                retval = 0U;
            }
            else{

                retval += pos;
            }        
        }        
    }

    return retval;
}

uint8_t BLINK_Compact_decodeF64(const uint8_t *in, uint32_t inLen, double *out, bool *isNull)
{
    BLINK_ASSERT(out != NULL)

    uint8_t retval;
    uint64_t result;

    retval = BLINK_Compact_decodeU64(in, inLen, &result, isNull);

    if((retval > 0U) && (*isNull == false)){

        *out = (double)result;        
    }

    return retval;
}

uint8_t BLINK_Compact_decodePresent(const uint8_t *in, uint32_t inLen, bool *out)
{
    BLINK_ASSERT(out != NULL)

    bool isNull;
    uint64_t value;
    uint8_t retval = BLINK_Compact_decodeVLC(in, inLen, false, &value, &isNull);

    if(retval > 0U){

        if(isNull || (value == 0x01U)){

            *out = (isNull) ? false : true;            
        }
        else{

            BLINK_ERROR("W9: presence flag must be 0xc0 or 0x01")
            retval = 0U;
        }
    }

    return retval;
}

uint8_t BLINK_Compact_encodeBool(bool in, uint8_t *out, uint32_t outMax)
{
    return BLINK_Compact_encodeVLC((in ? 0x01U : 0x00U), false, out, outMax);
}

uint8_t BLINK_Compact_encodeU8(uint8_t in, uint8_t *out, uint32_t outMax)
{
    return BLINK_Compact_encodeVLC((uint64_t)in, false, out, outMax);
}

uint8_t BLINK_Compact_encodeU16(uint16_t in, uint8_t *out, uint32_t outMax)
{
    return BLINK_Compact_encodeVLC((uint64_t)in, false, out, outMax);
}

uint8_t BLINK_Compact_encodeU32(uint32_t in, uint8_t *out, uint32_t outMax)
{
    return BLINK_Compact_encodeVLC((uint64_t)in, false, out, outMax);
}

uint8_t BLINK_Compact_encodeU64(uint64_t in, uint8_t *out, uint32_t outMax)
{
    return BLINK_Compact_encodeVLC(in, false, out, outMax);
}

uint8_t BLINK_Compact_encodeI8(int8_t in, uint8_t *out, uint32_t outMax)
{
    return BLINK_Compact_encodeVLC((uint64_t)in, true, out, outMax);
}

uint8_t BLINK_Compact_encodeI16(int16_t in, uint8_t *out, uint32_t outMax)
{
    return BLINK_Compact_encodeVLC((uint64_t)in, true, out, outMax);
}

uint8_t BLINK_Compact_encodeI32(int32_t in, uint8_t *out, uint32_t outMax)
{
    return BLINK_Compact_encodeVLC((uint64_t)in, true, out, outMax);
}

uint8_t BLINK_Compact_encodeI64(int64_t in, uint8_t *out, uint32_t outMax)
{
    return BLINK_Compact_encodeVLC(in, true, out, outMax);
}

uint32_t BLINK_Compact_encodeBinary(const uint8_t *in, uint32_t inLen, uint8_t *out, uint32_t outMax)
{
    uint32_t retval = 0U;
    uint32_t ret;
        
    ret = BLINK_Compact_encodeVLC((uint64_t)inLen, false, out, outMax);

    /* fail safe if (inLen + ret) > UINT32_MAX */
    if((ret > 0U) && ((outMax - ret) >= inLen)){

        (void)memcpy(&out[ret], in, inLen);
        retval = ret + inLen;
    }
        
    return retval;
}

uint32_t BLINK_Compact_encodeString(const uint8_t *in, uint32_t inLen, uint8_t *out, uint32_t outMax)
{
    return BLINK_Compact_encodeBinary(in, inLen, out, outMax);
}

uint32_t BLINK_Compact_encodeFixed(const uint8_t *in, uint32_t inLen, uint8_t *out, uint32_t outMax)
{
    BLINK_ASSERT(in != NULL)
    BLINK_ASSERT(out != NULL)

    uint32_t retval = 0U;

    if(outMax >= inLen){

        if(in != out){
            (void)memcpy(out, in, inLen);
        }
        retval = inLen;
    }

    return retval;
}

uint32_t BLINK_Compact_encodeOptionalFixed(const uint8_t *in, uint32_t inLen, uint8_t *out, uint32_t outMax)
{
    BLINK_ASSERT(in != NULL)
    BLINK_ASSERT(out != NULL)

    uint32_t retval = 0U;

    /* fail safe if (inLen + 1) > UINT32_MAX */
    if((outMax > 0U) && ((outMax - 1U) >= inLen)){

        out[0] = 0x01U;
        (void)memcpy(&out[1], in, inLen);
        retval = inLen + 1U;
    }
        
    return retval;
}

uint8_t BLINK_Compact_encodeF64(double in, uint8_t *out, uint32_t outMax)
{
    uint64_t *value = (uint64_t *)&in;  /*lint !e740 !e9087 double cast to uint64_t */
    
    return BLINK_Compact_encodeVLC(*value, false, out, outMax);
}

uint8_t BLINK_Compact_encodeDecimal(int64_t mantissa, int8_t exponent, uint8_t *out, uint32_t outMax)
{
    uint8_t retval = 0U;
    uint8_t ret;
    uint8_t pos;

    ret = BLINK_Compact_encodeVLC((int64_t)exponent, true, out, outMax);

    if(ret > 0U){

        pos = ret;
        
        ret = BLINK_Compact_encodeVLC(mantissa, true, &out[pos], outMax - pos);

        if(ret > 0U){

            retval = pos + ret;
        }
    }

    return retval;
}


