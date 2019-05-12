#include<stdio.h>
#include"json_new.h"
//测试文件里面的变量函数只有本单元用，所以用static声明。
static int main_ret = 0;//main_ret返回值，成功为0失败为1
static int test_count = 0;
static int test_pass  = 0;
//这个宏可以打印错误到标准输出，这个用函数写不是很好，因为要输出行号，要是用函数就是输出固定的行号了。
//另外format这个用的很好，因为expect以及actual可能是不同类型的，用这种方式可以忽略类型考虑
#define EXPECT_EQ_BASE(is_eq, expect, actual, format) \
	do {\
		test_count += 1;\
		if (is_eq) {\
			test_pass += 1;\
		}else {\
			fprintf(stderr, "%s %d expect" format "actual" format "\n",__FILE__, __LINE__, expect, actual);\
			main_ret = 1;\
		}\
	}while(0)   //这里没有加;号 是要在后面加

#define EXPECT_EQ_INT(expect, actual) EXPECT_EQ_BASE(expect==actual, expect, actual, %d)
#define EXPECT_EQ_DOUBLE(expect, actual) EXPECT_EQ_BASE(expect==actual, expect, actual, %f)//浮点数直接用==比较不好但是这里不做处理了
#define EXPECT_EQ_STRING(expect, ptr, len) EXPECT_EQ_BASE(strncmp(expect, ptr, len) == 0, expect, ptr, %s)

//这里可以定义变量，因为用do{}while包围起来的作用域
#define TEST_NUMBER(expect, json_num) \
	do { \
		lept_value val_number;\
		EXPECT_EQ_INT(LEPT_PARSE_OK, lept_value(&val_number, json_num));\
		EXPECT_EQ_DOUBLE(expect, lept_get_number(val_number));\
		EXPECT_EQ_INT(JSON_NUM, lept_get_type(val_number));\
	}while(0)

//在字符串这里引进了释放lept_value的操作，进行完一个测试后，该值就没有用了 我们将其释放掉
//另外任何变量要考虑初始化 我们解析其他值包括true false number都要进行初始化以及释放操作
#define TEST_STRING(expect, json_str) \
	do { \
		lept_value val_str;\
		LEPT_INIT(val_str);\
		EXPECT_EQ_INT(LEPT_PARSE_OK, lept_value_parse(&val_str, json_str));\
		EXPECT_EQ_STRING(expect, lept_get_string(val_str), lept_get_string_len(val_str));\
		EXPECT_EQ_INT(JSON_STR, lept_get_type(val_str));\
		lept_free(&val_str);\
	}while(0)
		
#define TEST_ERROR(error, json_val)\
	do {\
		lept_value val;\
		EXPECT_EQ_INT(error, lept_parse(&val, json_val));\
		EXPECT_EQ_INT(JSON_NONE, lept_get_type(val));\
	}while(0)

#define TEST_ACCESS_STRING(str, len) \
	do {\
		lept_value val;\
		LEPT_INIT(val);\
		lept_set_val_str(val, str, len);\
		EXPECT_EQ_STRING(str, lept_get_string(val), len);\
		EXPECT_EQ_INT(len, lept_get_string_len(val));\
		lept_free(&val);\
	}while(0)

#define TEST_ROUNDTRIP(json) \
	do{\
		lept_value v;\
		char *s;\
		size_t len;\
		LEPT_INIT(&v);\
		lept_parse(&v, json);\
		EXPECT_EQ_INT(LEPT_PARSE_OK, lept_stringfy(&v, &s, &len));\
		EXPECT_EQ_STRING(json, s, len);\
		free(s);\
		lept_free(&v);\
	}while(0)
		
static void test_parse_null() {
	lept_value val_null;
	//json值定义是  JSON-text = ws value ws  ws为单个或多个空格 制表 回车 换行组成的 所以要有提取value的一步
	//因为对null的解析不会得到值 只会得到类型，所以我们的单元测试只对类型在进行测试。基于此将bol分为true和false。
	EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&val_null, "null"));
	EXPECT_EQ_INT(JSON_NONE, lept_get_type(val_null));
}

static void test_parse_bol() {
	lept_value val_ture,val_false;
	EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&val_true, "true"));
	EXPECT_EQ_INT(JSON_TRUE, lept_get_type(val_true));
	EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&val_false, "false"));
	EXPECT_EQ_INT(JSON_FALSE, lept_get_type(val_false));
}

static void test_parse_number() {
	/*
	这有点重复完全可以写成一个宏
	EXPECT_EQ_INT(LEPT_PARSE_OK, lept_value(&val_number_1, "245.8"));
	EXPECT_EQ_DOUBLE(254.8, lept_get_number(val_number_1));
	EXPECT_EQ_INT(JSON_NUM, lept_get_type(val_number_1));
    */
	TEST_NUMBER(-12.23e+19, "-12.23e+19");
	TEST_NUMBER(-12e-19, "-12e-19");
	TEST_NUMBER(123, "123");
}

static void test_parse_string() {
	TEST_STRING("abc", "\"abc\"");
	TEST_STRING("abc\t", "\"abc\\t\"");
	TEST_STRING("0xE20x820xACabc", "\"\\U20ACabc\"");
}

