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
	}while(0)

int main(int argc, char* argv[])
{
	
}