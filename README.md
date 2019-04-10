# json-data-structure
json_encode and json_decode
# json-data-structure

## 简介

设计JSON数据结构，方便读取成员值

JSON是一种树形数据结构。具体JSON格式可参考：http://www.w3school.com.cn/json/json_syntax.asp

一个JSON，包含一个JSON值类型。

JSON值是一种可变类型，可以是：BOOL值，数值，字符串，数组，对象。

数组的成员类型是JSON值，也就是说一个数组，可能包含0到n个成员，每个成员都是JSON值。

对象由1到n个键值对组成，键值对由键（字符串）和值（即JSON值类型）组成。


JSON ::= Value 
Value ::= BOOLValue | Num | Str | Array | Object 
Array ::= Value* 
Object ::= KeyValue+ 
KeyValue ::= Key Value

## 要求

1. 支持json.h中要求的接口，视需要可以增加或修改API。

2. 能够用最简单的写法（最少代码）利用设计出来的API构建一个复杂的JSON值。比如，能构建出如下JSON值：
{
    "basic": {
        "enable": true,
        "ip": "200.200.3.61",
        "port": 389,
        "timeout": 10,
        "basedn": "aaa",
        "fd": -1,
        "maxcnt": 10000000000000000000000000000000000033333333333,
        "dns": ["200.200.0.1", "200.0.0.254"]
    },
    "advance": {
        "dns": [
            {"name":"huanan", "ip": "200.200.0.1"}, 
            {"name":"huabei", "ip": "200.0.0.254"}],
        "portpool": [130,131,132],
		"url": "http://200.200.0.4/main",
        "path": "/etc/sinfors",
        "value": 3.14
    }
}

3. 支持从JSON值中获取任意一个子孙成员的键值。

4. 保留有序列化的扩展能力。
将来需要扩展序列化能力，比如支持从文件中读入JSON值，以及把API构建的JSON值输出为YML格式文件。

5. 做到数据隐藏，除了API，不允许通过别的途径操作JSON值里面的内容，需要考虑设计怎么做好限制。

6. 需完成每个API的单元测试，单元测试覆盖率不低于90%

## 思路

1. 本习题的目的是让大家掌握一种设计/实现通用数据结构，以及为模块设计API的方法。

2. 需要考虑API的适用场景是什么，怎么让设计出来的API更好用，更容易调测，更容易扩展，更可靠。
