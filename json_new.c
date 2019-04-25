#include"json_new.h"
#include<stdio.h>
typedef struct {
	const char *json;
	char *stack;
	size_t size, top;
}lept_context;


#defined EXPECT(val, ch) do{ assert(val!=NULL && val->json!=NULL && *val->json==ch);val->json++;}while(0)
static int lept_parse_null(lept_value *, lept_context *);//想将一个函数固定到某一个源文件中一是不要写到头文件中二是声明为static
static int lept_parse_true(lept_value *, lept_context *);
static int lept_parse_false(lept_value *, lept_context *);
static void extract_val(lept_context *);
json_e lept_get_type(lept_value);

json_e lept_get_type(lept_value val) {
	return val.type;
}

static void extract_val(lept_context *text) {   
	assert(text!=NULL);
	const char *p = text->json;
	char c;
	for(c=*p; c == ' ' || c == '\n' || c == '\r' || c=='\t'; c=*(++p));
	text->json = p;
	return;
}

static int lept_parse_null(lept_value *val, lept_context *text) {
	EXPECT(text, 'n');//先要检测第一个字符，lept_parse_value中是根据第一个字符选择解析函数的，所以宏里面用assert检测下。
	const char *p = text->json;
	if(p[0] == 'u' && p[1] == 'l' && p[2] == 'l') {  //字符串结尾是\0，所以这里没有数组越界的情形。
		text->json += 3;
		val->type = JSON_NONE;   
		return LEPT_PARSE_OK;
	}
	return LEPT_PARSE_INVALID_VALUE;
}

static int lept_parse_false(lept_value *val, lept_context *text) {
	EXPECT(text, 'f');
	const char *p = text->json;
	if(p[0] == 'a' && p[1] == 'l' && p[2] == 's' && p[3] == 'e') {
		text->json += 4;
		val->type = JSON_FALSE;   
		return LEPT_PARSE_OK;
	}
	return LEPT_PARSE_INVALID_VALUE;
}

static int lept_parse_true(lept_value *val, lept_context *text) {
	EXPECT(text, 't');
	const char *p = text->json;
	if(p[0] == 'r' && p[1] == 'u' && p[2] == 'e') {
		text->json += 3;
		val->type = JSON_TRUE;   
		return LEPT_PARSE_OK;
	}
	return LEPT_PARSE_INVALID_VALUE;
}	

static int lept_parse_value(lept_val *val, lept_context *text) {
	char c = *(text->json);
	switch(c) {
		case 'n'  : return lept_parse_null(val, text);
		case 'f'  : return lept_parse_true(val, text);
		case 't'  : return lept_parse_false(val, text);
		case '\0' : return LEPT_PARSE_EXPECT_VALUE;
		default   : return LEPT_PARSE_INVALID_VALUE;
	}
}

int lept_parse(lept_value *val, const char *json_val) { 
	resId ret;
	assert(val!=NULL);
	lept_context text;
	text.json = json_val;
	extract_val(&text);
	ret = lept_parse_value(val, &text);
	if(ret == LEPT_PARSE_OK) {
		extract_val(&text);
		if(*(text.json) != '\0') ret = LEPT_PARSE_ROOT_NOT_SINGULAR;
	}
	return ret;
}