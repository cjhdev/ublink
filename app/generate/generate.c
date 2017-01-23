#include "generate.h"
#include "generate_c.h"

#include <stdlib.h>
#include <stdio.h>

#include <malloc.h>
#include <assert.h>

static bool newSchema(const char *fileName, blink_schema_t *schema);

int main(int argc, const char **argv)
{
    bool retval = false;

    struct arguments arg;

    if(newSchema("dev.blink", &arg.schema)){

        retval = GenerateC(&arg);
    }

    exit( (retval) ? EXIT_SUCCESS : EXIT_FAILURE);
}


static bool newSchema(const char *fileName, blink_schema_t *schema)
{
    assert(fileName != NULL);
    assert(schema != NULL);

    bool retval = false;

    FILE *f = fopen(fileName, "r");

    if(f != NULL){
        
        (void)fseek(f, 0, SEEK_END);
        long int size = ftell(f);

        char *buffer = calloc(1, size);

        size_t poolBufferSize = 10*1024;
        uint8_t *poolBuffer = calloc(1, poolBufferSize);

        if((buffer != NULL) && (poolBuffer != NULL)){

            struct blink_pool pool;

            (void)fseek(f, 0, SEEK_SET);
            (void)fread(buffer, size, 1, f);
            (void)BLINK_Pool_init(&pool, poolBuffer, poolBufferSize);

            struct blink_stream stream;
            (void)BLINK_Stream_initBufferReadOnly(&stream, (uint8_t *)buffer, (size_t)size);

            *schema = BLINK_Schema_new(&pool, &stream);
            retval = true;     
        }
        else{

            fprintf(stderr, "calloc()");

            free(buffer);
            free(poolBuffer);
        }

        fclose(f);
    }
    else{

        fprintf(stderr, "cannot open %s", fileName);
    }

    return retval;
}
