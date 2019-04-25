#ifndef JSON_NEW_H
#define JSON_NEW_H
typedef enum {
    JSON_NONE,
    JSON_BOL,
    JSON_STR,
    JSON_ARR,
    JSON_OBJ,
    JSON_INT,
} json_e;
typedef struct {
	json_e type;
	union {
		double num;
		bool bol;
		char *str;
		array arr;
		object obj;
	}u;
}lept_value;

//定义错误值，让我们容易定位错误。
enum {
    LEPT_PARSE_OK = 0,
    LEPT_PARSE_EXPECT_VALUE,
    LEPT_PARSE_INVALID_VALUE,
    LEPT_PARSE_ROOT_NOT_SINGULAR,
    LEPT_PARSE_NUMBER_TOO_BIG,
    LEPT_PARSE_MISS_QUOTATION_MARK,
    LEPT_PARSE_INVALID_STRING_ESCAPE,
    LEPT_PARSE_INVALID_STRING_CHAR,
    LEPT_PARSE_INVALID_UNICODE_HEX,
    LEPT_PARSE_INVALID_UNICODE_SURROGATE,
    LEPT_PARSE_MISS_COMMA_OR_SQUARE_BRACKET,
    LEPT_PARSE_MISS_KEY,
    LEPT_PARSE_MISS_COLON,
    LEPT_PARSE_MISS_COMMA_OR_CURLY_BRACKET
};
typedef unsigned int BOOL;
typedef unsigned int U32;
int lept_parse_value(lept_value *, const char *);//这个是对外的接口，在这个函数中会根据const char *类型调用不同的接口
#endif
