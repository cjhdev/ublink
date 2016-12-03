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

#include <string.h>

/* functions **********************************************************/

uint8_t BLINK_EncodeVLCNull(uint8_t *out, uint32_t outMax)
{
    uint8_t retval = 0U;

    if(outMax > 0U){

        *out = 0xC0U;
        retval = 1U;
    }

    return retval;
}

uint8_t BLINK_EncodePresent(uint8_t *out, uint32_t outMax)
{
    uint8_t retval = 0U;

    if(outMax > 0U){

        *out = 0x01U;
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

uint8_t BLINK_EncodeVLC(uint64_t in, bool isSigned, uint8_t *out, uint32_t outMax)
{
    BLINK_ASSERT(out != NULL)

    uint8_t bytes = (isSigned) ? BLINK_GetVLCSizeSigned((int64_t)in) : BLINK_GetVLCSizeUnsigned(in);
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

uint8_t BLINK_DecodeVLC(const uint8_t *in, uint32_t inLen, bool isSigned, uint64_t *out, bool *isNull)
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

uint8_t BLINK_DecodeBool(const uint8_t *in, uint32_t inLen, bool *out, bool *isNull)
{
    BLINK_ASSERT(out != NULL)

    uint64_t number;
    uint8_t retval;
    retval = BLINK_DecodeVLC(in, inLen, false, &number, isNull);
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

uint8_t BLINK_DecodeU8(const uint8_t *in, uint32_t inLen, uint8_t *out, bool *isNull)
{
    BLINK_ASSERT(out != NULL)

    uint64_t number;
    uint8_t retval;
    retval = BLINK_DecodeVLC(in, inLen, false, &number, isNull);
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

uint8_t BLINK_DecodeU16(const uint8_t *in, uint32_t inLen, uint16_t *out, bool *isNull)
{
    BLINK_ASSERT(out != NULL)
    
    uint64_t number;
    uint8_t retval;
    retval = BLINK_DecodeVLC(in, inLen, false, &number, isNull);
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

uint8_t BLINK_DecodeU32(const uint8_t *in, uint32_t inLen, uint32_t *out, bool *isNull)
{
    BLINK_ASSERT(out != NULL)
    
    uint64_t number;
    uint8_t retval;
    retval = BLINK_DecodeVLC(in, inLen, false, &number, isNull);
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

uint8_t BLINK_DecodeU64(const uint8_t *in, uint32_t inLen, uint64_t *out, bool *isNull)
{
    return BLINK_DecodeVLC(in, inLen, false, out, isNull);
}

uint8_t BLINK_DecodeI8(const uint8_t *in, uint32_t inLen, int8_t *out, bool *isNull)
{
    BLINK_ASSERT(out != NULL)
    
    int64_t number;
    uint8_t retval;
    retval = BLINK_DecodeVLC(in, inLen, true, (uint64_t *)&number, isNull);
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

uint8_t BLINK_DecodeI16(const uint8_t *in, uint32_t inLen, int16_t *out, bool *isNull)
{
    BLINK_ASSERT(out != NULL)

    int64_t number;
    uint8_t retval;
    retval = BLINK_DecodeVLC(in, inLen, true, (uint64_t *)&number, isNull);
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

uint8_t BLINK_DecodeI32(const uint8_t *in, uint32_t inLen, int32_t *out, bool *isNull)
{
    BLINK_ASSERT(out != NULL)
    
    int64_t number;
    uint8_t retval;
    retval = BLINK_DecodeVLC(in, inLen, true, (uint64_t *)&number, isNull);
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

uint8_t BLINK_DecodeI64(const uint8_t *in, uint32_t inLen, int64_t *out, bool *isNull)
{
    BLINK_ASSERT(out != NULL)
    
    return BLINK_DecodeVLC(in, inLen, true, (uint64_t *)out, isNull);
}

uint32_t BLINK_DecodeBinary(const uint8_t *in, uint32_t inLen, const uint8_t **out, uint32_t *outLen, bool *isNull)
{
    BLINK_ASSERT(out != NULL)
    BLINK_ASSERT(outLen != NULL)
    
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

uint32_t BLINK_DecodeString(const uint8_t *in, uint32_t inLen, const uint8_t **out, uint32_t *outLen, bool *isNull)
{
    return BLINK_DecodeBinary(in, inLen, out, outLen, isNull);
}

uint32_t BLINK_DecodeFixed(const uint8_t *in, uint32_t inLen, const uint8_t **out, uint32_t size)
{
    BLINK_ASSERT(out != NULL)
    
    uint32_t retval = 0U;

    if(inLen >= size){

        *out = in;
        retval = size;
    }
    else{

        BLINK_ERROR("S1: EOF")
    }

    return retval;
}    

uint32_t BLINK_DecodeOptionalFixed(const uint8_t *in, uint32_t inLen, const uint8_t **out, uint32_t size, bool *isNull)
{
    BLINK_ASSERT(out != NULL)

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

uint8_t BLINK_DecodeDecimal(const uint8_t *in, uint32_t inLen, int64_t *mantissa, int8_t *exponent, bool *isNull)
{
    BLINK_ASSERT(mantissa != NULL)
    BLINK_ASSERT(exponent != NULL)

    uint8_t retval;
    uint8_t pos;
    
    retval = BLINK_DecodeI8(in, inLen, exponent, isNull);

    if((retval > 0U) && (*isNull == false)){

        pos = retval;

        retval = BLINK_DecodeI64(&in[pos], inLen - pos, mantissa, isNull);

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

uint8_t BLINK_DecodeF64(const uint8_t *in, uint32_t inLen, double *out, bool *isNull)
{
    BLINK_ASSERT(out != NULL)

    uint8_t retval;
    uint64_t result;

    retval = BLINK_DecodeU64(in, inLen, &result, isNull);

    if((retval > 0U) && (*isNull == false)){

        *out = (double)result;        
    }

    return retval;
}

uint8_t BLINK_EncodeBool(bool in, uint8_t *out, uint32_t outMax)
{
    return BLINK_EncodeVLC((in ? 0x01U : 0x00U), false, out, outMax);
}

uint8_t BLINK_EncodeU8(uint8_t in, uint8_t *out, uint32_t outMax)
{
    return BLINK_EncodeVLC((uint64_t)in, false, out, outMax);
}

uint8_t BLINK_EncodeU16(uint16_t in, uint8_t *out, uint32_t outMax)
{
    return BLINK_EncodeVLC((uint64_t)in, false, out, outMax);
}

uint8_t BLINK_EncodeU32(uint32_t in, uint8_t *out, uint32_t outMax)
{
    return BLINK_EncodeVLC((uint64_t)in, false, out, outMax);
}

uint8_t BLINK_EncodeU64(uint64_t in, uint8_t *out, uint32_t outMax)
{
    return BLINK_EncodeVLC(in, false, out, outMax);
}

uint8_t BLINK_EncodeI8(int8_t in, uint8_t *out, uint32_t outMax)
{
    return BLINK_EncodeVLC((uint64_t)in, true, out, outMax);
}

uint8_t BLINK_EncodeI16(int16_t in, uint8_t *out, uint32_t outMax)
{
    return BLINK_EncodeVLC((uint64_t)in, true, out, outMax);
}

uint8_t BLINK_EncodeI32(int32_t in, uint8_t *out, uint32_t outMax)
{
    return BLINK_EncodeVLC((uint64_t)in, true, out, outMax);
}

uint8_t BLINK_EncodeI64(int64_t in, uint8_t *out, uint32_t outMax)
{
    return BLINK_EncodeVLC(in, true, out, outMax);
}

uint32_t BLINK_EncodeBinary(const uint8_t *in, uint32_t inLen, uint8_t *out, uint32_t outMax)
{
    uint32_t retval = 0U;
    uint32_t ret;
    
    ret = BLINK_EncodeVLC((uint64_t)inLen, false, out, outMax);

    /* fail safe if (inLen + ret) > UINT32_MAX */
    if((ret > 0U) && ((outMax - ret) >= inLen)){

        memcpy(&out[ret], in, inLen);
        retval = ret + inLen;
    }

    return retval;
}

uint32_t BLINK_EncodeString(const uint8_t *in, uint32_t inLen, uint8_t *out, uint32_t outMax)
{
    return BLINK_EncodeBinary(in, inLen, out, outMax);
}

uint32_t BLINK_EncodeFixed(const uint8_t *in, uint32_t inLen, uint8_t *out, uint32_t outMax)
{
    BLINK_ASSERT(in != NULL)
    BLINK_ASSERT(out != NULL)

    uint32_t retval = 0U;

    if(outMax >= inLen){

        memcpy(out, in, inLen);
        retval = inLen;
    }

    return retval;
}

uint32_t BLINK_EncodeOptionalFixed(const uint8_t *in, uint32_t inLen, uint8_t *out, uint32_t outMax)
{
    BLINK_ASSERT(in != NULL)
    BLINK_ASSERT(out != NULL)

    uint32_t retval = 0U;

    /* fail safe if (inLen + 1) > UINT32_MAX */
    if((outMax > 0U) && ((outMax - 1U) >= inLen)){

        out[0] = 0x01U;
        memcpy(&out[1], in, inLen);
        retval = inLen + 1U;
    }
        
    return retval;
}

uint8_t BLINK_EncodeF64(double in, uint8_t *out, uint32_t outMax)
{
    /*lint -e740 -e9087
     *
     * A double is cast as a uint64 so that it can be passed as input BLINK_EncodeVLC.
     *
     * This will cause problems on targets where double and uint64_t do not have the
     * same size and alignment.
     *
     * */
    uint64_t *value = (uint64_t *)&in;
    
    return BLINK_EncodeVLC(*value, false, out, outMax);
}

uint8_t BLINK_EncodeDecimal(int64_t mantissa, int8_t exponent, uint8_t *out, uint32_t outMax)
{
    uint8_t retval = 0U;
    uint8_t ret;
    uint8_t pos;

    ret = BLINK_EncodeVLC((int64_t)exponent, true, out, outMax);

    if(ret > 0U){

        pos = ret;
        
        ret = BLINK_EncodeVLC(mantissa, true, &out[pos], outMax - pos);

        if(ret > 0U){

            retval = pos + ret;
        }
    }

    return retval;
}
