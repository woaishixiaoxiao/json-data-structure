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
 
//提高内存分配效率可以一个变量capacity，表示容纳的能力>=count。当count == capacity的时候才
//需要重新分配内存下面object一样
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
		case JSON_OBJ : json->obj->kvs = NULL; json->obj->count = 0; break;
		case JSON_ARR : json->elems =NULL; json->obj->count = 0; break;
	}
    return json;
}
//json_delete都是对于obj对象来说的
void json_delete(JSON *obj)
{
	int num = obj->obj.count;
	keyval *kvs = obj->obj.kvs;
	for(int i = 0; i < num; i++)
	{
		free(kvs[i].key);
		kvs[i].key = NULL;
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
	if (json)
		free(json);
}
//从json对象中根据关键字提取出来值
const JSON *json_get_member(const JSON *json, const char *key, bool &is_exist)
{
    U32 i;
    assert(json);
    if (json->type != JSON_OBJ)
	{
		is_exist = false;
        return NULL;
	}
    assert(json->obj.count == 0);
    for (i = 0; i < json->obj.count; ++i) {
        if (strcmp(json->obj.kvs[i].key, key) == 0)
		{
			is_exist = true;
            return json->obj.kvs[i].val;
		}
    }
	is_exist = false;
    return NULL;
}

//
//对于值类型为int型的。
int json_get_int(const JSON *json, const char *key, int def) 
{
	bool is_exist;
	const JSON * val = json_get_member(json, key, &is_exist);
	if(val == NULL || val->type != JSON_INT)
		return def;
	return val->num;
}

//值类型为bool
BOOL json_get_bool(const JSON *json, const char *key);
{
	bool is_exist;
	const JSON * val = json_get_member(json, key, &is_exist);
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
			index = strlen(res) + 1;
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
		*pre_val_index = strlen(pre_val) + 1;
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
		index = strlen(arr_char) + 1;
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
	bool is_exist;
	const JSON * val = json_get_member(json, key, &is_exist);
	if(val == NULL)
		return def;
	char *res = NULL;
	res = malloc(sizeof(char) * MAX_NUM);
	if(!res)
		return def;
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

void arr_cpy(value *arr1, value *arr2)
{
	assert(arr1 || arr2);
	int sz = arr2->arr.count;
	arr1->count = sz;
	arr1->elems = malloc(sz * sizeof(value *));
	for(int i = 0; i < sz; i++)
	{
		arr1->elems[i] = NULL;
		depth_cpy(&(arr1->elems[i]), arr2->elems[i]);
	}
}

void obj_cpy(value *obj1, value *obj2, int extra)
{
	assert(obj1 || obj2);
	int sz = obj2->obj.count;
	int key_sz;
	obj1->obj.kvs = malloc(sz * sizeof(keyvalue) + extra);
	obj1->obj.count = sz;
	keyvalue *kvs1 = obj1->obj.kvs;
	keyvalue *kvs2 = obj2->obj.kvs;
	for(int i = 0; i < sz; i++)
	{
		key_sz = strlen(kvs2[i].key) + 1;
		kvs1[i].key = malloc(key_sz * sizeof(char));
		strcpy(kvs1[i].key, kvs2[i].key);
		kvs1[i].val = NULL;
		depth_cpy(&(kvs1[i].val), kvs2[i].val);
	}
}
void depth_cpy(value **src, value *dst)
{
	if (!dst)return;
	if (*src) {
		value_delete(*src);
		*src = NULL;
	}
	json_e type = dst->type;
	switch(type)
	{
		case JSON_INT :
		{
			*src = json_new(type);
			if(!(*src))
				return;
			(*src)->type = JSON_INT;
			(*src)->num = dst->num;
			break;
		}
		case JSON_BOL :
		{
			*src = json_new(type);
			if(!(*src))
				return;
			(*src)->type = JSON_BOL;
			(*src)->bol = dst->bol;
			break;
		}
		case JSON_ARR :
		{
			*src = json_new(type);
			if (!(*src))
				return;
			(*src)->type = JSON_ARR;
			arr_cpy(*src, dst);
			break;
		}
		case JSON_OBJ :
		{
			*src = json_new(type);
			if (!(*src))
				return;
			(*src)->type = JSON_OBJ;
			obj_cpy(*src, dst, 0);
			break;
		}
		case JSON_STR :
		{
			*src = json_new(type);
			if(!(*src))
				return;
			(*src)->type = JSON_STR;
			int sz = strlen(dst->str);
			char *str = malloc(sizeof(char) * sz);
			strcpy(str, dst->str);
			(*src)->str = str;
			break;
		}
	}
	
}
bool expand(JSON *json, int extra)
{
	JSON *new = json_new(JSON_OBJ);
	new->obj.kvs = json->obj.kvs;
	json->obj.kvs= NULL;
	obj_cpy(json, new, 1);
	json_delete(new);
}
void json_add_member(JSON *json, const char *key, JSON *val)
{
	expand(json, 1);
	int sz = strlen(key) + 1;
	json->obj.kvs[count].key = malloc(sizeof(char) * sz);
	strcpy(json->obj.kvs[count].key, key);
	json->obj.kvs[count].value = val;
	json->obj.count = json->obj.count + 1;
}
bool json_set_value(JSON *json, const char *key, void *val, json_e type)
{
	bool is_exist = false;
	const JSON * json_val_old = json_get_member(json, key, &is_exist);
	if (json_val_old)
		value_delete(json_val_old);
	value * json_val_new = json_new(type);
	if (!json_val_new)
		return false;
	switch(type)
	{
		case JSON_INT :
		{
			int num = *(int *)val;
			json_val_new->type = JSON_INT;
			json_val_new->num = num;
			break;
		}
		case JSON_BOL :
		{
			bool bol = *(bool *)val;
			json_val_new->type = JSON_BOL;
			json_val_new->bol = bol;
			break;
		}
		case JSON_STR :
		{
			char *str = char(*)val;
			json_val_new->type = JSON_STR;
			json_val_new->str = str;
			break;
		}
		case JSON_ARR :
		{
			array arr = *(array *)val;
			json_val_new->type = JSON_ARR;
			depth_cpy(&json_val_new, json_val_old);
			break;
		}
		case JSON_OBJ :
		{
			object obj = *(object *)val;
			json_val_new->type = JSON_OBJ;
			depth_cpy(&json_val_new, json_val_old);
			break;
		}
	}
	if (is_exist)
	{
		json_val_old = json_val_new;
		return ture;
	}else {
		json_add_member(json, key, json_val_new);
	}
}
booL json_set_int(JSON *json, const char *key, int val) 
{
	bool is_success = false;
	is_success = json_set_value(json, key, (void*)&val, JSON_INT);
	return is_success; 
}
bool json_set_obj(JSON *json, const char *key, value *obj)
{
	bool is_success = false;
	is_success = json_set_value(json, key, (void*)obj, JSON_OBJ);
	return is_success;
}

int main()
{
	json_e type = JSON_OBJ;
	json * json_val = json_new(type);
	if( json_set_int(json_val, "shi", 123) )
	{
		value_delete(json_val);
		return 1;
	}		
}

