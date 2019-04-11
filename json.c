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
//无论json_new还是json_delete都是对于obj对象来说的
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

void json_delete(JSON *obj)
{
	int num = obj->obj.count;
	keyval *kvs = obj->obj.kvs;
	for(int i = 0; i < num; i++)
	{
		free(kvs[i].key);
		value_delete(kvs[i].val);
		kvs[i] = NULL;
	}
	free(obj);
	obj = NULL;
}
void free_arr(value *arr)
{
	int count = arr->count;
	for(int i = 0; i < count; i++)
	{
		value_delete(arr[i]);
	}
	free(arr->elems);
}
void value_delete(value *json)
{
	if(!json)
		return;
	json_e type = json->type;
	switch(type) 
	{
		case JSON_INT :
		case JSON_BOL :
		{
			break;
		}
		case JSON_STR :
		{
			if(json->str)
				free(json->str);
			break;
		}
		case JSON_OBJ :
		{
			json_delete(json);
			break;
		}
		case JSON_ARR :
		{
			free_arr(json);
			break;
		}
	}
	free(json);
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
//对值类型经行编码，得到的结果字符串类型以','结尾。
void encode_value(value *val, char *pre_val, int *pre_val_index) 
{
	assert(val);
	json_e type = val->type;
	char *res = malloc(sizeof(char) * MAX_NUM);
	int index = 0;
	switch(type)
	{
		//值类型编码出来的中间结果都以'\0'结尾，为了能和之前的进行拼接。
		case JSON_STR : 
		{
			res[index++] = '\"';
			strcpy(res + index, val->str)；
			index = sizeof(*res);
			res[index - 1] = '\"';
			res[index++] = '\0';
			break;
		}	
		case JSON_INT :
		{
			sprintf(res, "%d", val->num);
			break;
		}
		case JSON_BOL :
		{
			if(val->bol == true)
				strcpy(res, "true");
			else if(val->bol == false)
				strcpy(res, "false");
			break;
		}
		case JSON_ARR :
		{
			json_encode_arr(val, res);
			break;
		}
		//obj类型编码最终也是得到'\0'结尾
		case JSON_OBJ :
		{
			json_encode_obj(val, def, res);
			break;
		}
	}
	if( res ) {
		//拼接后，在将结尾替换为','号
		strcat(pre_val + *pre_val_index, res);
		*pre_val_index = sizeof(*pre_val);
		pre_val[*pre_val_index - 1] = ',';
		free(res);
	}
}

void json_encode_arr(value *val, char *res)
{
	assert(val);
	if(val->type != JSON_ARR)
		return NULL;
	int count = val->count;
	if(!count || !val->arr.elems) return NULL;
	int index = 0;
	res[index++] = '[';
	for(int i = 0; i < count; i++)
	{
		value *temp = val->arr.elems[i];
		assert(temp);
		encode_value(temp, res, &index);
	}
	res[index - 1] = ']';
	res[index++] = '\0';
}
//将obj encode为字符串类型。最外层肯定是"{}\0"，里面每一项的关键字为 "key":
//调用函数得到value的字符串类型并且以,号结束。
void json_encode_obj(const JSON *json, const char *def, char *arr_char)
{
	U32 i;
    assert(json);
    if (json->type != JSON_OBJ)
        return def;
	assert(json->obj.count == 0 || json->obj.kvs);
	int index = 0;
	arr_char[index++] = '{';
	for (i = 0; i < json->obj.count; ++i)
	{
		/*
		if(index >= limit)
		{
			limit = limit * 2;
			char *realloc = malloc(limit);
			if(!realloc)
				return def;
			strcpy(realloc, arr_char);
			free(arr_char);
			arr_char = realloc;
		}
		*/
		assert(json->obj.kvs[i].key);
		arr_char[index++] = '\"';
		strcat(arr_char+index, json->obj.kvs[i].key);
		index = sizeof(*arr_char);
		arr_char[index - 1] = '\"';
		arr_char[index++] = ':';
		assert(json->obj.kvs[i].val);
		encode_value(json->obj.kvs[i].val, arr_char, &index);
	}
	arr_char[index - 1] = '}';
	arr_char[index++] = '\0';
	return arr_char;
}

//值类型为str 
const char *json_get_str(const JSON *json, const char *key, const char *def)
{
	const JSON * val = json_get_member(json, key);
	if(val == NULL)
		return def;
	char *res = malloc(sizeof(char) * MAX_NUM);
	int index = 0;
	encode_value(val, res, &index);
	res[index - 1] = ',';
	res[index++] = '\0';
	return res;
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

BOOL json_set_int(JSON *json, const char *key, int val) 
{
	
}



