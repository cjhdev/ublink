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
 * Configurable uni-directional IO stream.
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

struct blink_stream;

/** generic stream reference used by dependent modules */
typedef struct blink_stream * blink_stream_t;

/** generic base type */
struct blink_stream {
    enum {
        BLINK_STREAM_NULL = 0,
        BLINK_STREAM_INPUT_BUFFER,
        BLINK_STREAM_OUTPUT_BUFFER,
        BLINK_STREAM_INPUT_USER,
        BLINK_STREAM_OUTPUT_USER,
    } type;
};

/** fixed size input buffer subtype */
struct blink_stream_input_buffer {
    struct blink_stream type;
    const uint8_t *in;
    size_t inLen;
    size_t pos;
};

/** fixed size output buffer subtype */
struct blink_stream_output_buffer {
    struct blink_stream type;
    uint8_t *out;
    size_t outMax;
    size_t pos;
};

/** user configurable input subtype */
struct blink_stream_input_user {
    struct blink_stream type;
    void *state;
    bool (*read)(void *state, uint8_t *out, size_t outMax);    
};

/** user configurable output subtype */
struct blink_stream_output_user {
    struct blink_stream type;
    void *state;
    bool (*write)(void *state, const uint8_t *in, size_t inLen);    
};

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

/** Init a read buffer stream
 *
 * @param[in] self read buffer stream object
 * @param[in] in input buffer to read from
 * @param[in] inLen byte length of input buffer
 *
 * @return generic stream object reference
 * 
 * */
blink_stream_t BLINK_Stream_initInputBuffer(struct blink_stream_input_buffer *self, const uint8_t *in, size_t inLen);

/** Init a write buffer stream
 *
 * @param[in] self write buffer stream object
 * @param[in] out output buffer to write to
 * @param[in] outMax maximum byte length of output buffer
 *
 * @return generic stream object reference
 * 
 * */
blink_stream_t BLINK_Stream_initOutputBuffer(struct blink_stream_output_buffer *self, uint8_t *out, size_t outMax);


#ifdef __cplusplus
}
#endif

/** @} */

#endif
