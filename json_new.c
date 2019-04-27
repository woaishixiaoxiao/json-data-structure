#include"json_new.h"
#include<stdio.h>
#include<stdlib.h>
typedef struct {
	const char *json;
	union {
		struct { char *stack; size_t size, top;}s;
		double n;
	}v;
}lept_context;


#defined EXPECT(val, ch) do{ assert(val!=NULL && val->json!=NULL && *val->json==ch);val->json++;}while(0)
static int   lept_parse_null(lept_value *, lept_context *);//想将一个函数固定到某一个源文件中一是不要写到头文件中二是声明为static
static int   lept_parse_true(lept_value *, lept_context *);
static int   lept_parse_false(lept_value *, lept_context *);
static void  extract_val(lept_context *);
static void  lept_parse_number(lept_value *, lept_context *);
static int   lept_parse_literal(lept_value *, lept_context *);
json_e       lept_get_type(lept_value);

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

static int lept_parse_literal(lept_value *val, lept_context *text, const char *str, size_t sz) {
	size_t i;
	const char *p;
	p = text->json;
	for(i = 0; i < sz; i++) {
		if(*p != str[i]){
			return LEPT_PARSE_INVALID_VALUE;
		}else {
			++p;
		}
	}
	if(*p != '\0' && *p != '\r' && *p != '\n' && *p != '\t' && *p != ' ') {
		return LEPT_PARSE_INVALID_VALUE;
	}
	text->json = p;
	return LEPT_PARSE_OK;
}
static int lept_parse_null(lept_value *val, lept_context *text) {
	assert(val!=NULL && text!=NULL);
	int ret = lept_parse_literal(val, text, "null", 4);
	if(ret == LEPT_PARSE_OK) {
		val->type = JSON_NONE;
	}
	return ret;
}

static int lept_parse_false(lept_value *val, lept_context *text) {
	assert(val!=NULL && text!=NULL);
	int ret = lept_parse_literal(val, text, "false", 5);
	if(ret == LEPT_PARSE_OK) {
		val->type = JSON_FALSE;
	}
	return ret;
}

static int lept_parse_true(lept_value *val, lept_context *text) {
	assert(val!=NULL && text!=NULL);
	int ret = lept_parse_literal(val, text, "true", 4);
	if(ret == LEPT_PARSE_OK) {
		val->type = JSON_TRUE;
	}
	return ret;
}	

static int lept_parse_value(lept_val *val, lept_context *text) {
	assert(val!=NULL && text!=NULL);
	char c = *(text->json);
	switch(c) {
		case 'n'  : return lept_parse_null(val, text);
		case 'f'  : return lept_parse_true(val, text);
		case 't'  : return lept_parse_false(val, text);
		case '-'  :
		case '0'  ：
		case '1'  :
		case '2'  :
		case '3'  :
		case '4'  :
		case '5'  :
		case '6'  :
		case '7'  :
		case '8'  :
		case '9'  : return lept_parse_number(val, text);
		case '\0' : return LEPT_PARSE_EXPECT_VALUE;
		default   : return LEPT_PARSE_INVALID_VALUE;
	}
}

double lept_get_number(const lept_value val) {
	assert(val.type == JSON_NUMBER);
	return val.u.num;
}
/*
number = [ "-" ] int [ frac ] [ exp ]
int = "0" / digit1-9 *digit
frac = "." 1*digit
exp = ("e" / "E") ["-" / "+"] 1*digit
*/
static int lept_parse_number(lept_value *val, lept_context *text) {
	assert(val!=NULL && text!=NULL);
	double res;
	char *endpoint,ch;
	res = strtod(text->json, &endpoint);
	if(errno == ERANGE) {
		return LEPT_PARSE_NUMBER_BEYOND_LIMIT;
	}
	ch = *endpoint;
	if(ch != ' ' || ch != '\r' || ch != '\n' || ch != '\t') {
		return LEPT_PARSE_INVALID_VALUE;
	}
	val->u.num = res;
	val->type = JSON_NUMBER;
	//不要忘了将json指针进行修改
	text->json = endpoint;
	return LEPT_PARSE_OK;
}

int lept_parse(lept_value *val, const char *json_val) { 
	resId ret;
	assert(val!=NULL);
	lept_context text;
	text.json = json_val;
	extract_val(&text);
	val->type = JSON_NONE;//json值解析失败，我们也置为空，这里提前处理，后面解析函数中，失败了就不用考虑赋值为json_none了
	ret = lept_parse_value(val, &text);
	if(ret == LEPT_PARSE_OK) {
		extract_val(&text);
		if(*(text.json) != '\0') ret = LEPT_PARSE_ROOT_NOT_SINGULAR;
	}
	return ret;
}