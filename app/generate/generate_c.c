/* includes ***********************************************************/

#include "generate.h"
#include "generate_c.h"

#include <stdio.h>
#include <assert.h>
#include <inttypes.h>
#include <string.h>

#include <stdlib.h>

/* definitions ********************************************************/

#define LINE(OUT,...) do{fprintf(OUT,__VA_ARGS__);fprintf(OUT,"\n");}while(0)
#define EMPTY_LINE(OUT) do{fprintf(OUT, "\n");}while(0)

/* static function prototypes *****************************************/

static void putHeader(FILE *out, blink_schema_t schema);
static void putGroupStruct(FILE *out, blink_schema_t group);
static void putFieldStruct(FILE *out, blink_schema_t group, blink_schema_t field);
static void putGroupInit(FILE *out, blink_schema_t group);
static void putFieldAccessors(FILE *out, blink_schema_t group);
static void putGroupEncode(FILE *out, blink_schema_t group);
static void putGroupDecode(FILE *out, blink_schema_t group);
static void putSource(FILE *out, const char *header, blink_schema_t schema);
const char *typeAsCTypeString(enum blink_type_tag type);
static void putAccessorMacros(FILE *out);

/* functions **********************************************************/

bool GenerateC(struct arguments *arg)
{
    bool retval = false;

    const char *headerFileName = "groups.h";
    

    #if 0
    const char *sourceFileName = "groups.c";    
    FILE *header = fopen(headerFileName, "w");
    FILE *source = fopen(sourceFileName, "w");
    #else
    FILE *header = stdout;
    FILE *source = stdout;
    #endif

    if((header != NULL) && (source != NULL)){
        
        putHeader(header, arg->schema);
        putSource(source, headerFileName, arg->schema);

        retval = true;
    }

    fclose(header);
    fclose(source);

    return retval;
}

/* static functions ***************************************************/

static void putHeader(FILE *out, blink_schema_t schema)
{
    LINE(out, "#ifndef GEN_H");
    LINE(out, "#define GEN_H");
    EMPTY_LINE(out);
    LINE(out, "#include <stdint.h>");
    LINE(out, "#include <stdbool.h>");
    LINE(out, "#include <stddef.h>");
    EMPTY_LINE(out);

    putAccessorMacros(out);
    
    struct blink_group_iterator iter = BLINK_GroupIterator_init(schema);

    blink_schema_t group = BLINK_GroupIterator_next(&iter);

    do {

        if(group != NULL){

            putGroupStruct(out, group);
            EMPTY_LINE(out);
            group = BLINK_GroupIterator_next(&iter);
        }
    }
    while(group != NULL);

    LINE(out, "#endif GEN_H");    
}

static void putSource(FILE *out, const char *headerFileName, blink_schema_t schema)
{
    LINE(out, "#include \"%s\"", headerFileName);
    EMPTY_LINE(out);
    
    struct blink_group_iterator iter = BLINK_GroupIterator_init(schema);

    blink_schema_t group = BLINK_GroupIterator_next(&iter);

    do {

        if(group != NULL){
            
            putGroupEncode(out, group);
            putGroupDecode(out, group);
            putFieldAccessors(out, group);
            putGroupInit(out, group);
            EMPTY_LINE(out);

            group = BLINK_GroupIterator_next(&iter);
        }
    }
    while(group != NULL);
}

static void putGroupStruct(FILE *out, blink_schema_t group)
{
    const char *groupName = BLINK_Group_getName(group);
    bool hasID = BLINK_Group_getID(group);

    size_t stackSize = BLINK_Group_numberOfSuperGroup(group) + 1U;
    blink_schema_t stack[stackSize];
    struct blink_field_iterator iter;

    blink_schema_t field;

    LINE(out, "struct %s {", groupName);
    LINE(out, "    uint32_t size;");
    LINE(out, "    blink_pool_t pool;");
    LINE(out, "    bool hasID;");
    LINE(out, "    const char *name;");

    if(hasID){
    
        uint64_t id = BLINK_Group_getID(group);
        LINE(out, "    uint64_t id; /**< id := %" PRIu64 " */", id);
    }

    iter = BLINK_FieldIterator_init(stack, stackSize, group);

    do {

        field = BLINK_FieldIterator_next(&iter);

        if(field != NULL){

            putFieldStruct(out, group, field);
        }
    }
    while(BLINK_FieldIterator_peek(&iter));

    LINE(out, "};");
}

