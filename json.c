#include "json.h"

typedef struct array array;
typedef struct object object;
typedef struct value value;
typedef struct keyvalue keyvalue;
//json_encode得到的是字符串类型，json_decode得到的是对象类型。这里实现的和decode相关。
//平常说json格式指的是json值为对象格式，json值类型可以是int bool array，
//object类型，数据结构定义如下面所示。object是key-value类型的数组。而array类型，是值的数组，比如
//[{"firstname":"shi","secondname":"xiaoxiao"},{"firstname":"qin","secondname":"xiaonan"}] [123,456]。
/**
 *  想想：这些结构体定义在.c是为什么？
 */


//需要一个数据结构保存key 和 value的。key为char*。
struct keyvalue {
    char *key;
    value *val;
};

/**
 *  想想：如果要提升内存分配效率，这个结构体该作什么变化？
 */
struct array {
    value **elems;      /* 想想: 这里如果定义为'value *elems'会怎样？ */
    U32 count;
};

/**
 *  想想：如果要提升内存分配效率，这个结构体该作什么变化？
 */
struct object {
    keyvalue *kvs;
    U32 count;
};
//值的类型是union里面的类型并且只能是一种。
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
JSON *json_new(json_e type)
{
    JSON *json = (JSON *)calloc(1, sizeof(JSON));
    if (!json)
        return NULL;
    json->type = type;
	switch( type ) {
		case JSON_BOL : json->bol = false;  break;
		case JSON_INT : json->num = 0;      break;
		case JSON_STR : json->str = NULL;   break;
		case JSON_OBJ : json->obj->kvs = NULL; break;
		case JSON_ARR : json->elems =NULL; break;
	}
    return json;
}

//从json对象中根据关键字提取出来值
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

//
//对于值类型为int型的。
int json_get_int(const JSON *json, const char *key, int def) 
{
	const JSON * val = json_get_member(json, key);
	if(val == NULL || val->type != JSON_INT)
		return def;
	return val->num;
}

//值类型为bool
BOOL json_get_bool(const JSON *json, const char *key);
{
	const JSON * val = json_get_member(json, key);
	if(val == NULL || val->type != JSON_BOL)
		return false;
	return val->bol;
}

//值类型为str 
const char *json_get_str(const JSON *json, const char *key, const char *def)
{
	const JSON * val = json_get_member(json, key);
	if(val == NULL || val->type != JSON_STR)
		return def;
	if(val->type == JSON_STR)
		return val->str;
	else {
		//要定义将数组以及对象转化为字符串的函数
	}
}

//对于值类型为数组的，还需要定义函数提取值
const JSON *json_get_index(const JSON *json, U32 idx)
{
    assert(json);
    if (json->type != JSON_ARR)
        return NULL;
    if (idx >= json->arr.count)
        return NULL;
    return json->arr.elems[idx];
}




