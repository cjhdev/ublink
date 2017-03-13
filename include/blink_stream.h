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

/* types **************************************************************/

struct blink_stream_user {
    bool (*read)(void *state, void *out, size_t bytesToRead);
    bool (*write)(void *state, const void *in, size_t bytesToWrite);
    uint32_t (*tell)(void *state);
    bool (*peek)(void *state, void *c);
    bool (*seekCur)(void *state, int32_t offset);
    bool (*seekSet)(void *state, uint32_t offset);
    bool (*eof)(void *state);
};

struct blink_stream {
    enum blink_stream_type {
        BLINK_STREAM_NULL = 0,        /**< uninitialised */
        BLINK_STREAM_BUFFER,          /**< buffer stream */
        BLINK_STREAM_USER,            /**< user stream */
        BLINK_STREAM_BOUNDED          /**< bounded stream */
    } type;
    union blink_stream_state {
        struct {
            const uint8_t *in;  /**< readable buffer */
            uint8_t *out;       /**< writeable buffer */
            uint32_t max;       /**< maximum size of buffer */
            uint32_t pos;       /**< current position */
            bool eof;
        } buffer;
        struct {
            struct blink_stream_user fn;
            void *state;
        } user;
        struct {
            struct blink_stream *stream;
            uint32_t max;                   
            uint32_t pos;
            bool eof;
        } bounded;
    } value;
    
};

typedef struct blink_stream * blink_stream_t;


/* function prototypes ************************************************/

/** Write to a stream
 *
 * @param[in] self stream
 * @param[in] buf buffer to write
 * @param[in] nbyte number of bytes to write 
 *
 * @return true if successful
 *
 * */
bool BLINK_Stream_write(blink_stream_t self, const void *buf, size_t nbyte);

/** Read from a stream
 *
 * @param[in] self stream
 * @param[out] buf buffer of at least `nbyte` bytes
 * @param[in] nbyte number of bytes to read
 *
 * @return true if successful
 *
 * */
bool BLINK_Stream_read(blink_stream_t self, void *buf, size_t nbyte);

/** Read next byte in stream without removing it
 *
 * @param[in] self input stream
 * @param[out] buf single byte buffer
 *
 * @return true if successful
 *
 * */
bool BLINK_Stream_peek(blink_stream_t self, void *buf);

/** Init a read only buffer stream
 *
 * @param[in] self
 * @param[in] buf buffer
 * @param[in] max length of buffer
 *
 * @return stream
 * 
 * */
blink_stream_t BLINK_Stream_initBufferReadOnly(struct blink_stream *self, const void *buf, uint32_t max);

/** Init a read/write buffer stream
 *
 * @param[in] self
 * @param[in] buf buffer
 * @param[in] max length of buffer
 *
 * @return stream
 * 
 * */
blink_stream_t BLINK_Stream_initBuffer(struct blink_stream *self, void *buf, uint32_t max);

/** Init a user defined stream
 *
 * @param[in] self
 * @param[in] user user state
 * @param[in] fn user defined functions
 *
 * @return stream
 * 
 * */
blink_stream_t BLINK_Stream_initUser(struct blink_stream *self, void *state, struct blink_stream_user fn);

/** Init a bounded stream
 *
 * @param[in] self
 * @param[in] stream stream to bound
 * @param[in] max stream is allowed to read/seek within range (cur .. cur + max) bytes
 *
 * @return stream
 *
 * */
blink_stream_t BLINK_Stream_initBounded(struct blink_stream *self, blink_stream_t stream, uint32_t max);

/** Get current position
 *
 * @param[in] self
 * @return stream position from origin
 *
 * */
uint32_t BLINK_Stream_tell(blink_stream_t self);

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

/** Check if stream has reached EOF
 *
 * @param[in] self
 *
 * @return true if at EOF
 *
 * */
bool BLINK_Stream_eof(blink_stream_t self);

/** Return the maximum position of the stream
 *
 * @param[in] self
 *
 * @return maximum byte position
 *
 * @retval 0 stream doesn't have a maximum
 *
 * */
uint32_t BLINK_Stream_max(blink_stream_t self);


/** Set the maximum position of the stream
 *
 * @note for bounded streams
 *
 * @param[in] self
 * @param[in] offset byte offset from zero
 *
 * @return true if max could be set
 *
 * */
bool BLINK_Stream_setMax(blink_stream_t self, uint32_t offset);

#ifdef __cplusplus
}
#endif

/** @} */

#endif
