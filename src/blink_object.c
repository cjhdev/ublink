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

#include "blink_object.h"
#include "blink_compact.h"
#include "blink_stream.h"
#include "blink_schema.h"
#include "blink_debug.h"

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

/* types **************************************************************/

struct blink_object_sequence {  
    union blink_object_value value;
    struct blink_object_sequence *next;
};

struct blink_object_field {
    blink_schema_t definition;           /**< field definition */
    union {
        union blink_object_value value;
        struct {
            struct blink_object_sequence *head;
            struct blink_object_sequence *tail;
            uint32_t size;
        } sequence;
    } data;                         /**< field data may be singular or sequence */
    bool initialised;               /**< true if data has been initialised */
};

struct blink_object {
    uint32_t size;                      /** encoded size cache */
    blink_pool_t pool;
    blink_schema_t definition;          /**< group definition */    
    struct blink_object_field *fields;  /**< array of fields */
    size_t numberOfFields;
};

/* static function prototypes *****************************************/

static struct blink_object_field *lookupField(struct blink_object *group, const char *name, size_t nameLen);
static size_t countFields(blink_schema_t group);

/* functions **********************************************************/

blink_object_t BLINK_Object_newGroup(blink_pool_t pool, blink_schema_t group)
{
    BLINK_ASSERT(group != NULL)

    blink_object_t retval = NULL;
    struct blink_object *self = BLINK_Pool_calloc(pool, sizeof(struct blink_object));

    if(self != NULL){

        self->definition = group;
        self->numberOfFields = countFields(group);
        self->fields = BLINK_Pool_calloc(pool, (sizeof(struct blink_object_field) * self->numberOfFields));

        if(self->fields != NULL){

            size_t stackDepth = BLINK_Group_numberOfSuperGroup(group) + 1U;
            blink_schema_t stack[stackDepth];
            struct blink_field_iterator iter = BLINK_FieldIterator_init(stack, stackDepth, group);
            blink_schema_t f = BLINK_FieldIterator_next(&iter);
            size_t i = 0U;

            while(f != NULL){
            
                self->fields[i].definition = f;
                i++;
            }

            retval = (blink_object_t)self;
        }
    }

    return retval;
}

bool BLINK_Object_set(blink_object_t group, const char *fieldName, const union blink_object_value *value)
{
    BLINK_ASSERT(group != NULL)

    bool retval = false;
    struct blink_object_field *field = lookupField(group, fieldName, strlen(fieldName));

    if(field != NULL){
    
        enum blink_type_tag type = BLINK_Field_getType(field->definition);

        switch(type){
        case BLINK_TYPE_STRING:            
        case BLINK_TYPE_BINARY:         

            if(value->string.len <= BLINK_Field_getSize(field->definition)){
        
                field->data.value.string = value->string;
                //todo need to alloc
                retval = true;
            }
            else{

                BLINK_ERROR("string too large for definition")
            }
            break;
        case BLINK_TYPE_FIXED:
            if(value->string.len == BLINK_Field_getSize(field->definition)){
    
                field->data.value.string = value->string;
                //todo need to alloc
                retval = true;
            }
            else{

                BLINK_ERROR("wrong size fixed field")
            }
            break;
        case BLINK_TYPE_BOOL:
            field->data.value.boolean = value->boolean;
            retval = true;
            break;
        case BLINK_TYPE_U8:
        case BLINK_TYPE_U16:
        case BLINK_TYPE_U32:
        case BLINK_TYPE_U64:
        case BLINK_TYPE_TIME_OF_DAY_MILLI:
        case BLINK_TYPE_TIME_OF_DAY_NANO:
            field->data.value.u64 = value->u64;
            retval = true;
            break;        
        case BLINK_TYPE_I8:        
        case BLINK_TYPE_I16:
        case BLINK_TYPE_I32:
        case BLINK_TYPE_DATE:
        case BLINK_TYPE_NANO_TIME:
        case BLINK_TYPE_MILLI_TIME:
        case BLINK_TYPE_I64:
            field->data.value.i64 = value->i64;
            retval = true;
            break;        
        case BLINK_TYPE_ENUM:
        {
            blink_schema_t e = BLINK_Enum_getSymbolByName(field->definition, value->enumeration);

            if(e != NULL){

                field->data.value.i64 = (int64_t)BLINK_Symbol_getValue(e);
                retval = true;
            }                        
        }
            break;            
        case BLINK_TYPE_F64:
            field->data.value.f64 = value->f64;
            retval = true;
            break;                
        case BLINK_TYPE_DECIMAL:
            field->data.value.decimal = value->decimal;
            retval = true;
            break;        
        case BLINK_TYPE_DYNAMIC_GROUP:
            if(BLINK_Group_hasID(value->group->definition)){

                field->data.value.group = value->group;
                retval = true;
            }
            else{

                BLINK_ERROR("expecting a Group that can be encoded dynamically")                
            }
            break;
        case BLINK_TYPE_STATIC_GROUP:
            field->data.value.group = value->group;
            retval = true;
            break;        
        default:
            break;
        }          
    }

    return retval;    
}

union blink_object_value BLINK_Object_get(blink_object_t group, const char *fieldName)
{
    BLINK_ASSERT(group != NULL)

    union blink_object_value retval;
    (void)memset(&retval, 0, sizeof(retval));
    struct blink_object_field *field = lookupField(group, fieldName, strlen(fieldName));

