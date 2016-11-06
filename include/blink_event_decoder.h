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
 * @defgroup blink_event_decoder
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

/* forward declarations ***********************************************/

struct blink_schema;    /**< forward declaration */
struct blink_group;     /**< forward declaration */
struct blink_field;     /**< forward declaration */

/* typedefs ***********************************************************/

typedef void (* begin_static_group_fn_t)(void *, const struct blink_group *);
typedef void (* end_static_group_fn_t)(void *, const struct blink_group *);
typedef void (* begin_dynamic_group_fn_t)(void *, const struct blink_group *);
typedef void (* end_dynamic_group_fn_t)(void *, const struct blink_group *);
typedef void (* begin_sequence_fn_t)(void *, uint32_t);
typedef void (* end_sequence_fn_t)(void *);
typedef void (* string_fn_t)(void *, const uint8_t *, uint32_t);    /**< handle a `string` */
typedef void (* binary_fn_t)(void *, const uint8_t *, uint32_t);    /**< handle a `binary` */
typedef void (* fixed_fn_t)(void *, const uint8_t *, uint32_t);     /**< handle a `fixed` */
typedef void (* bool_fn_t)(void *, bool);       /**< handle a `bool` */
typedef void (* u8_fn_t)(void *, uint8_t);      /**< handle a `u8` */
typedef void (* u16_fn_t)(void *, uint16_t);    /**< handle a `u16` */
typedef void (* u32_fn_t)(void *, uint32_t);    /**< handle a `u32` */
typedef void (* u64_fn_t)(void *, uint64_t);    /**< handle a `u64` */
typedef void (* i8_fn_t)(void *, int8_t);       /**< handle a `u8` */
typedef void (* i16_fn_t)(void *, int16_t);     /**< handle an `i16` */
typedef void (* i32_fn_t)(void *, int32_t);     /**< handle an `i32` */
typedef void (* i64_fn_t)(void *, int64_t);     /**< handle an `i64` */
typedef void (* float_fn_t)(void *, double);    /**< handle an `f64` */
typedef void (* decimal_fn_t)(void *, int64_t, int8_t); /**< handle a `decimal` */
typedef void (* enum_fn_t)(void *, const char *, uint16_t, uint64_t);   /**< handle an `enum` */
typedef void (* milli_time_fn_t)(void *, int64_t);  /**< handle a `millitime` */
typedef void (* nano_time_fn_t)(void *, int64_t);   /**< handle a `nanotime` */
typedef void (* milli_timeofday_fn_t)(void *, uint32_t);    /**< handle a `timeOfDayMilli` */
typedef void (* nano_timeofday_fn_t)(void *, uint64_t);     /**< handle a `timeOfDayNano` */
typedef void (* date_fn_t)(void *, int64_t);                /**< handle a `date` */

/* structs ************************************************************/

/** Function pointers are called in response to events
 *
 * @note a function pointer may be NULL if the corresponding event is never handled
 *
 * */
struct blink_decoder_events {
    string_fn_t string;             /**< `string` handler */
    binary_fn_t binary;             /**< `binary` handler */
    fixed_fn_t fixed;               /**< `fixed` handler */
    bool_fn_t bool;                 /**< `bool` handler */
    u8_fn_t u8;                     /**< `u8` handler */
    u16_fn_t u16;                   /**< `u16` handler */
    u32_fn_t u32;                   /**< `u32` handler */
    u64_fn_t u64;                   /**< `u64` handler */
    i8_fn_t i8;                     /**< `i8` handler */
    i16_fn_t i16;                   /**< `i16` handler */
    i32_fn_t i32;                   /**< `i32` handler */
    i64_fn_t i64;                   /**< `i64` handler */
    float_fn_t f64;                 /**< `f64` handler */
    decimal_fn_t decimal;           /**< `decimal` handler */
    milli_time_fn_t millitime;      /**< `millitime` handler */
    nano_time_fn_t nanotime;        /**< `nanotime` handler */
    milli_time_fn_t timeOfDayMilli; /**< `timeOfDayMilli` handler */
    nano_time_fn_t timeOfDayNano;   /**< `timeOfDayNano` handler */
    date_fn_t date;                 /**< `date` handler */

    begin_static_group_fn_t beginStaticGroup;   /**< handler for beginning of a static group */
    end_static_group_fn_t endStaticGroup;       /**< handler for end of a static group */

    begin_dynamic_group_fn_t beginDynamicGroup; /**< handler for beginning of a dynamic group */
    end_dynamic_group_fn_t endDynamicGroup;     /**< handler for end of a dynamic group */

    begin_sequence_fn_t beginSequence;          /**< handler for beginning of a sequence */
    end_sequence_fn_t endSequence;              /**< handler for end of a sequence */    
};

/** decoder state
 *
 * @private */
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
struct blink_decoder *BLINK_InitEventDecoder(struct blink_decoder *decoder, void *user, const struct blink_schema *schema, const struct blink_decoder_events *events);

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
