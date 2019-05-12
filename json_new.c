#include"json_new.h"
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.c>
typedef struct {
	const char *json;
	char *stack;
	size_t sz, top;
}lept_context;

#define LEPT_PARSE_STACK_INIT_SIZE 256
#define EXPECT(text, ch)    do{ assert((text)->json==(ch));(text)->json++;}while(0)
#define PUTC(text, ch)      do{*(char *)context_push(text, sizeof(char)) = ch;}while(0)
#define PUTS(text, str, len)     do{strncpy(context_push(text, len), str, len);}while(0)
#define STRING_ERROR(error) do{text->top = head;return error;}while(0)
#define DOUBLE_EQ(left, right) (fabs(left) - fabs(right) > (-1e-8) || fabs(left) - fabs(right) < (1e-8))
static int   lept_parse_null(lept_value *, lept_context *);//想将一个函数固定到某一个源文件中一是不要写到头文件中二是声明为static
static int   lept_parse_true(lept_value *, lept_context *);
static int   lept_parse_false(lept_value *, lept_context *);
static void  lept_parse_whitespace(lept_context *);
static void  lept_parse_number(lept_value *, lept_context *);
static int   lept_parse_literal(lept_value *, lept_context *);
static int   lept_parse_string(lept_value *, lept_context *);
static void *context_push(lept_context *text, size_t len);
static void *context_pop(lept_context *text, size_t len);
static char *lept_parse_hex4(char *, unsigned *);
static char *lept_encode_UTF8(lept_context *, unsigned);
static int   lept_parse_string_raw(lept_context *, char *, size_t *);
static int   lept_parse_array(lept_value *, lept_context *);
static int   lept_parse_obj(lept_value *, lept_context *);
static int   lept_parse_obj_key(lept_member *, lept_context *);
static int   lept_stringfy_value(const lept_value *, lept_context *);
static int   lept_stringfy_simple(lept_context *, const char *, size_t);
static int   lept_stringfy_string(lept_value *, lept_context *);
static int   lept_stringfy_arr(const lept_value *, lept_context *);
static int   lept_is_equal_arr(const lept_value *, const lept_value *)
static int   lept_is_equal_obj(const lept_value *, const lept)

json_e lept_get_type(lept_value val) {
	return val.type;
}

static void lept_parse_whitespace(lept_context *text) {   
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
		case '['  : return lept_parse_array(val, text);
		case '{'  : return lept_parse_obj(val, text);
		case '\0' : return LEPT_PARSE_EXPECT_VALUE;
		default   : return LEPT_PARSE_INVALID_VALUE;
	}
}

double lept_get_number(const lept_value val) {
	assert(val.type == JSON_NUM);
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
	val->type = JSON_NUM;
	//不要忘了将json指针进行修改
	text->json = endpoint;
	return LEPT_PARSE_OK;
}

void lept_set_number(lept_value *val, const double n) {
	assert(val != NULL);
	val->type = JSON_ARR;
	val->u.num = n;
}

//编写stack_push和stack_pop两个函数的时候，不用考虑lept_value_string怎么实现，编写通用型
//这样写不太好，要是压入字符，还要传入参字符的指针，可以省略第二个参数，在函数中将栈顶更新，
//但是返回的是旧的栈顶，这样的好处是在context_push之后，可以调用lept_parse_value对刚才分配的空间
//进行初始化。在写两个辅助的宏函数，对栈赋值 
//static void *context_push(lept_context *text, const char *str, size_t len) { 
static void *context_push(lept_context *text, size_t len) {
	assert(text!=NULL && len > 0);
	void *ret;
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
	ret = text->stack + text->top;
	text->top += len;
	return ret;
}

static void *context_pop(lept_context *text, size_t len) {  
	assert(text != NULL && len > 0 && text->top >= len);
	return text->stack += (text->top -= len);
}

