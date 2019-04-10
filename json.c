#include "json.h"

typedef struct array array;
typedef struct object object;
typedef struct value value;
typedef struct keyvalue keyvalue;

/**
 *  想想：这些结构体定义在.c是为什么？
 */

/**
 *  想想：如果要提升内存分配效率，这个结构体该作什么变化？
 */
struct array {
    value **elems;      /* 想想: 这里如果定义为'value *elems'会怎样？ */
    U32 count;
};

struct keyvalue {
    char *key;
    value *val;
};

/**
 *  想想：如果要提升内存分配效率，这个结构体该作什么变化？
 */
struct object {
    keyvalue *kvs;
    U32 count;
};

struct value {
    json_e type;
    union {
        int num;
        BOOL bol;
        char *str;
        array arr;
        object obj;
    };
};


/**
 *
 */
const JSON *json_get_member(const JSON *json, const char *key)
{
    U32 i;
    assert(json);
    if (json->type != JSON_OBJ)
        return NULL;
    assert(json->obj.count == 0 || json->obj.kvs);
    for (i = 0; i < json->obj.count; ++i) {
        if (strcmp(json->obj.kvs[i].key, key) == 0)
            return json->obj.kvs[i].val;
    }
    return NULL;
}

const JSON *json_get_index(const JSON *json, U32 idx)
{
    assert(json);
    if (json->type != JSON_ARR)
        return NULL;
    if (idx >= json->arr.count)
        return NULL;
    return json->arr.elems[idx];
}

JSON *json_new(json_e type)
{
    JSON *json = (JSON *)calloc(1, sizeof(JSON));
    if (!json)
        return NULL;
    json->type = type;
    return json;
}