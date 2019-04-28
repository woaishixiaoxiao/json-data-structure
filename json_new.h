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
typedef struct {
	json_e type;
	union {
		double num;            
		bool bol;
		struct {char *str;size_t len}s;
		array arr;
		object obj;
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
    LEPT_PARSE_MISS_COMMA_OR_SQUARE_BRACKET,
    LEPT_PARSE_MISS_KEY,
    LEPT_PARSE_MISS_COLON,
    LEPT_PARSE_MISS_COMMA_OR_CURLY_BRACKET,
}resId;
typedef unsigned int BOOL;
typedef unsigned int U32;
#define LEPT_INIT(val) do{(val)->type = JSON_NONE;}while(0)  //val要用()括起来，这样严谨
#define LEPT_PARSE_STACK_INIT_SIZE 256
int lept_parse(lept_value *, const char *);//这个是对外的接口，在这个函数中会根据const char *类型调用不同的接口
json_e lept_get_type(lept_value);
double lept_get_number(const lept_value);
const char * lept_get_string(const lept_value);
size_t lept_get_string_len(const lept_value);
int lept_set_val_str(lept_value *, const char *, size_t);
void lept_free(lept_value *);
#endif