static char *lept_parse_hex4(char *p, unsigned *u) {
	assert(p != NULL && u != NULL);
	*u = 0;
	size_t sz = 4, i;
	for(i = 0; i < sz; i++) {
		/*
		本来想先检测合法性在用sscanf(p, "%x",&n) 这样写太垃圾了,
		if( p[i] != '\0' && ( p[i] <= '9' && p[i] >= '0') || (p[i] <= 'F' && p[i] >= 'A') || (p[i] <= 'f' && p[i] >= 'a') ) {
			p++
		}else {
			return NULL;
		}
		*/
		char c = *p++; //之前所有的都是用的*p++ 这里保持原样，一是减少错误，二是代码写的好看
		*u <<= 4；		
		if(c >= '0' && c <= '9') *u |= (c - '0'); 
		else if(c >= 'a' && c <= 'f') *u |= (c - 'a' + 10);
		else if(c >= 'A' && c <= 'F') *u |= (c - 'A' + 10);
		else return NULL;
	}
	return p;
}
static char *lept_encode_UTF8(lept_context *text, unsigned u) {
	assert(text != NULL);
	char c;
	/*这里没必要判断两个，只要判断右边就行了
	if(u >= 0 && u <= 0x7F) {
		c = u
		PUTC(text, )
	}else if(u >= 0x80 && u <= 0x7FF) {
		c = (char)((u >> 6) | 0xc0);
		PUTC(text, c);
		c = (char)()
	}else if(u >= 0x800 && u <= 0xFFFF){
		c = (char);
		PUTC(text, (u >> 12) & 0xFF) | 0xE0);
		c = (char)((u >> 6 ) & 0x3F) | 0x80);
		PUTC(text, c);
		c = (char)((u >> 0 ) & 0x3F) | 0x80);
		PUTC(text, c);
	}else if(u >= 0x10000 && u <= 0x10FFFF) {
	}else {
	}*/
	if(u <= 0x7F) {
		PUTC(text, ((u >> 0) & 0xFF);
	}else if(u <= 0x7FF) {
		PUTC(text, ((u >>  6) & 0xFF) | 0xC0);
		PUTC(text, ((u >>  0) & 0x3F) | 0xE0);
	}else if(u <= 0xFFFF) {
		PUTC(text, ((u >> 12) & 0xFF) | 0xE0);
		PUTC(text, ((u >>  6) & 0x3F) | 0x80);
		PUTC(text, ((u >>  0) & 0x3F) | 0x80);
	}else {
		assert(u <= 0x10FFFF);  //这里用assert因为是内部解析的
		PUTC(text, ((u >> 18) & 0xFF) | 0xF0);
		PUTC(text, ((u >> 12) & 0x3F) | 0x80);
		PUTC(text, ((u >>  6) & 0x3F) | 0x80);
		PUTC(text, ((u >>  0) & 0x3F) | 0x80);
	}
}

/*
string = quotation-mark *char quotation-mark
char = unescaped /
   escape (
       %x22 /          ; "    quotation mark  U+0022
       %x5C /          ; \    reverse solidus U+005C
       %x2F /          ; /    solidus         U+002F
       %x62 /          ; b    backspace       U+0008
       %x66 /          ; f    form feed       U+000C
       %x6E /          ; n    line feed       U+000A
       %x72 /          ; r    carriage return U+000D
       %x74 /          ; t    tab             U+0009
       %x75 4HEXDIG )  ; uXXXX                U+XXXX
escape = %x5C          ; \
quotation-mark = %x22  ; "
unescaped = %x20-21 / %x23-5B / %x5D-10FFFF
以"作为开头以及结尾，中间包含转义字符以及非转义字符
转移字符包含上面九种。非转义字符包含的码点范围是0x20-21 0x23-5B 0x5D-10FFFF
码点指的是Unicode编码。不包含0X22 以及0X5C  这两个分别是" \
注意json中 \"和" 以及 \和\\的区别。因为json是文本格式，存在文件或者网络字节流中。
我们将json文本格式放到char *中保存，char *中的所有字符均是普通字符，没有任何转义含义的。
通常保存json文本的编码格式为UTF-8，我们将json字符串也解析为UTF-8编码方式，最后得到的解析结果是
UTF-8编码的字节流。我们得到下述转换规则
1、对于转义字符
除了\UXXXX 均转化了ASCII编码方式。
对于\UXXXX或者\UXXXX\UXXXX，我们将其先转化为Unicode码点，然后在根据UTF-8编码格式将码点转化为
字节流。
2、对于非转义字符
我们直接将非转义字符的字节流保存到解析结果中去。因为json文本是UTF-8编码的，可能会存在几个字节流表示一个字符的情况，
我们直接将这些字节流按顺序放到解析中去就好了

注意json解析不涉及到encode decode，都是在encode编码范围内做转变，需要decodde那是其他的操作了
*/
/*static int lept_parse_string(lept_value *val, lept_context *text) {
	assert(val!=NULL && text!=NULL);
	size_t head = text->top, len;
	EXPECT(text, '\"');
	const char * p = text->json;
	char c;
	for(;;) {
		c = *p++;
		switch(c) {
			case '"'   : { //注意这里写不写\"都行，加\是因为表示不了，比如在char *s ="\"sf","必须要用\转义才可以
				len = text->top - head;
				lept_set_val_str(val, json, context_pop(text, len), len);    //这里进行改写，函数专注于自己的功能，也就是低耦合。将赋值单独拿出来写成一个函数。
				text->json = p;                          
				return LEPT_PARSE_OK;
			}
			case '\\'  : { //这里就必须用\将'引起来 到了这里，意味这出现转义字符了。
				c=*p++;
				switch(c) {
					case '"'  : PUTC(text, '\"');
					case 't'  : PUTC(text, '\t');
					case 'b'  : PUTC(text, '\b');
					case 'f'  : PUTC(text, '\f');
					case 'n'  : PUTC(text, '\n');
					case 'r'  : PUTC(text, '\r');
					case '\\' : PUTC(text, '\\');
					case '/'  : PUTC(text, '/');
					case 'u'  : {
						/*\UXXXX,指的是unicode的码点，XXXX表示0xXXXX。unicode收集好多国家的字符统一码点，
						码点范围为0~0x10FFFF。另外还规定了存储码点的方式，并产生了UTF-8 UTF-16编码方式
						\UXXXX，最多能表示0~0XFFFF。大于0XFFFF的码点用两个\UXXXX\UXXXX来表示，他们之间有
						映射函数。
						*/
						/*
						unsigned int high = 0;
						unsigned int low = 0;
						if((p = lept_parse_hex4(p, &high)) == NULL) {
							STRING_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX);
						}
						if(high >= 0xD800 && high <= 0xD8FF) {
							if(*p == '\\' && *(p+1) == 'u') {
								p += 2;
								if((p = lept_parse_hex4(p, &low)) == NULL) {
									STRING_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX);
								}
								if(low < 0xDC00 || low > 0xDFFF) {
									STRING_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE);
								}
								//u = 0x10000 + (high − 0xD800) × 0x400 + (low − 0xDC00)这样直接写不太好
								//high减去之后就剩下低8位了  low也一样 将乘法变为转移后在进行按位与操作也可以
								high = (((high - 0xD800) << 10) | (low - 0xDC00)) + 0x10000;
							}
						}
						lept_encode_UTF8(text, high);
						break;
					}
				}
			}
			case '\0'  : { //出现这个说明是有问题的。
				STRING_ERROR(LEPT_PARSE_MISS_QUOTATION_MARK);
			}
			default    : { 
				//unescaped = %x20-21 / %x23-5B / %x5D-10FFFF，这个范围之外的都是不合法的。因为我们上面处理了0x22以及0x5c
				//这里只要比0x20小就行了。char是unsigned char还是signed char看编译器的实现，如果是signed char保存不了>127
				//的整数,但是强制转换成unsigned char肯定是>127的，所以无论编译器如果解析char，下面的判断表达式都可以找出
				//无效的非转义字符
				if((unsigned char)c < 0x20) {
					/*只要json字符串不合法，都会有执行下面两个语句，text->top=head是固定的，但是错误码不一定
					将这两个语句组成宏,这个宏并没有传递text以及head的参数，因为是固定的，只传递了错误码。所以这个宏只限于此函数中使用
					text->top = head;
					return LEPT_PARSE_INVALID_STRING_CHAR;
					*/
					/*
					STRING_ERROR(LEPT_PARSE_INVALID_STRING_CHAR);
				}
				PUTC(text, c);
			}
		}
	}
}
*/

//对上述函数进行改写  改写的原因是 我们后面解析obj的时候，key也是json字符串类型，但是我们不用lept_value保存Key
//而是用char *s保存key，所以将上述函数拆分成两个函数，一个用来保存解析json字符串，返回str以及len，另一个函数赋值，
//这样解析obj key的时候，可以复用这个逻辑
static int lept_parse_string_raw(lept_context *text, char *str, size_t *len) {
	assert(text!=NULL && str!=NULL && len!=NULL);
	size_t head = text->top, len;
	EXPECT(text, '\"');
	const char * p = text->json;
	char c;
	for(;;) {
		c = *p++;
		switch(c) {
			case '"'   : { //注意这里写不写\"都行，加\是因为表示不了，比如在char *s ="\"sf","必须要用\转义才可以
				*len = text->top - head;
				str = context_pop(text, *len);
				text->json = p;
				return LEPT_PARSE_OK;
			}
			case '\\'  : { //这里就必须用\将'引起来 到了这里，意味这出现转义字符了。
				c=*p++;
				switch(c) {
					case '"'  : PUTC(text, '\"');
					case 't'  : PUTC(text, '\t');
					case 'b'  : PUTC(text, '\b');
					case 'f'  : PUTC(text, '\f');
					case 'n'  : PUTC(text, '\n');
					case 'r'  : PUTC(text, '\r');
					case '\\' : PUTC(text, '\\');
					case '/'  : PUTC(text, '/');
					case 'u'  : {
						/*\UXXXX,指的是unicode的码点，XXXX表示0xXXXX。unicode收集好多国家的字符统一码点，
						码点范围为0~0x10FFFF。另外还规定了存储码点的方式，并产生了UTF-8 UTF-16编码方式
						\UXXXX，最多能表示0~0XFFFF。大于0XFFFF的码点用两个\UXXXX\UXXXX来表示，他们之间有
						映射函数。
						*/
						unsigned int high = 0;
						unsigned int low = 0;
						if((p = lept_parse_hex4(p, &high)) == NULL) {
							STRING_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX);
						}
						if(high >= 0xD800 && high <= 0xD8FF) {
							if(*p == '\\' && *(p+1) == 'u') {
								p += 2;
								if((p = lept_parse_hex4(p, &low)) == NULL) {
									STRING_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX);
								}
								if(low < 0xDC00 || low > 0xDFFF) {
									STRING_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE);
								}
								//u = 0x10000 + (high − 0xD800) × 0x400 + (low − 0xDC00)这样直接写不太好
								//high减去之后就剩下低8位了  low也一样 将乘法变为转移后在进行按位与操作也可以
								high = (((high - 0xD800) << 10) | (low - 0xDC00)) + 0x10000;
							}
						}
						lept_encode_UTF8(text, high);
					}
					default   : return LEPT_PARSE_INVALID_STRING_ESCAPE;
				}
				break;
			}
			case '\0'  : { //出现这个说明是有问题的。
				STRING_ERROR(LEPT_PARSE_MISS_QUOTATION_MARK);
			}
			default    : { 
				//unescaped = %x20-21 / %x23-5B / %x5D-10FFFF，这个范围之外的都是不合法的。
				//按照上面的定义出现比0x20小的肯定是错误的。而比0x7F大的是合法的。
				//上述规则意味着，0x20以下的字符全部需要用\U来表示。而大于0x20的即可以用\U表示，也可以用编码表示。
				//JSON文本一般用UTF8保存，所以上述的编码为UTF8编码表示。
				//因为我们上面处理了0x22以及0x5c，下面不合法的只需要判断时是否小于0x20。
				//char是unsigned char还是signed char看编译器的实现。但是我们都是将char强制转化成unsignd char进行比较。
				if((unsigned char)c < 0x20) {
					/*只要json字符串不合法，都会有执行下面两个语句，text->top=head是固定的，但是错误码不一定
					将这两个语句组成宏,这个宏并没有传递text以及head的参数，因为是固定的，只传递了错误码。所以这个宏只限于此函数中使用
					text->top = head;
					return LEPT_PARSE_INVALID_STRING_CHAR;
					*/
					STRING_ERROR(LEPT_PARSE_INVALID_STRING_CHAR);
				}
				PUTC(text, c);
			}
		}
	}
}

