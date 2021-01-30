#include "func.h"
extern jmp_buf jmpbuf;
//把每一种已知符号放入哈希表的结构体中并哈希初始化
TkWord keywords[] = {
	{TK_PLUS,"+",NULL},
	{TK_MINUS,"-",NULL},
	{TK_STAR,"*",NULL},
	{TK_DIVIDE,"/",NULL},
	{TK_MOD,"%",NULL},
	{TK_EQ,"==",NULL},
	{TK_NEQ,"!=",NULL},
	{TK_LT,"<",NULL},
	{TK_LEQ,"<=",NULL},
	{TK_GT,">",NULL},
	{TK_GEQ,">=",NULL},
	{TK_ASSIGN,"=",NULL},
	{TK_POINTSTO,"->",NULL},
	{TK_DOT,".",NULL},
	{TK_AND,"&",NULL},
	{TK_OPENPA,"(",NULL},
	{TK_CLOSEPA,")",NULL},
	{TK_OPENBR,"[",NULL},
	{TK_CLOSEBR,"]",NULL},
	{TK_BEGIN,"{",NULL},
	{TK_END,"}",NULL},
	{TK_SEMICOLON,";",NULL},
	{TK_COMMA,",",NULL},
	{TK_ELLIPSIS,"...",NULL},
	{TK_ANDAND,"&&",NULL},
	{TK_OR,"||",NULL},
	{TK_NOT,"!",NULL},
	{TK_EOF,"End Of File",NULL},

	{TK_CINT,"整形常量",NULL},
	{TK_CFLOAT,"浮点型常量",NULL},
	{TK_CCHAR,"字符常量",NULL},
	{TK_CSTR,"字符串常量",NULL},

	{KW_CHAR,"char",NULL},
	{KW_SHORT,"short",NULL},
	{KW_INT,"int",NULL},
	{KW_FLOAT,"float",NULL},
	{KW_VOID,"void",NULL},
	{KW_STRUCT,"struct",NULL},

	{KW_IF,"if",NULL},
	{KW_ELSE,"else",NULL},
	{KW_FOR,"for",NULL},
	{KW_WHILE,"while",NULL},
	{KW_CONTINUE,"continue",NULL},
	{KW_BREAK,"break",NULL},
	{KW_RETURN,"return",NULL},
	{KW_SIZEOF,"sizeof",NULL},
};
pTkWord hashtable[MAXKEY] = { NULL };//词法查token哈希表
pSfunc hash_func[MAXKEY] = { NULL };//函数名哈希表
words wordlist[MAXLEN] = { 0 };//全局存储表
pWord pcur;
int dep,for_dep;
int wpos = 0;//全局存储表下标
int JmpJud;
int row_num;//全局行数
int flag = 1;//多行注释用
int pend, err;
int FT;
int main() {
	int keywordsLen = sizeof(keywords) / sizeof(TkWord);
	FILE* fp = fopen("test.txt", "r");
	if (!fp) {
		perror("fopen");
		return 0;
	}
	char buf[1024] = { 0 };
	int i = 0, j = 0;
	for (i = 0; i < sizeof(keywords) / sizeof(TkWord); i++)
	{
		estab_hash(keywords[i].spe, keywords[i].tkcode);
	}//初始化哈希表
	while (fgets(buf, sizeof(buf), fp) != NULL)
	{
		row_num++;
		line_analyse(buf,fp);
	}
	fclose(fp);
	/*-----------------------------------------语法分析检查结果------------------------------------*/
	char *res;
	JmpJud = setjmp(jmpbuf);
	set_white();
	switch (JmpJud)
	{
	case SOMETHINGWRONG:res = "编译失败，未知错误！"; break;
	case NOT_DECLARE:res = "编译失败，函数未声明！"; break;
	case CALLERROR:res = "编译失败，函数调用错误！"; break;
	case NOT_DEFINE:res = "编译失败，变量或函数未定义！"; break;
	case REDEFINE:res = "编译失败，变量或函数重定义！"; break;
	case NO_CYCLE:res = "编译失败，不在循环体或开关语句中！"; break;
	case FUNC_TYPE:res = "编译失败,函数参数或返回值类型错误！"; break;
	case MALLOCFAIL:res = "堆空间分配失败！"; break;
	case LACK_SEMICOLON:res = "编译失败，缺少';'！"; break;
	case EXTER_DEC:res = "编译失败，外部变量或函数错误！"; break;
	case LEFTERROR:res = "编译失败，赋值语句左值错误！"; break;
	case LACK_TYPE:res = "编译失败，缺少类型符！"; break;
	case WRONGASSIGNED:res = "编译失败，非赋值表达式中使用等号！"; break;
	case VRA_WRONG:res = "编译失败，变量错误！"; break;
	case FUNC_WRONG:res = "编译失败，函数声明或定义错误！"; break;
	case LACK_IDENT:res = "编译失败，缺少标识符！"; break;
	case LACK_OPENPA:res = "编译失败，缺少'('！"; break;
	case LACK_CLOSEPA:res = "编译失败，缺少' '！"; break;
	case LACK_OPENBR:res = "编译失败，缺少'['！"; break;
	case LACK_CLOSEBR:res = "编译失败，缺少']'！"; break;
	case LACK_BEGIN:res = "编译失败，缺少'{'！"; break;
	case LACK_END:res = "编译失败，缺少'}'！"; break;
	case NESTED_FUNC_DECLEAR:res = "编译失败，函数嵌套定义！"; break;
	case LACK_PRIMARY:res = "编译失败，初等表达式错误！"; break;
	case RETURN_ERROR:res = "编译失败，return类型错误！"; break;
	case BREAK_LACK_SEMICOLON:res = "编译失败，break缺少';'！"; break;
	case CONTINUE_LACK_SEMICOLON:res = "编译失败，continue缺少';'！"; break;
	case IF_LACK_OPENPA:res = "编译失败，if中缺少'('！"; break;
	case IF_LACK_CLOSEPA:res = "编译失败，if中缺少')'!"; break;
	case FOR_LACK_OPENPA:res = "编译失败，for缺少'('！"; break;
	case FOR_LACK_CLOSEPA:res = "编译失败，for缺少')'！"; break;
	case LACK_ST:res = "编译失败，缺少语句！"; break;
	case CONTINUE_ERROR:res = "编译失败，continue使用错误！"; break;
	case ELSE_ERROR:res = "编译失败，else使用错误！"; break;
	case FOR_ERROR:res = "编译失败，for使用错误！"; break;
	case WHILE_LACK_OPENPA:res = "编译失败，while缺少左圆括号！"; break;
	case WHILE_LACK_CLOSEPA:res = "编译失败，while缺少右圆括号！"; break;
	case WHILE_ERROR:res = "编译失败，while使用错误！"; break;
	case PROGRAM_BEGIN:Compile();
	default:
		res = "\n================成功 1 个，失败 0 个，跳过 0 个==============\n";
		break;
	}
	if (SUCC == JmpJud) {
		printf("\n\n================已完成项目code.c的编译工作===================");
		printf("\n%s", res);
		return 0;
	}
	else {
		printf("\n\n============%s============\n", res);
		printf("\n============== 错误行数：%d ===============\n", row_num);

	}
	return 0;
}