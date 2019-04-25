#include<stdio.h>
#include"json_new.h"
//测试文件里面的变量函数只有本单元用，所以用static声明。
static int main_ret = 0;
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

#define EXPECT_EQ_INT(expect, actual) EXPECT_EQ_BASE(expect==actual, expect, actual, %d);
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
static void test_parse_invalid() {
	lept_value val;
	EXPECT_EQ_INT(LEPT_PARSE_INVALID_VALUE, lept_parse(&val, "dfd"));
}
static void test_parse() {
	test_parse_null();
	test_parse_bol();
	test_parse_invalid();
}
int main(int argc, char* argv[])
{
	test_parse();
}