static int lept_parse_string(lept_value *val, lept_context *text) {
	char *str;
	size_t len;
	if( (ret = lept_parse_string_raw(text, str, &len)) == LEPT_PARSE_Ok ) {
		lept_set_val_str(val, str, len);
	}
	return ret;
}

int lept_parse(lept_value *val, const char *json_val) { 
	resId ret;
	assert(val!=NULL);
	lept_context text;
	text.json = json_val;
	text.stack = NULL;   //这里可以对栈进行初始化 但是不要分配空间
	text.top = text.sz = 0;
	lept_parse_whitespace(&text);
	//json值解析失败，我们也置为空，这里提前处理，后面解析函数中，失败了就不用考虑赋值为json_none了
	LEPT_INIT(val);             
	ret = lept_parse_value(val, &text);
	if(ret == LEPT_PARSE_OK) {
		lept_parse_whitespace(&text);
		if(*(text.json) != '\0') ret = LEPT_PARSE_ROOT_NOT_SINGULAR;
	}
	assert(text.top == 0);//解析完成之后，top应该为0了
	free(text.stack);   //这里也要释放
	return ret;
}

void lept_free(lept_value *val) {
	size_t i = 0;
	assert(val!=NULL);
	if(val->type == JSON_STR) {   //因为这里要判断val的类型，如果创建一个lept_value变量的时候，应该先将变量初始化为NONE。
		if(val->u.s.str) {   		//这个宏要写在头文件中。
			u.s.len = 0;
			free(val->u.s.str);
		}
	}else if(val->type === JOSN_ARR) {
		for(i = 0; i < val->u.arr.sz; i++) {
			lept_free(val->u.arr.e + i);
		}
		free(val->u.arr.e);  //注意这个不能忘了
	}else if(val->type == JSON_OBJ) {
		for(i = 0; i < val->u.obj.sz; i++) {
			free(val->u.obj.m[i].s);
			lept_free(&(val->u.obj.m[i].v));
		}
		free(val->u.obj.m);
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

size_t lept_get_array_size(lept_value val) {
	assert(val.type != JSON_ARR);
	return val.u.arr.sz;
}

const lept_value * lept_get_array_elem(lept_value val, size_t index) {
	assert(val.type != JSON_ARR && index >=0 && index < val.u.arr.sz);
	return val.u.arr.e + index;
}
/*
这样写有问题 可以看出来 [ ]这种方式的解析不了  所以先lept_parse_whitespace 然后判断下下一个字符是不是 ']'
此时 初始化的时候先解析了空白，在循环中一开始就不要解析空白了，直接进行元素解析 并且每次解析完一个元素的时候需要继续解析空白，
碰到 ,的时候还要在进行一次空白的解析。碰到 ]就结束了  其他的则报错。
static int lept_parse_array(lept_value *val, lept_context *text) {
	assert(val != NULL && text != NULL);
	EXPECT(text, '[');
	lept_value temp;
	size_t sz = 0;
	int ret;
	for(;;) {
		LEPT_INIT(temp);
		lept_parse_whitespace(text);
		if((ret=lept_parse_value(&temp, text)) != LEPT_PARSE) {
			break;
		}
		lept_parse_whitespace(text);
		memcpy(context_push(text, sizeof(lept_value)), (void *)&temp, sizeof(lept_value));
		sz++;
		//lept_free(&temp);这里不能释放这个变量，要是释放掉了，那字符串会出问题。在lept_free中添加释放数组的逻辑
		if(*text->json == ',') {
			text->json++;
		}else if(*text->json == ']') {
			text->json++;
			val->type = JSON_ARR;
			val->u.arr.sz = sz;
			val->u.arr.e = (lept_value *)malloc(sizeof(lept_value) * sz);
			total_ = text->top - head;
			memcpy((void *)val->u.arr.e, context_pop(text, ), total_len);
			return LEPT_PARSE_OK;
		}else {
			
		}
	}
	
}
*/

//在解析数组的时候，text中的栈不光存储已经解析过的lept_value元素 还可能会存储解析json的值，但是没有关系
//因为每次解析string的时候，均会将内容弹出来。并不影响存储lept_value元素。后面解析object也是同样道理
static int lept_parse_array(lept_value *val, lept_context *text) {
	assert(val != NULL && text != NULL);
	EXPECT(text, '[');
	lept_parse_whitespace(text);
	if(*text->json == ']') {
		text->json++;
		val->type = JSON_ARR;
		val->u.arr.sz = 0;
		val->u.arr.e = NULL;
		return LEPT_PARSE_OK;
	}
	size_t sz = 0, i;
	int ret;
	lept_value *fail_val;
	//还有一种做法是，每次解析一个元素的时候，先分配空间，也就是进行context_push操作，然后在以该
	//变量调用lept_parse_value函数。这种做法看似不错其实有bug。因为在lept_parse_value函数中可能会解析字符串
	//或者数组以及object,解析这些值类型都会用到栈的，并且给先前分配的空间赋值是在解析完这些元素之后的，所以会出现
	//解析数组字符串object的时候会造成栈重新分配空间从而造成指向预分配的指针称为悬挂指针。
	for(;;) {
		lept_value temp;
		LEPT_INIT(temp);  //这里不能复用同一个临时变量，因为这个是结构体，结构体的大小和最小的元素有关系，并且是共用同一块内存的
		if((ret=lept_parse_value(&temp, text)) != LEPT_PARSE_OK) {
			break;
		}
		lept_parse_whitespace(text);
		memcpy(context_push(text, sizeof(lept_value)), (void *)&temp, sizeof(lept_value));
		sz++;
		//lept_free(&temp);这里不能释放这个变量，要是释放掉了，那字符串会出问题。在lept_free中添加释放数组的逻辑
		if(*text->json == ',') {
			text->json++;
			lept_parse_whitespace(text);
		}else if(*text->json == ']') {
			text->json++;
			val->type = JSON_ARR;
			val->u.arr.sz = sz;
			val->u.arr.e = (lept_value *)malloc(sizeof(lept_value) * sz);
			memcpy((void *)val->u.arr.e, context_pop(text, sz * sizeof(lept_value)), sz * sizeof(lept_value));
			return LEPT_PARSE_OK;
		}else {
			ret = LEPT_PARSE_MISS_COMMA_OR_SQUARE_BRACKET;
			break;
		}
	}
	for(i = 0; i < sz ; i++) {
		fail_val = (lept_val *)context_pop(text, sizeof(lept_value));
		lept_free(fail_val);
	}	
	return ret;
}

size_t  lept_get_obj_size(const lept_value *v) {
	assert(v != NULL);
	return v->u.obj.sz;
}

const char * lept_get_obj_key(const lept_value *v， size_t index) {
	assert(v != NULL && index < v->u.obj.sz);
	return v->u.obj.m[index].s;
}

size_t  lept_get_obj_key_len(const lept_value *v, size_t index) {
	assert(v != NULL && index < v->u.obj.sz);
	return v->u.obj.m[index].len;
}

const lept_value * lept_get_obj_value(const lept_value *v, size_t index) {
	assert(v != NULL && index < v->u.obj.sz);
	return &(v->u.obj.m[index].v);
}

static int lept_parse_obj_key(lept_member *m, lept_context *text) {
	assert(m != NULL && text != NULL);
	char *str;
	size_t len;
	int ret;
	if( (ret = lept_parse_string_raw(text, str, &len)) == LEPT_PARSE_Ok ) {
		m->k.len = len;
		m->k.s = (char *)malloc(len * sizeof(char) + 1);
		memcpy((void *)m->k.s, (void *)str, len);
		m->k.s[len] = '\0';
	}
	return ret;
}

int lept_find_obj_index(const lept_value *v, const char *key, size_t len) {
	assert(v != NULL && v->type == JSON_OBJ && key != NULL);
	size_t sz = v->u.obj.sz;
	size_t i;
	lept_member *m = v->u.obj.m;
	for(i = 0; i < sz; i++) {
		if(len == m[i].len && strncmp(m[i].s, key, len) == 0) {
			return i;
		}
	}
	return LEPT_OBJ_KEY_NOT_EXIST;
}
const lept_value *lept_find_obj_value(const lept_value *v, const char *key, size_t len) {
	int index = lept_find_obj_index(v, key, ken);
	index != LEPT_OBJ_KEY_NOT_EXIST? return &v->u.obj.m[index].v : NULL;
}

static int lept_parse_obj(lept_value *val, lept_context *text) {
	assert(val != NULL && text != NULL);
	int ret;
	size_t sz = 0, i;
	EXPECT(text, '{');
	lept_parse_whitespace(text);
	if(*text->json == '}') {
		text->json++;
		val->type = JSON_OBJ;
		val->u.obj.sz = 0;
		val->u.obj.m = NULL;
		return LEPT_PARSE_OK;
	}
	for(;;) {
		lept_member kv;
		LEPT_INIT(&kv.v);
		if((ret=lept_parse_obj_key(&kv, text)) != LEPT_PARSE_OK) {
			break;
		}
		lept_parse_whitespace(text);
		if(*text->json != ':') {
			ret = LEPT_PARSE_MISS_COLON;
			break;
		}
		text->json++;
		lept_parse_whitespace(text);
		if((ret=lept_parse_value(&(kv.v), text)) != LEPT_PARSE_OK) {
			break;
		}
		sz++;
		memcpy(context_push(text, sizeof(lept_member)), (void *)&kv, sizeof(lept_member));
		lept_parse_whitespace(text);
		if(*text->json == ',') {
			text->json++;
			lept_parse_whitespace(text);
		}else if(*text->json == '}') {
			text->json++;
			val->type = JSON_OBJ;
			val->u.obj.sz = sz;
			val->u.obj.m = (lept_member *)malloc(sz * sizeof(lept_member));
			memcpy((void *)val->u.obj.m, context_pop(text, sz * sizeof(lept_member)), sz * sizeof(lept_member));
			return LEPT_PARSE_OK;
		}else {
			ret = LEPT_PARSE_MISS_COMMA_OR_CURLY_BRACKET;
			break;
		}
	}
	for(i = 0; i < sz; i++) {
		lept_member *p = (lept_member *)context_pop(text, sizeof(lept_member));
		free(p->s);
		lept_free(&p->v);
	}
	return ret;
}

static int lept_stringfy_simple(lept_context *c, const char *s, size_t len) {
	assert(s!=NULL && c!=NULL);
	PUTS(c, s, len);
	return LEPT_PARSE_STRINGFY_OK;
}

//上面字符串解析的时候分析过了，json字符串范围是unescaped = %x20-21 / %x23-5B / %x5D-10FFFF
//解析后的lept_value中保存了UTF-8的编码。包含转义字符以及非转义字符。转义字符包含下面的几种。
//对于小于0x20的编码（json中是\U），我们需要给他转换成"\UXXXX"的类型，而其他的则不需要。
/*static int lept_stringfy_string(lept_value *v, lept_context *c) {
	assert(v != NULL && c != NULL);
	int i;
	unsigned char ch;
	const char *p = v->u.s;
	PUTC(c, '"');
	for(i = 0; i < v->u.s.len; i++) {
		ch = (unsigned char)p[i];
		switch(ch) {
			case '\t' : PUTS(c, "\\t", 2); break;
			case '\b' : PUTS(c, "\\b", 2); break;
			case '\n' : PUTS(c, "\\n", 2); break;
			case '\f' : PUTS(c, "\\f", 2); break;
			case '\r' : PUTS(c, "\\r", 2); break;
			case '\\' : PUTS(c, "\\\\", 2);break;
			case '/'  : PUTS(c, "\\/", 2); break;
			case '"'  : PUTS(C, "\\\"", 2);break;
			default   : {
				if(ch < 0x20) {
					snprintf(context_push(c, 6), 6, "\u%04x", (unsigned char)ch);
				}else {
					PUTC(c, ch);
				}
			}
		}
	}
	PUTC(c, '"');
	return LEPT_PARSE_STRINGFY_OK;
}*/

//对上面的函数进行重构。上面的会大量调用PUTS函数，而这个函数是比较耗时间的，另外snprintf这个函数处理也比较费时间
//可以自己写一个进制转化为字符串的函数 就是时空转换的问题。
//这里参数不要写成后面的两个因为到时候解析obj的时候要解析key。static int lept_stringfy_string(lept_value *v, lept_context *c)
static int lept_stringfy_string(lept_context *c, const char *s, size_t len) {
	assert(c != NULL && s != NULL);
	static char *hex[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E'};
	size_t i, sz;
	unsigned char ch;
	//c->stack = (char *)malloc(sizeof(char) * c->sz);这里不要这么写 因为这个对单纯解析字符串是有效的，但是要解析
	//数组或者obj是和所有的元素用一个c，这样子就有问题了要用context_push函数
	char *q, *head;
	q = head = context_push(c, sz = len * 6 + 2);
	*q++ = '"';
	for(i = 0; i < len; i++) {
		ch = (unsigned char)s[i];
		switch(ch) {
			case '\t' : *q++ = '\\'; *q++ = 't';break;
			case '\b' : *q++ = '\\'; *q++ = 'b';break;
			case '\n' : *q++ = '\\'; *q++ = 'n';break;
			case '\f' : *q++ = '\\'; *q++ = 'f';break;
			case '\r' : *q++ = '\\'; *q++ = 'r';break;
			case '\\' : *q++ = '\\'; *q++ = '\\';break;
			case '/'  : *q++ = '\\'; *q++ = '/';break;
			case '"'  : *q++ = '\\'; *q++ = '"';break;
			default   : {
				if(ch < 0x20) {
					//snprintf(context_push(c, 6), 6, "\u%04x", (unsigned char)ch);
					*q++ = '\\';*q++ = 'u';*q++ = '0';*q++ = '0';
					*q++ = hex[ch>>4];
					*q++ = hex[ch&0x0F];
				}else {
					*q++ = ch;
				}
			}
		}
	}
	*q++ = '"';
	c->top -= size - (q - head);
	return LEPT_PARSE_STRINGFY_OK;
}

static int lept_stringfy_arr(const lept_value *v, lept_context *c) {
	assert(v != NULL && c != NULL);
	assert(v->type == JSON_ARR);
	size_t i;
	PUTC(c, '[');
	for(i = 0; i < v->u.arr.sz; i++) {
		if(i > 0) {
			PUTC(c, ',');
		}
		lept_stringfy_value(&v->u.arr.e[i], c);
	}
	PUTC(c, ']');
	return LEPT_PARSE_STRINGFY_OK;
}

static int lept_stringfy_obj(const lept_value *v, lept_context *c) {
	assert(v != NULL && c != NULL);
	assert(v->type == JSON_OBJ);
	size_t i;
	PUTC(c, '{');
	for(i = 0; i < v->u.obj.sz; i++) {
		if(i > 0) {
			PUTC(c, ',');
		}
		lept_stringfy_string(c, v->u.obj.m[i].s, v->u.obj.m[i].len);
		PUTC(c, ':');
		lept_stringfy_value(&v->u.obj.m[i].v, c);
	}
	PUTC(c, '}');
	return LEPT_PARSE_STRINGFY_OK;
}

static int lept_string_number(const lept_value *v, lept_context *c) {
	assert(v != NULL && v->type == JSON_NUM && c != NULL);
	c->top -= 32 - sprintf(context_push(c, 32), "%.17g", v->u.num);//%g是可以以最小的宽带输出浮点数，就是根据浮点数选择以小数输出还是以科学计数法输出，17是可以输出后面17个小数
	return LEPT_PARSE_STRINGFY_OK;
}

static int lept_stringfy_value(const lept_value *v, lept_context *c) {
	assert(v!=NULL && c!=NULL);
	json_e type = v->type;
	switch(type) {
		case JSON_NONE   : return lept_stringfy_simple(c "null", 4);
		case JSON_FALSE  : return lept_stringfy_simple(v, "false", 5);
		case JSON_TRUE   : return lept_stringfy_simple(v, "true", 4);
		case JSON_NUM    : return lept_stringfy_number(v, c);
		case JSON_STR 	 : return lept_stringfy_string(v, c);
		case JSON_ARR    : return lept_stringfy_arr(v, c);
		case JSON_OBJ    : return lept_stringfy_obj(v, c);
	}
}

int lept_stringfy(const lept_value *v, char **json, size_t &len) {
	assert(v!=NULL);
	lept_context c;
	int ret;
	c.stack = (char *)malloc(LEPT_PARSE_STACK_INIT_SIZE * sizeof(char));
	c.top = 0; 
	c.sz = LEPT_PARSE_STACK_INIT_SIZE;
	ret = lept_stringfy_value(v, &c);
	if(ret == LEPT_PARSE_STRINGFY_OK) {
		PUT(c.stack, '\0');
		*json = c.stack
		if(len!=NULL) {
			*len = c.top;
		}
	}else {
		free(c.stack);
		*json = NULL;
	}
	return ret;
}

static int lept_is_equal_arr(const lept_value *lhs, const lept_value *rhs) {
	assert(lhs != NULL && rhs != NULL);
	assert(lhs->type == JSON_ARR && rhs->type == JSON_ARR);
	size_t lhs_sz, rhs_sz, i;
	lhs_sz = lhs->u.arr.sz;
	rhs_sz = rhs->u.arr.sz;
	if( lhs_sz != rhs_sz ) {
		return 0;
	}
	for( i = 0; i < lhs_sz; i++) {
		if(!lept_is_equal(lhs->u.arr.e[i], rhs->u.arr.e[i])) {
			return 0;
		}
	}
	return 1;
}

static int lept_is_equal_obj(const lept_value *lhs, const lept_value *rhs) {
	assert(lhs != NULL && rhs != NULL);
	assert(lhs->type == JSON_OBJ && rhs->type == JSON_OBJ);
	size_t i, j;
	if(lhs->u.obj.sz != rhs->u.obj.sz) return 0;
	/*下面是不对的，因为键值对是无序的 假设键是不重复的。
	for(i = 0; i < lhs->u.obj.sz; i++) {
		if(lhs->u.obj.m[i].len != rhs->u.obj.m[i].len || strncmp(lhs->u.obj.m[i].s, rhs->u.obj.m[i].s, lhs->u.obj.m[i].len) != 0) {
			return 0;
		}
		if(!lept_is_equal(&lhs->u.obj.m[i].v, &rhs->u.obj.m[i].v)) {
			return 0;
		}	
	}*/
	for(i = 0; i < lhs->u.obj.sz; i++) {
		j = lept_find_obj_index(rhs, lhs->u.obj.m[i].s, lhs->u.obj.m[i].len);
		if(!j) {
			return 0;
		}
		if(lhs->u.obj.m[i].len != rhs->u.obj.m[j].len || strncmp(lhs->u.obj.m[i].s, rhs->u.obj.m[j].s, lhs->u.obj.m[i].len) != 0) {
			return 0;
		}
		if(!lept_is_equal(&lhs->u.obj.m[i].v, &rhs->u.obj.m[j].v)) {
			return 0;
		}
	}
	return 1;
}
int lept_is_equal(const lept_value *lhs, const lept_value *rhs) {
	assert(lhs != NULL && rhs != NULL);
	if(lhs->type != rhs->type) {
		return 0;
	}
	json_e type = lhs->type;
	switch(type) {
		/*
		case JSON_NONE : 
		case JSON_FALSE:
		case JSON_TRUE : return 1;
		这里上述三个默认返回1就行了
		*/
		case JSON_NUM  : return DOUBLE_EQ(lhs->u.num, rhs->u.num);
		case JSON_STR  : {
			return (lhs->u.s.len == rhs->u.s.len) && 
				strncmp(lhs->u.s.str, rhs->u.s.str, lhs->u.s.len) == 0;
		}
		case JOSN_ARR  : return lept_is_equal_arr(lhs, rhs);
		case JSON_OBJ  : return lept_is_equal_obj(lhs, rhs);
		default 	   : return 1;
	}
}

void lept_copy(lept_value *dst, const lept_value *src) {
	assert(dst != NULL && src != NULL && dst != src);//判断下src和dst不相等 因为下面用到了memcpy 所以要对外来的进行assert
	size_t i;
	switch(src->type) {
		case JSON_STR : lept_set_val_str(dst, src->u.s.str, src->u.s.len); break;
		case JSON_ARR : {
			lept_free(dst);
			dst->u.arr.e = (lept_value *)malloc(src->u.arr.sz * sizeof(lept_value));
			dst->u.arr.sz = src->u.arr.sz;
			for(i = 0; i < dst->u.arr.sz; i++) {
				LEPT_INIT(&dst->u.arr.e[i]);
				lept_copy(dst->u.arr.e[i], src->u.arr.e[i]);
			}
			break;
		}
		case JSON_OBJ : {
			lept_free(dst);
			dst->u.obj.m = (lept_member *)malloc(src->u.obj.sz * sizeof(lept_member));
			dst->u.obj.sz = src->u.obj.sz;
			for(i = 0; i < dst->u.obj.sz; i++) {
				LEPT_INIT(&dst->u.obj.m[i].v);
				dst->u.obj.m[i].len = dst->u.obj.m[i].len;
				dst->u.obj.m[i].s = (char *)malloc((dst->u.obj.m[i].len + 1) * sizeof(char)); 
				memcpy(dst->u.obj.m[i].s, src->u.obj.m[i].s, dst->u.obj.m[i].len + 1);
				lept_copy(&dst->u.obj.m[i].v, &src->u.obj.m[i].v);
			}
			break;
		}
		default       : {
			lept_free(dst);
			memcpy(dst, src, sizeof(lept_value));
		}
	}
}

/*
这么写不太好 目的是为了接收src的资源 而不是使src置空
void lept_move(lept_value **dst, lept_value **src) {
	assert(dst != NULL && src != NULL && dst != src);
	lept_free(dst);
	*dst = *src;
	
}
*/

void lept_move(lept_value *dst, lept_value *src) {
	assert(dst != NULL && src != NULL && dst != src);
	lept_free(dst);
	memcpy(dst, src, sizeof(lept_value));
	LEPT_INIT(src);
}

void lept_swap(lept_value *dst, lept_value *src) {
	assert(dst != NULL && src != NULL);
	lept_value temp;
	if(dst != src) {  //这里在这个判断一下 但是允许相等
		memcpy(&temp, dst, sizeof(lept_value));
		memcpy(dst, src, sizeof(lept_value));
		memcpy(src, &temp, sizeof(lept_value));
	}
}

void lept_set_array(lept_value *v, size_t capacity) {
	assert(v != NULL && capacity >= 0);
	v->type = JSON_ARR;
	v->u.arr.capacity = capacity;
	v->u.arr.sz = 0;
	v->u.arr.e = (capacity != 0 ? (lept_value *)malloc(capacity * sizeof(lept_value)) : NULL);
}	

void lept_get_array_capacity(lept_value *v) {
	assert(v != NULL && v->type == JSON_ARR);
	return v->u.arr.capacity;
}

void lept_reserve_array(lept_value *v, size_t capacity) {
	assert(v != NULL && v->type == JSON_ARR && capacity > v->u.arr.capacity);
	v->u.arr.capacity = capacity;
	v->u.arr.e = (lept_value *)realloc((void *)v->u.arr.e, capacity);
	assert(v->u.arr.e != NULL);
}

void lept_shrink_array(lept_value v*, size_t capacity) {
	assert(v != NULL && v->type == JSON_ARR && capacity < v->u.arr.capacity);
	v->u.arr.capacity = capacity;
	v->u.arr.e = (lept_value *)realloc((void *)v->u.arr.e, capacity);
	assert(v->u.arr.e != NULL);
}

lept_value *lept_push_back_arr_elem(lept_value *arr) {
	assert(arr != NULL && arr->type == JSON_ARR);
	/*sz = lepe_get_array_sz(*arr);
	capacity = lept_get_array_capacity(arr);
	*/
	//虽然提供了获取arr size以及arr capacity函数，但是内部没有必要调用
	if(arr->u.arr.capacity == arr->u.arr.sz) {
		//capacity += capacity >> 1;这里没有考虑为0的情况
		lept_reserve_array(arr, arr->u.arr.capacity == 0? 1 : arr->u.arr.capacity >> 1);  
	}
	LEPT_INIT(&arr->u.arr.e[sz]);
	return &arr->u.arr.e[arr->u.arr.sz++];
}

void lept_pop_back_arr_elem(lept_value *arr) {
	assert(arr != NULL && arr->type == JSON_ARR && arr->u.arr.sz > 0);
	lept_free(&arr->u.arr.e[--arr->u.arr.sz]);
}	

void lept_set_object(lept_value *v, size_t capacity) {
	assert(v != NULL && capacity >= 0);
	v->type = JSON_OBJ;
	v->u.obj.capacity = capacity;
	v->u.obj.sz = 0;
	v->u.obj.m = (capacity != 0 ? (lept_member *)malloc(capacity * sizeof(lept_member)) : NULL);
}

