#ifndef JSON_H_
#define JSON_H_

/**
 *  想想：
 *  1. 你的JSON接口是为什么场景设计的？
 *  2. 这些场景有什么特点，会怎么影响的API设计风格？
 *  3. 你的用户怎么用这些API构建程序？
 *  4. 怎么设计API，会让用户用起来更爽？
 *  5. 怎么设计API，会让这些API更容易调试、测试？
 */
#define MAX_NUM 2000
typedef enum json_e {
    JSON_NONE,
    JSON_BOL,
    JSON_STR,
    JSON_ARR,
    JSON_OBJ,
    JSON_INT,
} json_e;

typedef unsigned int BOOL;
typedef unsigned int U32;
typedef struct value JSON;

/**
 *  \brief 从对象类型的json中获取名字为key的JSON值
 *  
 *  \param [in] json 对象类型的JSON值
 *  \param [in] key 键名
 *  \return 键值
 *  
 *  \details 要求json是个对象类型
 */
const JSON *json_get_member(const JSON *json, const char *key, bool &is_exist);
/**
 *  \brief 从数组类型的json中获取索引为idx的值
 *      
 *  \param [in] json 数组类型的JSON值 
 *  \param [in] idx 索引号
 *  \return 数组中第idx个位置的值
 *  
 *  \details 要求json是个数组
 */
const JSON *json_get_index(const JSON *json, U32 idx)

/**
 *  \brief 新建一个type类型的JSON值，采用缺省值初始化
 *  
 *  \param [in] type JSON值的类型，见json_e的定义
 *  \return 堆分配的JSON值
 *  
 *  \details 
 *  1. 对于数值，初始化为0
 *  2. 对于BOOL，初始化为FALSE
 *  3. 对于字符串，初始化为NULL
 *  4. 对于OBJ，初始化为空对象
 *  5. 对于ARR，初始化为空数组
 */
JSON *json_value_new(json_e type);

//-----------------------------------------------------------------------------
//  方案1
//-----------------------------------------------------------------------------
int json_get_int(const JSON *json, const char *key, int def);
BOOL json_get_bool(const JSON *json, const char *key);
/**
 * 获取JSON对象中键名为key的值，如果获取不到，则返回缺省值def
 * 如果json不是对象类型，则返回def
 * 如果对应的值为其它类型，先将其转换为字符串
 * 如: 
 *  json: {"key": "str"}
 *  json_get_str(json, "key", NULL) = "str"
 *  json_get_str(json, "noexist", NULL) = NULL
 *  json_get_str(json, "noexist", "") = ""
 *  
 *  想想：如果key对应的JSON值是个对象或数组，怎么办？
 */
const char *json_get_str(const JSON *json, const char *key, const char *def);



//  想想: 如果对应的键已经存在该怎么办，如果不存在该怎么办？要不要提供一个json_add_xxx版本的分别应对这两种场景？
BOOL json_set_int(JSON *json, const char *key, int val);
BOOL json_set_bool(JSON *json, const char *key, BOOL val);
BOOL json_set_str(JSON *json, const char *key, const char *val);
//-----------------------------------------------------------------------------
//  方案2
//-----------------------------------------------------------------------------
int json_get_int(const JSON *json, int def);
BOOL json_get_bool(const JSON *json);
const char *json_get_str(const JSON *json, const char *def);
JSON *json_new_int(int val);
JSON *json_new_bool(BOOL val);
JSON *json_new_str(const char *str);

//const JSON *json_get_member(const JSON *json, const char *key);
//const JSON *json_get_index(const JSON *json, U32 idx);
//  想想：val应该是堆分配，还是栈分配？如果json_add_member失败，该由谁来释放val？
void json_add_member(JSON *json, const char *key, JSON *val);
JSON *json_add_index(JSON *json, U32 idx, JSON *val);


//-----------------------------------------------------------------------------
//  TODO: 增加你认为还应该增加的接口
//自定义的函数为了得到字符串类型的值
void json_encode_obj(const JSON *json, const char *def, char *arr_char);
void json_encode_arr(value *val, char *res);
void encode_value(value *val, char *pre_val, int *pre_val_index);
//-------------------------------------------
//
//1. 增加键值对；
//2. 增加数组元素
//3. 创建JSON
//4. 释放JSON
//...


#endif