static void putFieldStruct(FILE *out, blink_schema_t group, blink_schema_t field)
{
    const char *fieldName = BLINK_Field_getName(field);
    bool isSequence = BLINK_Field_isSequence(field);
    //bool isOptional = BLINK_Field_isOptional(field);
    enum blink_type_tag type = BLINK_Field_getType(field);
    const char *ctype = typeAsCTypeString(type);
    const char *groupName = BLINK_Group_getName(group);

    LINE(out, "    struct {");
    LINE(out, "        bool initialised;");

    switch(type){
    case BLINK_TYPE_STRING:
    case BLINK_TYPE_BINARY:
    case BLINK_TYPE_FIXED:
        LINE(out, "        uint8_t value[%" PRIu32 "U];", BLINK_Field_getSize(field));
        break;    
    case BLINK_TYPE_ENUM:
        break;
    default:
        LINE(out, "        %s value;", ctype);
        break;
    }
    
    LINE(out, "    } %s;", fieldName);

    if(isSequence){

        LINE(out, "    bool (*append_%s)(struct %s *group, %s value);", fieldName, groupName, ctype);
        LINE(out, "    bool (*get_%s)(struct %s *group, uint32_t index, %s *value);", fieldName, groupName, ctype);
        LINE(out, "    bool (*%s_isPresent)(struct %s *group);", fieldName, groupName);
    }
    else{

        LINE(out, "    bool (*set_%s)(struct %s *group, %s value);", fieldName, groupName, ctype);
        LINE(out, "    bool (*get_%s)(struct %s *group, %s *value);", fieldName, groupName, ctype);
        LINE(out, "    bool (*%s_isPresent)(struct %s *group);", fieldName, groupName);            
    }
}

const char *typeAsCTypeString(enum blink_type_tag type)
{
    const char *retval = NULL;
    
    switch(type){
    case BLINK_TYPE_STRING:
    case BLINK_TYPE_BINARY:
    case BLINK_TYPE_FIXED:
        retval = "struct blink_string";
        break;    
    case BLINK_TYPE_BOOL:
        retval = "bool";
        break;
    case BLINK_TYPE_U8:
        retval = "uint8_t";
        break;
    case BLINK_TYPE_U16:
        retval = "uint8_t";
        break;
    case BLINK_TYPE_U32:
        retval = "uint8_t";
        break;
    case BLINK_TYPE_U64:
        retval = "uint8_t";
        break;
    case BLINK_TYPE_I8:
        retval = "uint8_t";
        break;
    case BLINK_TYPE_I16:
        retval = "uint8_t";
        break;
    case BLINK_TYPE_I32:
        retval = "uint8_t";
        break;
    case BLINK_TYPE_I64:
        retval = "uint8_t";
        break;
    case BLINK_TYPE_F64:
        retval = "double";
        break;
    case BLINK_TYPE_DATE:
    case BLINK_TYPE_TIME_OF_DAY_MILLI:
    case BLINK_TYPE_TIME_OF_DAY_NANO:
    case BLINK_TYPE_NANO_TIME:
    case BLINK_TYPE_MILLI_TIME:
    case BLINK_TYPE_DECIMAL:
    case BLINK_TYPE_OBJECT:
    case BLINK_TYPE_ENUM:
    case BLINK_TYPE_STATIC_GROUP:
    case BLINK_TYPE_DYNAMIC_GROUP:
    default:
        break;
    }

    return retval;
}

#if 0

    enum blink_type_tag type = BLINK_Field_getType(field);

    LINE("static bool set_%s(struct %s_%s *group, %s value)", fieldName, prefix, groupName, typeName);
    LINE("{");
    
    LINE("");

    const char *setter;
    
    switch(type){
    case BLINK_TYPE_BOOL:
    case BLINK_TYPE_I8:
    case BLINK_TYPE_I16:
    case BLINK_TYPE_I32:
    case BLINK_TYPE_DATE:
    case BLINK_TYPE_I64:
    case BLINK_TYPE_U8:
    case BLINK_TYPE_U16:
    case BLINK_TYPE_U32:
    case BLINK_TYPE_U64:
        setter"    group->%s.value = value;", fieldName);
        break;
    case BLINK_TYPE_BINARY:
    case BLINK_TYPE_STRING:
        
        "group->value.data = BLINK_Pool_calloc(group->pool, value.len);"
        "if(group->value.data != NULL){\n"
        "   (void)memcpy(group->value.data, value.data, value.len);\n"
        "   field->value.len = value.len;"
        "retval = NULL;"
        "}"
        break;
    case BLINK_TYPE_
            
    
    }

    LINE("}");
}
#endif

