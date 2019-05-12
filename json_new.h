#ifndef JSON_NEW_H
#define JSON_NEW_H
typedef enum {
    JSON_NONE = 0,
    JSON_TRUE,
	JSON_FALSE,
    JSON_STR,
    JSON_ARR,
    JSON_OBJ,
    JSON_NUM,
} json_e;
typedef struct lept_value lept_value;//前向声明
typedef struct lept_member {
	char *s;
	size_t len;
	lept_value v;
}lept_member;
typedef struct {
	json_e type;
	union {
		double num;            
		bool bol;
		//对arr和obj引入了capacity 使其变成动态数组以及动态对象从而可以实现赋值操作。注意解析json和给这两个类型赋值是单独的，即不能给解析出来的数组或者对象进行赋值操作。当然可以提供这种操作但是当前没有实现
		struct {char *str;size_t len;}s;    //这里需要保存str的长度，一个是为了更方便的得出字符串的长度，另外一个重要原因是json字符串中的\U0000这个代表的是'\0'，所以char *str不是以空结尾的，打印的时候也不能用string.h中的API
		struct {lept_value *e, size_t sz;size_t capacity;}arr;
		struct {lept_member *m, size_t sz;size_t capacity;}obj;
	}u;
}lept_value;
//定义错误值，让我们容易定位错误。
typedef enum {
    LEPT_PARSE_OK = 0,
    LEPT_PARSE_EXPECT_VALUE,
    LEPT_PARSE_INVALID_VALUE,
    LEPT_PARSE_ROOT_NOT_SINGULAR,
    LEPT_PARSE_NUMBER_BEYOND_LIMIT,
    LEPT_PARSE_MISS_QUOTATION_MARK,
    LEPT_PARSE_INVALID_STRING_ESCAPE,
    LEPT_PARSE_INVALID_STRING_CHAR,
    LEPT_PARSE_INVALID_UNICODE_HEX,
    LEPT_PARSE_INVALID_UNICODE_SURROGATE,
	LEPT_PARSE_ARRAY_OVERFLOW,
    LEPT_PARSE_MISS_COMMA_OR_SQUARE_BRACKET,
    LEPT_PARSE_MISS_KEY,
    LEPT_PARSE_MISS_COLON,
    LEPT_PARSE_MISS_COMMA_OR_CURLY_BRACKET,
	LEPT_PARSE_STRINGFY_OK,
	LEPT_OBJ_KEY_NOT_EXIST
}resId;
typedef unsigned int BOOL;
typedef unsigned int U32;
#define LEPT_INIT(val) do{(val)->type = JSON_NONE;}while(0)  //val要用()括起来，这样严谨
#define LEPT_PARSE_STACK_INIT_SIZE 256
int          lept_parse(lept_value *, const char *);//这个是对外的接口，在这个函数中会根据const char *类型调用不同的接口
json_e       lept_get_type(lept_value);
double       lept_get_number(const lept_value);
void         lept_set_number(lept_value *, const double n);
const char*  lept_get_string(const lept_value);
size_t       lept_get_string_len(const lept_value);
void         lept_set_val_str(lept_value *, const char *, size_t);
void         lept_free(lept_value *);
size_t       lept_get_array_size(lept_value);
const lept_value *lept_get_array_elem(lept_value, size_t);  //相关get函数参数用指针比较好，因为避免参数赋值了 但是这里就不做修改了
                                                           //因为数组里面包含了元素，并且数组中返回是元素的指针，所以上述的最好都用*比较好

//obj用动态数组数据结构表示了，没有用哈希结构，可以通过数组下标来访问  测试证明 const char *p 指向一个char数组，返回的p + i，不是const类型
size_t       lept_get_obj_size(const lept_value *);
const char * lept_get_obj_key(const lept_value *， size_t);      //从解析Obj开始参数都用lept_value*
size_t  	 lept_get_obj_key_len(const lept_value *, size_t);
const 	   	 lept_value * lept_get_obj_value(const lept_value *, size_t);
int          lept_stringfy(const lept_value *, char **, size_t *);
int   		 lept_find_obj_index(const lept_value *, const char *, size_t);
int 		 lept_find_obj_value(const lept_value *, const char *, size_t);
int 		 lept_is_equal(const lept_value *, const lept_value *);
void         lept_copy(lept_value *, const lept_value *);
void    	 lept_move(lept_value *, lept_value *);
void         lept_set_array(lept_value *, size_t);
void         lept_get_array_capacity(lept_value *, size_t);
void   	     lept_reserve_array(lept_value *, size_t);
void         lept_shrink_array(lept_value *, size_t);
lept_value 	*lept_push_back_arr_elem(lept_value *);
void 		 lept_pop_back_arr_elem(lept_value *);
/*下面三个接口以后在实现把 实在是写烦了*/
lept_value*  lept_insert_array_element(lept_value *, size_t);
void 		 lept_erase_array_element(lept_value *, size_t, size_t);
void 		 lept_clear_array(lept_value *);
size_t 		 lept_get_object_capacity(const lept_value* v);
void 		 lept_reserve_object(lept_value* v, size_t capacity);
void         lept_shrink_object(lept_value* v);
void         lept_clear_object(lept_value* v);
lept_value  *lept_set_object_value(lept_value* v, const char* key, size_t klen);
void         lept_remove_object_value(lept_value* v, size_t index);