    if(field != NULL){
    
        enum blink_type_tag type = BLINK_Field_getType(field->definition);

        if(field->initialised){

            if(type == BLINK_TYPE_ENUM){

                blink_schema_t s = BLINK_Enum_getSymbolByValue(field->definition, (int32_t)field->data.value.i64);

                if(s != NULL){

                    retval.enumeration = BLINK_Symbol_getName(s);
                }                            
            }
            else{

                retval = field->data.value;
            }
        }
    }

    return retval;    
}

bool BLINK_Object_clear(blink_object_t group, const char *fieldName)
{
    BLINK_ASSERT(group != NULL)

    bool retval = false;
    struct blink_object_field *field = lookupField(group, fieldName, strlen(fieldName));

    if(field != NULL){

        field->initialised = false;
        retval = true;
    }

    return retval;
}

bool BLINK_Object_setEnum(blink_object_t group, const char *fieldName, const char *value)
{
    return BLINK_Object_set(group, fieldName, (union blink_object_value *)value);
}

bool BLINK_Object_setBool(blink_object_t group, const char *fieldName, bool value)
{
    return BLINK_Object_set(group, fieldName, (union blink_object_value *)value);    
}

bool BLINK_Object_setDecimal(blink_object_t group, const char *fieldName, struct blink_decimal *value)
{
    return BLINK_Object_set(group, fieldName, (union blink_object_value *)&value);    
}

bool BLINK_Object_setUint(blink_object_t group, const char *fieldName, uint64_t value)
{
    return BLINK_Object_set(group, fieldName, (union blink_object_value *)&value);    
}

bool BLINK_Object_setInt(blink_object_t group, const char *fieldName, int64_t value)
{
    return BLINK_Object_set(group, fieldName, (union blink_object_value *)&value);    
}

bool BLINK_Object_setF64(blink_object_t group, const char *fieldName, double value)
{
    return BLINK_Object_set(group, fieldName, (union blink_object_value *)&value);    
}

bool BLINK_Object_setString(blink_object_t group, const char *fieldName, struct blink_string *value)
{
    return BLINK_Object_set(group, fieldName, (union blink_object_value *)value);        
}

bool BLINK_Object_setBinary(blink_object_t group, const char *fieldName, struct blink_string *value)
{
    
    return BLINK_Object_set(group, fieldName, (union blink_object_value *)value);        
}

bool BLINK_Object_setFixed(blink_object_t group, const char *fieldName, struct blink_string *value)
{
    return BLINK_Object_set(group, fieldName, (union blink_object_value *)value);        
}

bool BLINK_Object_setGroup(blink_object_t group, const char *fieldName, blink_object_t value)
{
    return BLINK_Object_set(group, fieldName, (union blink_object_value *)value);        
}

bool BLINK_Object_fieldIsNull(blink_object_t group, const char *fieldName)
{
    BLINK_ASSERT(group != NULL)

    bool retval = false;
    struct blink_object_field *field = lookupField(group, fieldName, strlen(fieldName));

    if(field != NULL){

        retval = (false == field->initialised);
    }

    return retval;
}

struct blink_string BLINK_Object_getEnum(blink_object_t group, const char *fieldName)
{
    return BLINK_Object_get(group, fieldName).string;
}

bool BLINK_Object_getBool(blink_object_t group, const char *fieldName)
{
    return BLINK_Object_get(group, fieldName).boolean;
}

struct blink_decimal BLINK_Object_getDecimal(blink_object_t group, const char *fieldName)
{
    return BLINK_Object_get(group, fieldName).decimal;
}

uint64_t BLINK_Object_getUint(blink_object_t group, const char *fieldName)
{
    return BLINK_Object_get(group, fieldName).u64;
}

int64_t BLINK_Object_getInt(blink_object_t group, const char *fieldName)
{
    return BLINK_Object_get(group, fieldName).i64;
}

double BLINK_Object_getF64(blink_object_t group, const char *fieldName)
{
    return BLINK_Object_get(group, fieldName).f64;
}

struct blink_string BLINK_Object_getString(blink_object_t group, const char *fieldName)
{
    return BLINK_Object_get(group, fieldName).string;
}

struct blink_string  BLINK_Object_getBinary(blink_object_t group, const char *fieldName)
{
    return BLINK_Object_get(group, fieldName).string;
}

struct blink_string  BLINK_Object_getFixed(blink_object_t group, const char *fieldName)
{
    return BLINK_Object_get(group, fieldName).string;
}   

blink_object_t BLINK_Object_getGroup(blink_object_t group, const char *fieldName)
{
    return BLINK_Object_get(group, fieldName).group;
}

/* static function ****************************************************/

static struct blink_object_field *lookupField(struct blink_object *group, const char *name, size_t nameLen)
{
    size_t i;
    struct blink_object_field *retval = NULL;

    for(i=0; i < group->numberOfFields; i++){

        const char *fname = BLINK_Field_getName(group->fields[i].definition);
        if(strcmp(fname, name) == 0){

            retval = &group->fields[i];
            break;
        }
    }

    if(i == group->numberOfFields){

        /* field not found */
        BLINK_DEBUG("field name %s does not exist for group %s", name, BLINK_Group_getName(group->definition))
    }

    return retval;
}

static size_t countFields(blink_schema_t group)
{
    size_t retval = 0U;
    size_t stackSize = BLINK_Group_numberOfSuperGroup(group) + 1U;
    blink_schema_t stack[stackSize];
    struct blink_field_iterator iter = BLINK_FieldIterator_init(stack, stackSize, group);

    while(BLINK_FieldIterator_next(&iter) != NULL){

        retval++;
    }

    return retval;
}