static void test_parse_object() {
	lept_value val;
	LEPT_INIT(&val);//使用lept_value的时候先初始化，因为用完一个变量可能会进行释放，这时候需要根据type进行lept_free
	EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&val, "{\"123\" : \"shi\", \"name\" : null}"));
	EXPECT_EQ_INT(2, lept_get_obj_size(&val));
	EXPECT_EQ_INT(JSON_STR, lept_get_type(*lept_get_obj_value(&val, 0)));
	EXPECT_EQ_STRING("123", lept_get_obj_key(&val, 0), lept_get_obj_key_len(&val, 0));
	lept_free(&val);
}
static void test_parse_arr() {
	lept_value json_arr;
	LEPT_INIT(&json_arr);
	EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&json_arr, "[null, "123", 123, false, [1,2,3]]"));
	EXPECT_EQ_INT(LEPT_PARSE_OK, lept_get_type(json_arr));
	EXPECT_EQ_INT(5, lept_get_array_size());
	EXPECT_EQ_INT(JSON_NONE, lept_get_type(*(lept_get_array_elem(json_arr, 0))));
	EXPECT_EQ_STRING("123", lept_get_string(*(lept_get_array_elem(json_arr, 1))));
	lept_free(&json_arr);
}

static void test_access_string() {
	TEST_ACCESS_STRING("shixiaoxiao", 11);
	TEST_ACCESS_STRING("0xEC0xBC", 8);
}

static void test_access_number() {
	lept_value number;
	LEPT_INIT(&number);
	lept_set_string(&number, "shi", 3);
	lept_set_number(&number, 123.345);
	EXPECT_EQ_DOUBLE(123.345, lept_get_number(number));
	lept_free(number);
}

static void test_parse_invalid() {
	TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "dfs");
	TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "nul");
	TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "fals");
	
	TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "+123");
	TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "0123");
	TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "12eab");
}

static void test_parse_number_beyond_limit() {
	lept_value val_neg_min,val_neg_max,val_posi_min,val_posi_max;
	EXPECT_EQ_INT(LEPT_PARSE_NUMBER_BEYOND_LIMIT, lept_parse(&val_neg_min, "-4.94065645841246544E-325"));//-324这两个谁大啊
	EXPECT_EQ_INT(LEPT_PARSE_NUMBER_BEYOND_LIMIT, lept_parse(&val_neg_max, "-1.79769313486231570E+309"));//+308 
	EXPECT_EQ_INT(LEPT_PARSE_NUMBER_BEYOND_LIMIT, lept_parse(&val_posi_min, "4.94065645841246544E-325"));
	EXPECT_EQ_INT(LEPT_PARSE_NUMBER_BEYOND_LIMIT, lept_parse(&val_posi_max, "1.79769313486231570E+309"));
}

static void test_parse_expect_val() {
	TEST_ERROR(LEPT_PARSE_EXPECT_VALUE, " ");
	TEST_ERROR(LEPT_PARSE_EXPECT_VALUE, "");
	TEST_ERROR(LEPT_PARSE_EXPECT_VALUE, "  \r\n");
}

static void test_parse_root_not_singular() {
	TEST_ERROR(LEPT_PARSE_ROOT_NOT_SINGULAR, " null x");
	TEST_ERROR(LEPT_PARSE_ROOT_NOT_SINGULAR, " false  a");
}

static void test_parse_invalid_string_escape() {
	TEST_ERROR(LEPT_PARSE_INVALID_STRING_ESCAPE, "\"\\v\"");
    TEST_ERROR(LEPT_PARSE_INVALID_STRING_ESCAPE, "\"\\'\"");
    TEST_ERROR(LEPT_PARSE_INVALID_STRING_ESCAPE, "\"\\0\"");
    TEST_ERROR(LEPT_PARSE_INVALID_STRING_ESCAPE, "\"\\x12\"");
}

static void test_parse_invalid_unicode_surrogate() {
	TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\uDBFF\"");
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\uE000\"");
}

static void test_stringfy_array() {
	TEST_ROUNDTRIP("[]");
    TEST_ROUNDTRIP("[null,false,true,123,\"abc\",[1,2,3]]");
}

static void test_stringfy_number() {
	TEST_ROUNDTRIP("1.234e-20");
	TEST_ROUNDTRIP("1.0000000000000002"); /* the smallest number > 1 */
}

static void test_stringfy_object() {
	TEST_ROUNDTRIP("{}");
    TEST_ROUNDTRIP("{\"n\":null,\"f\":false,\"t\":true,\"i\":123,\"s\":\"abc\",\"a\":[1,2,3],\"o\":{\"1\":1,\"2\":2,\"3\":3}}");
}

static void test_stringfy_string() {
	TEST_ROUNDTRIP("\"Hello\\nWorld\"");
	TEST_ROUNDTRIP("\"\\\" \\\\ / \\b \\f \\n \\r \\t\"");
}

static void test_stringfy() {
	TEST_ROUNDTRIP("null");
	TEST_ROUNDTRIP("true");
	TEST_ROUNDTRIP("false");
	test_stringfy_number();
	test_stringfy_string();
	test_stringfy_array();
	test_stringfy_object();
}

static void test_parse() {
	test_parse_null();
	test_parse_bol();
	test_parse_invalid();
	test_parse_expect_val();
	test_parse_root_not_singular();
	test_parse_number();
	test_parse_string();
	test_access_string();
	test_parse_invalid_string_escape();
	test_parse_invalid_unicode_surrogate();
	test_parse_arr();
	test_parse_object();
	test_stringfy();
}

int main(int argc, char* argv[])
{
	test_parse();
	printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count, test_pass * 100.0 / test_count);
	return main_ret;
}