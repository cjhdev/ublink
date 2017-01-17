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

#ifndef BLINK_STREAM_H
#define BLINK_STREAM_H

/**
 * @defgroup blink_stream blink_stream
 * @ingroup ublink
 *
 * Configurable IO streams.
 * 
 * @{
 *
 * */

 #ifdef __cplusplus
extern "C" {
#endif

/* includes ***********************************************************/

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/* structs ************************************************************/

typedef bool (*blink_stream_read_t)(void *state, uint8_t *out, size_t bytesToRead);
typedef bool (*blink_stream_write_t)(void *state, const uint8_t *in, size_t bytesToWrite);

struct blink_stream {
    enum blink_stream_type {
        BLINK_STREAM_NULL = 0,        /**< uninitialised */
        BLINK_STREAM_BUFFER,          /**< buffer stream */
        BLINK_STREAM_USER,            /**< user stream */
    } type;
    union blink_stream_state {
        struct {
            const uint8_t *in;  /**< readable buffer */
            uint8_t *out;       /**< writeable buffer */
            uint32_t max;       /**< maximum size of buffer */
            uint32_t pos;        /**< current position */
        } buffer;
        struct {
            blink_stream_read_t reader;
            blink_stream_write_t writer;
            void *state;
        } user;
    } value;
};

typedef struct blink_stream * blink_stream_t;


/* function prototypes ************************************************/

/** Write to an output stream
 *
 * @param[in] self output stream
 * @param[in] in buffer to write to stream
 * @param[in] inLen byte length of buffer to write to stream
 *
 * @return `bytesToWrite` bytes were successfully written
 * @retval true
 * @retval false
 *
 * */
bool BLINK_Stream_write(blink_stream_t self, const uint8_t *in, size_t bytesToWrite);

/** Read from an input stream
 *
 * @param[in] self input stream
 * @param[out] out buffer of at least `bytesToRead` bytes
 * @param[in] bytesToRead number of bytes to read from stream into buffer
 *
 * @return `bytesToRead` bytes were successfully read 
 * @retval true
 * @retval false
 *
 * */
bool BLINK_Stream_read(blink_stream_t self, uint8_t *out, size_t bytesToRead);

/** Init a read only buffer
 *
 * @param[in] self
 * @param[in] buf buffer
 * @param[in] max length of buffer
 *
 * @return generic stream
 * 
 * */
blink_stream_t BLINK_Stream_initBufferReadOnly(struct blink_stream *self, const uint8_t *buf, uint32_t max);

/** Init a read/write buffer
 *
 * @param[in] self
 * @param[in] buf buffer
 * @param[in] max length of buffer
 *
 * @return generic stream
 * 
 * */
blink_stream_t BLINK_Stream_initBuffer(struct blink_stream *self, uint8_t *buf, uint32_t max);

/** Init a user defined stream
 *
 * @param[in] self
 * @param[in] user defined state
 * @param[in] reader read handler
 * @param[in] writer write handler
 *
 * @return generic stream
 * 
 * */
blink_stream_t BLINK_Stream_initUser(struct blink_stream *self, void *state, blink_stream_read_t reader, blink_stream_write_t writer);

/** Get current position
 *
 * @param[in] self
 * @return stream position from origin
 *
 * */
uint32_t BLINK_Stream_tell(blink_stream_t self);

/** Duplicate stream state
 *
 * @param[in] self new stream structure
 * @param[in] existing stream state to duplicate
 *
 * @return generic stream (duplicate of existing)
 *
 * @retval NULL existing could not be duplicated
 *
 * */
blink_stream_t BLINK_Stream_dup(struct blink_stream *self, blink_stream_t existing);

/** Set position to offset
 *
 * i.e. seek relative to start of stream
 * 
 * @param[in] self
 * @param[in] offset byte offset from origin
 *
 * @return true if position could be set to offset
 *
 * */
bool BLINK_Stream_seekSet(blink_stream_t self, uint32_t offset);

/** Add offset to current position
 *
 * i.e. seek relative to current stream position
 *
 * @param[in] self
 * @param[in] offset byte offset to add to current position
 * 
 * @return true if position could be modified by offset
 *
 * */
bool BLINK_Stream_seekCur(blink_stream_t self, int32_t offset);

#ifdef __cplusplus
}
#endif

/** @} */

#endif
