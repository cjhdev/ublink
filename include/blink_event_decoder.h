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
 
#ifndef BLINK_EVENT_DECODER_H
#define BLINK_EVENT_DECODER_H

/**
 * @defgroup blink_event_decoder blink_event_decoder
 * @ingroup ublink
 *
 * Event driven decoder
 * 
 * @{
 * */

#ifdef __cplusplus
extern "C" {
#endif

/* includes ***********************************************************/

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* forward declarations ***********************************************/

struct blink_schema;    /**< forward declaration */
struct blink_group;     /**< forward declaration */
struct blink_field;     /**< forward declaration */

/* typedefs ***********************************************************/

typedef void (* blink_error_fn_t)(void *user, uint32_t offset, const char *reason);

/** beginning of static group event handler
 *
 * @param[in] user
 * @param[in] group group definition
 * 
 * */
typedef void (* blink_begin_static_group_fn_t)(void *user, const struct blink_group *group);

/** end of static group event handler
 *
 * @param[in] user
 * @param[in] group group definition
 * 
 * */
typedef void (* blink_end_static_group_fn_t)(void *user, const struct blink_group *group);

/** beginning of dynamic group event handler
 *
 * @param[in] user
 * @param[in] group group definition
 * 
 * */
typedef void (* blink_begin_dynamic_group_fn_t)(void *user, const struct blink_group *group);

/** end of dynamic group event handler
 *
 * @param[in] user
 * @param[in] group group definition
 * 
 * */
typedef void (* blink_end_dynamic_group_fn_t)(void *user, const struct blink_group *group);

/** beginning of sequence handler
 *
 * @param[in] user
 * @param[in] numberOfElements number of elements to follow
 * 
 * */
typedef void (* blink_begin_sequence_fn_t)(void *user, uint32_t numberOfElements);

/** end of sequence handler
 *
 * @param[in] user
 * 
 * */
typedef void (* blink_end_sequence_fn_t)(void *user);

/** beginning of field handler
 *
 * @param[in] user
 * @param[in] name field name
 * @param[in] optional true if this field is optional
 * 
 * */
typedef void (* blink_begin_field_fn_t)(void *user, const char *name, bool optional);

/** end of field handler
 *
 * @param[in] user
 * @param[in] name field name
 * 
 * */
typedef void (* blink_end_field_fn_t)(void *user, const char *name);

/**
 * @param[in] user
 * @param[in] value     byte string
 * @param[in] valueLen  byte length of `value`
 *
 * */
typedef void (* blink_string_fn_t)(void *user, const uint8_t *value, uint32_t valueLen);

/**
 * @param[in] user
 * @param[in] value     byte string
 * @param[in] valueLen  byte length of `value`
 *
 * */
typedef void (* blink_binary_fn_t)(void *user, const uint8_t *value, uint32_t valueLen);

/**
 * @param[in] user
 * @param[in] value     byte string
 * @param[in] valueLen  byte length of `value`
 *
 * */
typedef void (* blink_fixed_fn_t)(void *user, const uint8_t *value, uint32_t valueLen);

/**
 * @param[in] user
 * @param[in] value `bool`
 *
 * */
typedef void (* blink_bool_fn_t)(void *user, bool value);

/**
 * @param[in] user
 * @param[in] value `u8`
 *
 * */
typedef void (* blink_u8_fn_t)(void *user, uint8_t value);

/**
 * @param[in] user
 * @param[in] value `u16`
 *
 * */
typedef void (* blink_u16_fn_t)(void *user, uint16_t value);

/**
 * @param[in] user
 * @param[in] value `u32`
 *
 * */
typedef void (* blink_u32_fn_t)(void *user, uint32_t value);

/**
 * @param[in] user
 * @param[in] value `u64`
 *
 * */
typedef void (* blink_u64_fn_t)(void *user, uint64_t value);

/**
 * @param[in] user
 * @param[in] value `i8`
 *
 * */
typedef void (* blink_i8_fn_t)(void *user, int8_t value);

/**
 * @param[in] user
 * @param[in] value `i16`
 *
 * */
typedef void (* blink_i16_fn_t)(void *user, int16_t value);

/**
 * @param[in] user
 * @param[in] value `i32`
 *
 * */
typedef void (* blink_i32_fn_t)(void *user, int32_t value);

/**
 * @param[in] user
 * @param[in] value `i64`
 *
 * */
typedef void (* blink_i64_fn_t)(void *user, int64_t value);

/**
 * @param[in] user
 * @param[in] value `f64`
 *
 * */
typedef void (* blink_float_fn_t)(void *user, double value);

/**
 * value = mantissa x 10 ^ exponent
 * 
 * @param[in] user
 * @param[in] mantissa `decimal` mantissa
 * @param[in] exponent `decimal` exponent
 *
 * */
typedef void (* blink_decimal_fn_t)(void *user, int64_t mantissa, int8_t exponent);

/**
 * @param[in] user
 * @param[in] name `enum` symbol name string
 *
 * */
typedef void (* blink_enum_fn_t)(void *user, const char *name);   /**< handle an `enum` */

/**
 * @param[in] user
 * @param[in] time `millitime`
 *
 * */
typedef void (* blink_milli_time_fn_t)(void *user, int64_t time);

/**
 * @param[in] user
 * @param[in] time `nanotime`
 *
 * */
typedef void (* blink_nano_time_fn_t)(void *user, int64_t time);

/**
 * @param[in] user
 * @param[in] timeOfDay `milliTimeOfDay`
 *
 * */   
typedef void (* blink_milli_timeofday_fn_t)(void *user, uint32_t timeOfDay);

/**
 * @param[in] user
 * @param[in] timeOfDay `nanoTimeOfDay`
 *
 * */   
typedef void (* blink_nano_timeofday_fn_t)(void *user, uint64_t timeOfDay);

/**
 * @param[in] user
 * @param[in] date `date`
 *
 * */   
typedef void (* blink_date_fn_t)(void *user, int64_t date);

/* structs ************************************************************/

/** Function pointers are called in response to events
 *
 * @note a function pointer may be NULL if the corresponding event is never handled
 *
 * */
struct blink_decoder_events {
    blink_string_fn_t string;
    blink_binary_fn_t binary;
    blink_fixed_fn_t fixed;
    blink_bool_fn_t boolean;
    blink_u8_fn_t u8;
    blink_u16_fn_t u16;
    blink_u32_fn_t u32;
    blink_u64_fn_t u64;
    blink_i8_fn_t i8;
    blink_i16_fn_t i16;
    blink_i32_fn_t i32;
    blink_i64_fn_t i64;
    blink_float_fn_t f64;
    blink_decimal_fn_t decimal;
    blink_enum_fn_t enumeration;
    blink_milli_time_fn_t millitime;
    blink_nano_time_fn_t nanotime;
    blink_milli_timeofday_fn_t timeOfDayMilli;
    blink_nano_timeofday_fn_t timeOfDayNano;
    blink_date_fn_t date;
    blink_begin_field_fn_t beginField;
    blink_end_field_fn_t endField;
    blink_begin_static_group_fn_t beginStaticGroup;
    blink_end_static_group_fn_t endStaticGroup;
    blink_begin_dynamic_group_fn_t beginDynamicGroup;
    blink_end_dynamic_group_fn_t endDynamicGroup;
    blink_begin_sequence_fn_t beginSequence;
    blink_end_sequence_fn_t endSequence;
};

/** decoder state */
struct blink_decoder {
        
    const struct blink_schema *schema;      /**< schema to decode against */
    void *user;                             /**< optional pointer to user state */ 
    struct blink_decoder_events events;     /**< event callbacks */
};

/* functions **********************************************************/

/**
 * Initialise an event driven decoder
 *
 * @param[in] decoder
 * @param[in] user optional user state passed to event handlers
 * @param[in] schema schema to decode against
 * @param[in] events event handlers to be copied into `self`
 *
 * @return pointer to initialised decoder
 * @retval NULL decoder could not be initialised
 * 
 * */
struct blink_decoder *BLINK_EventDecoderInit(struct blink_decoder *decoder, void *user, const struct blink_schema *schema, const struct blink_decoder_events *events);

/**
 * Decode a compact form group
 *
 * @param[in] self receiver
 * @param[in] in compact form string
 * @param[in] inLen byte length of `in`
 *
 * @return number of bytes successfully read from `in`
 * @retval 0..0xffffffff
 *
 * */
uint32_t BLINK_EventDecoderDecode(struct blink_decoder *self, const uint8_t *in, uint32_t inLen);

#ifdef __cplusplus
}
#endif

/** @} */
#endif
