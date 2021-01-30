#define _CRT_SECURE_NO_WARNINGS
#include<stdio.h>
#include<stdlib.h>
#include<Windows.h>
#include<string.h>
#include<ctype.h>
#include <setjmp.h>

#define MAXKEY 1000
#define MAXLEN 10000
#define SIZE 100
#define CLEAR memset(tmp, 0, sizeof(TkWord));memset(s, 0, 100);j = 0;
#define SAVE(str,t) {wordlist[wpos].tkcod = t;strcpy(wordlist[wpos].sp, str);wordlist[wpos].row=row_num;wpos++;}
#define GOERROR(etoken) row_num = pcur->row;longjmp(jmpbuf, etoken);
#define IS_TYPE(sym) (KW_INT<=sym&&sym<=KW_VOID)
enum e_TokenCode
{
	/* 运算符及分隔符 */
	TK_PLUS,		// + 加号
	TK_MINUS,		// - 减号
	TK_STAR,		// * 星号
	TK_DIVIDE,		// / 除号
	TK_MOD,			// % 求余运算符
	TK_EQ,			// == 等于号
	TK_NEQ,			// != 不等于号
	TK_LT,			// < 小于号
	TK_LEQ,			// <= 小于等于号
	TK_GT,			// > 大于号
	TK_GEQ,			// >= 大于等于号
	TK_ASSIGN,		// = 赋值运算符 
	TK_POINTSTO,	// -> 指向结构体成员运算符
	TK_DOT,			// . 结构体成员运算符
	TK_AND,         // & 地址与运算符
	TK_OPENPA,		// ( 左圆括号
	TK_CLOSEPA,		// ) 右圆括号
	TK_OPENBR,		// [ 左中括号
	TK_CLOSEBR,		// ] 右中括号
	TK_BEGIN,		// { 左大括号
	TK_END,			// } 右大括号
	TK_FOR_END,		// } 右大括号
	TK_SEMICOLON,	// ; 分号    
	TK_COMMA,		// , 逗号
	TK_ELLIPSIS,	// ... 省略号
	TK_ANDAND,		//&& 逻辑与
	TK_OR,			//|| 逻辑或
	TK_NOT,			//! 逻辑非
	TK_EOF,			// 文件结束符

	/* 常量 */
	TK_CINT,		// 整型常量
	TK_CFLOAT,		// 浮点型常量
	TK_CCHAR,		// 字符常量
	TK_CSTR,		// 字符串常量

	/* 关键字 */
	KW_INT,			// int关键字
	KW_SHORT,		// short关键字
	KW_FLOAT,		// float关键字
	KW_CHAR,		// char关键字
	KW_VOID,		// void关键字  
	KW_STRUCT,		// struct关键字   
	KW_IF,			// if关键字
	KW_ELSE,		// else关键字
	KW_FOR,			// for关键字
	KW_WHILE,		// while关键字
	KW_CONTINUE,	// continue关键字
	KW_BREAK,		// break关键字   
	KW_RETURN,		// return关键字
	KW_SIZEOF,		// sizeof关键字(其实是运算符）

	/* 标识符 ,自己定义的变量和函数*/
	TK_VAR,
	TK_FUNC
};
enum Error_type {
	PROGRAM_BEGIN,		//执行语法分析
	SUCC,				//成功
	SOMETHINGWRONG,		//未知错误
	LACK_SEMICOLON,		//缺少分号
	RETURN_ERROR,		//return类型错误
	BREAK_LACK_SEMICOLON,		//break缺少分号
	CONTINUE_LACK_SEMICOLON,	//continue缺少分号
	WRONGASSIGNED,		//非赋值表达式中使用等号
	NO_CYCLE,			//不在循环体中
	EXTER_DEC,			//外部声明处出现错误
	LACK_TYPE,			//缺少类型
	VRA_WRONG,			//变量错误
	FUNC_WRONG,			//函数定义或声明错误
	CALLERROR,			//函数调用错误
	REDEFINE,			//变量或函数重定义
	NOT_DEFINE,			//未定义使用
	NOT_DECLARE,        //函数未声明
	FUNC_TYPE,			//函数参数或返回值类型错误
	LEFTERROR,			//赋值语句左值错误
	LACK_IDENT,			//缺少标识符
	LACK_OPENPA,		//缺少左圆括号
	LACK_CLOSEPA,		//缺少右圆括号
	LACK_OPENBR,		//缺少左中括号
	LACK_CLOSEBR,		//缺少右中括号
	LACK_BEGIN,			//缺少左花括号
	LACK_END,			//缺少右花括号
	NESTED_FUNC_DECLEAR,//函数嵌套定义
	IF_LACK_OPENPA,		//if缺少左圆括号
	IF_LACK_CLOSEPA,	//if缺少右圆括号
	FOR_LACK_OPENPA,	//for缺少左圆括号
	FOR_LACK_CLOSEPA,	//for缺少右圆括号
	LACK_PRIMARY,		//初等表达式错误
	LACK_ST,			//缺少语句
	CONTINUE_ERROR,		//continue使用错误
	ELSE_ERROR,			//else使用错误
	FOR_ERROR,			//for使用错误
	WHILE_LACK_OPENPA,	//while缺少左圆括号
	WHILE_LACK_CLOSEPA,	//while缺少右圆括号
	WHILE_ERROR,		//while使用错误
	MALLOCFAIL,			//堆空间分配失败
	
};
typedef enum e_TokenCode tokencode;
typedef struct Tk { //查token哈希表专用结构体
	int tkcode;	//token
	char spe[100];	//字符串
	struct Tk* next;
}TkWord, * pTkWord;

typedef struct Tk2 { //函数哈希表专用结构体
	char sp[SIZE];	//字符串
	int dec;		//标记，为0代表已声明未定义，1代表已定义
	int tkr;		//返回类型token
	int tk[SIZE];	//传参token
	struct Tk2* next;
}Sfunc, * pSfunc;

typedef struct { //存储专用结构体
	int tkcod;	//token
	char sp[100];	//字符串
	int row;
}words, *pWord;
/*------------------------词法分析函数声明----------------------------*/
int elf_hash(char key[]);
void estab_hash(char str[], int tkco);
void search_hash_print(char str[]);
void estab_id_hash(char str[],int tkco);
void int_or_float(char*);
void line_analyse(char* c,FILE *fp);
void Print(char str[], tokencode token);
void set_white();

/*------------------------语法分析函数声明----------------------------*/
void Compile();
void NextNode();
int trans_to_ctype(int kw);
void var_estab_hash(char str[], int tkco);
void func_estab_hash(char str[], int tkco, int tklist[],int sta);
int search_vhash(char str[]);
pSfunc search_fhash(char str[]);
void External_dec();
void type_Statement();
void Variate(int i);
void Func();
int Func_use();
void Func_Virate(int tlist[], int sta);
void Func_Body();
void call_list(int*);
void For_Process();
void If_Process();
void Else_Process();
void While_Process();
void Return_Process();
void one_line();
void is_bool();
void A_Process(int);
int Na_Variate(int);
void Assign_Variate(int i,int sta);
void Primary();
void Next();
void E();
void AO();
void RE();
void AD();
void MU();
void UN();
void PR();
void SE();