static void putGroupInit(FILE *out, blink_schema_t group)
{
    const char *groupName = BLINK_Group_getName(group);
    const char *namespaceName = BLINK_Namespace_getName(BLINK_Group_getNamespace(group));

    LINE(out, "void Init_%s(struct %s *group)", groupName, groupName);
    LINE(out, "{");
    LINE(out, "    (void)memset(group, 0, sizeof(*group);");

    size_t nameLen = strlen(groupName) + ((strlen(namespaceName) == 0U) ? 0 : strlen(namespaceName) + 1U) + 1U;
    char name[nameLen];

    name[nameLen-1U] = '\0';

    if(strlen(namespaceName) > 0U){

        strcpy(name, namespaceName);
        name[strlen(namespaceName)] = ':';
        strcpy(&name[strlen(namespaceName)+1U], groupName);
    }
    else{

        strcpy(name, groupName);
    }
    

    LINE(out, "    group->name = \"%s\";", name);

    LINE(out, "    group->hasID = %s;", BLINK_Group_hasID(group) ? "true" : "false");

    if(BLINK_Group_hasID(group)){

        LINE(out, "    group->id = %" PRIu64 "U;", BLINK_Group_getID(group));
    }

    size_t stackSize = BLINK_Group_numberOfSuperGroup(group) + 1U;
    blink_schema_t stack[stackSize];

    struct blink_field_iterator iter = BLINK_FieldIterator_init(stack, stackSize, group);

    do{

        blink_schema_t field = BLINK_FieldIterator_next(&iter);

        if(field != NULL){

            const char *fieldName = BLINK_Field_getName(field);

            if(BLINK_Field_isSequence(field)){

            }
            else{

                LINE(out, "    group->get_%s = %s_get_%s;", fieldName, groupName, fieldName);
                LINE(out, "    group->set_%s = %s_get_%s;", fieldName, groupName, fieldName);
            }                
        }
    }
    while(BLINK_FieldIterator_peek(&iter));
        
    LINE(out, "}");
}

static void putFieldAccessors(FILE *out, blink_schema_t group)
{
    const char *groupName = BLINK_Group_getName(group);

    size_t stackSize = BLINK_Group_numberOfSuperGroup(group) + 1U;
    blink_schema_t stack[stackSize];
    struct blink_field_iterator iter = BLINK_FieldIterator_init(stack, stackSize, group);

    do{

        blink_schema_t field = BLINK_FieldIterator_next(&iter);

        if(field != NULL){

            const char *fieldName = BLINK_Field_getName(field);

            if(BLINK_Field_isSequence(field)){

            }
            else{

                LINE(out, "static %s %s_get_%s(struct %s *self)", typeAsCTypeString(BLINK_Field_getType(field)), groupName, fieldName, groupName);
                LINE(out, "{");
                LINE(out, "    return self->%s.value;", fieldName);
                LINE(out, "}");
                EMPTY_LINE(out);
                LINE(out, "static bool %s_set_%s(struct %s *self, %s value)", groupName, fieldName, groupName, typeAsCTypeString(BLINK_Field_getType(field)));
                LINE(out, "{");
                LINE(out, "    return true;");
                LINE(out, "}");
                EMPTY_LINE(out);
                LINE(out, "static bool %s_%s_isPresent(struct %s *self)", groupName, fieldName, groupName);
                LINE(out, "{");
                LINE(out, "    return self->%s.initialised;", fieldName);
                LINE(out, "}");
                EMPTY_LINE(out);
            }                
        }
    }
    while(BLINK_FieldIterator_peek(&iter));
}

static void putGroupEncode(FILE *out, blink_schema_t group)
{
}

static void putGroupDecode(FILE *out, blink_schema_t group)
{
}

static void putAccessorMacros(FILE *out)
{   
    LINE(out, "#define SET(GROUP,FIELD,VALUE) GROUP->set_FIELD(GROUP, VALUE)");
    EMPTY_LINE(out);
    LINE(out, "#define GET(GROUP,FIELD,VALUE) GROUP->get_FIELD(GROUP)");
    EMPTY_LINE(out);
    LINE(out, "#define IS_PRESENT(GROUP,FIELD) GROUP->FIELD_isPresent(GROUP)");
    EMPTY_LINE(out);
}


