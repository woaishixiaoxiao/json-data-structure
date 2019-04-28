#include"json_new.h"
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
typedef struct {
	const char *json;
	char *stack;
	size_t sz, top;
}lept_context;


#defined EXPECT(text, ch) do{ assert((text)->json==(ch));(text)->json++;}while(0)
#defined PUTC(text, ch) context_push(text, ch ,1)
	
static int   lept_parse_null(lept_value *, lept_context *);//想将一个函数固定到某一个源文件中一是不要写到头文件中二是声明为static
static int   lept_parse_true(lept_value *, lept_context *);
static int   lept_parse_false(lept_value *, lept_context *);
static void  extract_val(lept_context *);
static void  lept_parse_number(lept_value *, lept_context *);
static int   lept_parse_literal(lept_value *, lept_context *);
static int   lept_parse_string(lept_value *, lept_context *);
static void  *context_push(lept_context *text, size_t len);
static void  *context_pop(lept_context *text, size_t len);

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
		case '\"' : return lept_parse_string(val, text);
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
int lept_parse_number(lept_value *val, lept_context *text) {
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
//编写stack_push和stack_pop两个函数的时候，不用考虑lept_value_string怎么实现，编写通用型
void *context_push(lept_context *text, const char *str, size_t len) {  //返回栈顶指针
	assert(text!=NULL && len > 0);
	int i;
	if( (text->top + len) > text->sz ) {  
		if(text->top == 0) {    //这里要有这个判断并且给text->top赋值的原因是 text->sz += (text->sz >> 1)
			text->sz = LEPT_PARSE_STACK_INIT_SIZE; //如果sz为0，那么while会死循环
		}
		while( (text->top + len) > text->sz ) {
			text->sz += (text->sz >> 1);  //比原来扩大1.5倍
		}
		text->stack = (char *)realloc( (void*)text->stack, text->sz );  //注意这里第一个参数可以为NULL，所以初始化很重要
	}
	strncpy(text->stack+text->top, str, len);
	text->top += len;
	return (void *)(text->stack + text->top);
}

void *context_pop(lept_context *text, size_t len) {  
	assert(text != NULL && len > 0 && text->top >= len);
	return text->stack += (text->top -= len);
}

int lept_parse_string(lept_value *val, lept_context *text) {
	assert(val!=NULL && text!=NULL);
	size_t head = text->top, len;
	EXPECT(text, '\"');
	const char * p = text->json;
	char c;
	for(;;) {
		c = *p++;
		switch(c) {
			case '\"': {
				len = text->top - head;
				lept_set_val_str(val, json, context_pop(text, len), len);
				text->json = p;
				return LEPT_PARSE_OK;
			}
			default  : {
				PUTC(text, c);
			}
		}
	}
}

int lept_parse(lept_value *val, const char *json_val) { 
	resId ret;
	assert(val!=NULL);
	lept_context text;
	text.json = json_val;
	text.stack = NULL;   //这里可以对栈进行初始化 但是不要分配空间
	text.top = text.sz = 0;
	extract_val(&text);
	//json值解析失败，我们也置为空，这里提前处理，后面解析函数中，失败了就不用考虑赋值为json_none了
	LEPT_INIT(val);             
	ret = lept_parse_value(val, &text);
	if(ret == LEPT_PARSE_OK) {
		extract_val(&text);
		if(*(text.json) != '\0') ret = LEPT_PARSE_ROOT_NOT_SINGULAR;
	}
	assert(text.top == 0);//解析完成之后，top应该为0了
	free(text.stack);
	return ret;
}


void lept_free(lept_value *val) {
	assert(val!=NULL);
	if(val->type == JSON_STR) {   //因为这里要判断val的类型，如果创建一个lept_value变量的时候，应该先将变量初始化为NONE。
		if(val->u.s.str) {   		//这个宏要写在头文件中。
			u.s.len = 0;
			free(val->u.s.str);
		}
	}
	val->type == JSON_NONE;
}

void lept_set_val_str(lept_value *val, const char *json, size_t len) {
	assert(val!=NULL && json!=NULL);
	lept_free(val);       
	val->u.s.str = (char*)malloc(sizeof(char)*len + 1);
	strncpy(val->u.s.str, json, len);
	val->u.s.str[len] = '\0'; 
	val->u.s.len = len;
	val->type = JSON_STR;
}
//返回字符串，这里直接将val中的指针返回了。虽然返回的是const char *但是是可以free的。
const char * lept_get_string(const lept_value val) {
	assert(val.type != JSON_STR);  //这里不用返回错误码，直接assert就好
	return val.u.s.str;
}
size_t lept_get_string_len(const lept_value val) {
	assert(val.type != JSON_STR);
	return val.u.s.len;
}

