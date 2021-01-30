#include "func.h"
#include "stack.h"
/*--------------------------------------全局变量区------------------------------------------------*/
jmp_buf jmpbuf;
pSvar hash_var[MAXKEY] = {NULL};
extern pTkWord hashtable[];
extern pSfunc hash_func[];
extern words wordlist[];
extern pWord pcur;
extern int row_num;
extern int wpos;
extern flag;
extern int JmpJud;
extern int FT;
extern int pend, err;
extern int dep;//TODO:有栈了用不着
extern int for_dep;//仅记录最浅循环体
Sfunc kong;
VStack stelse;//if else栈
KStack st;//括号栈，小括号不压,中括号没压
Stack expre;//表达式括号栈，暂未用到
//变量栈群在函数中建立和销毁
/*----------------------------------------词法分析------------------------------------------------------*/
int elf_hash(char* key) {
	int h = 0, g, i = 0;
	while (key[i]) {
		h = (h << 4) + key[i++];
		g = h & 0xf0000000;
		if (g)
			h ^= g >> 24;
		h &= ~g;
	}
	return h % MAXKEY;
}
void set_white()
{
	HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(h, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
}
void Print(char* str, tokencode token) {
	HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
	if (token >= TK_VAR) {//变量名，或者函数名为灰色
		SetConsoleTextAttribute(h, FOREGROUND_INTENSITY);
	}
	else if (token >= KW_INT) {//关键字为绿色
		SetConsoleTextAttribute(h, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
	}
	else if (token >= TK_CINT) {//整型常量、浮点型常量、字符常量、字符串常量等为褐色
		SetConsoleTextAttribute(h, FOREGROUND_RED | FOREGROUND_GREEN);
	}
	else {//运算符为红色
		SetConsoleTextAttribute(h, FOREGROUND_RED | FOREGROUND_INTENSITY);
	}
	if (str[0] == EOF)
	{
		set_white();
	}
	printf("%s", str);
}
void estab_hash(char str[], int tkco)
{
	int k = elf_hash(str);
	pTkWord pcu = hashtable[k], pre = pcu;
	if (pcu == NULL) {
		pTkWord word = (pTkWord)calloc(1,sizeof(TkWord));
		if (word == NULL) {
			longjmp(jmpbuf, MALLOCFAIL);
		}
		word->next = NULL;
		strcpy(word->spe, str);
		hashtable[k] = word;
		word->tkcode = tkco;
		return;
	}
	while (pcu) {
		pre = pcu;
		pcu = pcu->next;
	}
	pTkWord word = (pTkWord)calloc(1,sizeof(TkWord));
	if (word == NULL) {
		longjmp(jmpbuf, MALLOCFAIL);
	}
		strcpy(word->spe, str);
		word->next = NULL;
		pre->next = word;
		word->tkcode = tkco;
}
void estab_id_hash(char str[],int tkco)
{
	int k = elf_hash(str);
	pTkWord pcu = hashtable[k], pre = pcu;
	if (pcu == NULL) {
		pTkWord word = (pTkWord)calloc(1,sizeof(TkWord));
		if (word == NULL) {
			longjmp(jmpbuf, MALLOCFAIL);
		}
		word->next = NULL;
		strcpy(word->spe, str);
		word->tkcode = tkco;
		SAVE(word->spe, tkco)
		Print(word->spe, word->tkcode);
		hashtable[k] = word;
		word->next = NULL;
		return;
	}
	while (pcu) {
		if (strcmp(pcu->spe, str) == 0) {
				SAVE(pcu->spe,pcu->tkcode)
				Print(pcu->spe, pcu->tkcode);
				return;
		}
		pre = pcu;
		pcu = pcu->next;
	}
	pTkWord word = (pTkWord)calloc(1,sizeof(TkWord));
	if (word == NULL) {
		longjmp(jmpbuf, MALLOCFAIL);
	}
		strcpy(word->spe, str);
		word->tkcode = tkco;
		word->next = NULL;
		pre->next = word;
		SAVE(word->spe, word->tkcode)
		if (pcu == NULL) {
				longjmp(jmpbuf, MALLOCFAIL);
			}
		Print(pcu->spe, pcu->tkcode);
		return;
}
void search_hash_print(char str[])
{
	int k = elf_hash(str);
	pTkWord pcu = hashtable[k], pre = pcu;
	while (pcu) {
		if (strcmp(pcu->spe, str) == 0) {
			SAVE(pcu->spe, pcu->tkcode)
			Print(str, pcu->tkcode);
			return;
		}
		pre = pcu;
		pcu = pcu->next;
	}
}
void int_or_float(char str[])
{
	int i=0,j = 0;
	while (str[i])
	{
		if (str[i] == '.')
		{
			j++;
		}
		i++;
	}
	if (j == 0)
	{
		SAVE(str, TK_CINT)
		Print(str, TK_CINT);
		return;
	}
	else if (j == 1)
	{
		SAVE(str, TK_CFLOAT)
		Print(str, TK_CFLOAT);
		return;
	}
	else
	{
		longjmp(jmpbuf,VRA_WRONG);
	}

}
void line_analyse(char c[],FILE *fp) {
	set_white();
	printf("%3d  ",row_num);
	int i = 0, j = 0;
	char s[100] = { 0 };
	pTkWord tmp=(pTkWord)calloc(1,sizeof(TkWord));
notes:
	if (flag == 0)
	{
		set_white();
		while (c[i])
		{
			if (c[i] == '*' && c[i + 1] == '/')
			{
				flag = 1;
				printf("%c", c[i++]);
				printf("%c", c[i++]);
				break;
			}
			printf("%c", c[i++]);	
		}
	}
	while (c[i]) {
		if (c[i] == '/' && c[i + 1] == '*') //多行注释，直接打，不存储
		{
			flag = 0;
			goto notes; //可以不用goto，但会增加判断次数
		}
		if (c[i] == ' ' || c[i] == '\n' || c[i] == '\t') {
			printf("%c", c[i++]);//此三类无色，直接打印，也不会出现在标识符中
		}
		else if (c[i] == '/' && c[i + 1] == '/') { //由于\0的存在，不会越界
			set_white();
			while (c[i]) {	//直接打完一行，不存储
				printf("%c", c[i++]);
			}
			return;
		}
		
		else if (isalpha(c[i]) || c[i] == '_') { //如果是字母,可能是关键字，也可能是标识符,注意不能数字开头
			while (isalnum(c[i]) || c[i] == '_') { //读到单词末尾,用s记录
				s[j] = c[i];
				j++;
				i++;
			}
			int k = i;
			while (c[k] == ' ') {		//忽略连续空格
				k++;
			}
			if (c[k] == '(')
			{
				estab_id_hash(s, TK_FUNC);//哈希查找并区分关键字和标识符(函数名或变量名也要区分）
			}
			else
			{
				estab_id_hash(s, TK_VAR);//如果是关键字不会使用传入的token
			}
			CLEAR
		}
		else if (isdigit(c[i])||(c[i]=='.'&&isdigit(c[i+1]))) {//如果是数字或点开头
			while (isdigit(c[i])||c[i]=='.'){
				s[j] = c[i];
				i++;
				j++;
			}
			int_or_float(s);//区分整形和浮点
			CLEAR
		}
		else if (c[i] == '\'') {//字符常量，不支持嵌套定义
			s[j] = c[i++];
			j++;
			while (c[i] != '\'') {//输入字符串，报错 todo
				s[j] = c[i];
				j++;
				i++;
			}
			s[j] = c[i]; 
			i++;
			if (j > 2)
			{
				printf("\nword error:invalid char");
				exit(0);
			}
			SAVE(s, TK_CCHAR)
			Print(s, TK_CCHAR);
			CLEAR
		}
		else if (c[i] == '\"') {//字符串常量
			s[j] = c[i++];
			j++;
			while (c[i] != '\"') {
				s[j] = c[i];
				i++;
				j++;
			}
			s[j] = c[i++]; //最后一个双引号
			SAVE(s, TK_CSTR)
			Print(s, TK_CSTR);
			CLEAR
		}
		else {
			if (c[i] == '=' && c[i + 1] == '=') {
				s[0] = c[i++];	s[1] = c[i++];
			}
			else if (c[i] == '!' && c[i + 1] == '=') {
				s[0] = c[i++];	s[1] = c[i++];
			}
			else if (c[i] == '<' && c[i + 1] == '=') {
				s[0] = c[i++];	s[1] = c[i++];
			}
			else if (c[i] == '>' && c[i + 1] == '=') {
				s[0] = c[i++];	s[1] = c[i++];
			}
			else if (c[i] == '-' && c[i + 1] == '>') {
				s[0] = c[i++];	s[1] = c[i++];
			}
			else if (c[i] == '&' && c[i + 1] == '&') {
				s[0] = c[i++];	s[1] = c[i++];
			}
			else if (c[i] == '|' && c[i + 1] == '|') {
				s[0] = c[i++];	s[1] = c[i++];
			}
			else { s[0] = c[i++]; } //单运算符
			search_hash_print(s);
			CLEAR
		}
	}
}

/*----------------------------------------语法分析-----------------------------------------------------*/
//语法分析入口
//凭空给一个main函数的声明,要求为INT型，无参
void Compile() {
	int tlist[SIZE] = { 0 };
	func_estab_hash("main", KW_INT, tlist, 0);
	JmpJud = SUCC;
	pcur = wordlist;
	External_dec();
}
void NextNode() {
	pcur++;
}
//变量转常量类型，记录在变量哈希中,便于类型判断
//函数名不转，返回类型在它的结构体中查找
int trans_to_ctype(int kw)
{
	if (kw == KW_INT)
	{
		return TK_CINT;
	}
	if (kw == KW_FLOAT)
	{
		return TK_CFLOAT;
	}
	if (kw == KW_SHORT)//暂时short和int不区分
	{
		return TK_CINT;
	}
	if (kw == KW_CHAR)
	{
		return TK_CCHAR;
	}
	GOERROR(SOMETHINGWRONG);//此处应该抛出一个错误
}
//变量名哈希，用于建立和重定义检查
void var_estab_hash(char str[], int tkco)
{
	int tknew=trans_to_ctype(tkco);//在变量哈希表中存为常量类型
	int k = elf_hash(str);
	pSvar pcu = hash_var[k], pre = pcu;
	if (pcu == NULL) {
		pSvar word = (pSvar)calloc(1,sizeof(Svar));
		if (word == NULL) {
			longjmp(jmpbuf, MALLOCFAIL);
		}
		word->next = NULL;
		strcpy(word->sp, str);
		hash_var[k] = word;
		word->tkcode = tknew;
		word->depth = dep;
		return;
	}
	while (pcu) {
		if (strcmp(pcu->sp, str) == 0) {
				if (dep == pcu->depth)//同层重定义
				{
					GOERROR(REDEFINE)
				}
				return;
				//TODO:非同层重定义，token和深度压栈，当退出变量所在当前深度时，弹栈

		}
		pre = pcu;
		pcu = pcu->next;
	}
	pSvar word = (pSvar)calloc(1,sizeof(Svar));
	if (word == NULL) {
		longjmp(jmpbuf, MALLOCFAIL);
	}
	strcpy(word->sp, str);
	word->next = NULL;
	pre->next = word;
	word->tkcode = tknew;
	word->depth = dep;
}
//函数名哈希,用于建立、实参验证和重定义验证,声明、定义对应0、1两种状态
void func_estab_hash(char str[], int tkco,int tklist[],int sta)
{
	int k = elf_hash(str);
	pSfunc pcu = hash_func[k], pre = pcu;
	if (pcu == NULL) {
		if (sta == 1){ //定义时未找到
			GOERROR(NOT_DECLARE)
		}
		pSfunc word = (pSfunc)calloc(1,sizeof(Sfunc));
		if (word == NULL) {
			longjmp(jmpbuf, MALLOCFAIL);
		}
		word->next = NULL;
		strcpy(word->sp, str);
		hash_func[k] = word;
		word-> tkr= tkco;
		memcpy(word->tk, tklist, SIZE*sizeof(int));
		return;
	}
	while (pcu) {
		if (strcmp(pcu->sp, str) == 0)//函数名相同
		{
			if (memcmp(pcu->tk, tklist, SIZE * sizeof(int)) == 0 && pcu->tkr == tkco)//传参和返回值相同
			{
				if (sta == 0)return;//相同声明可重复
				if (sta == 1)
				{
					if (pcu->dec == 1)//定义时找到且已定义
					{
						GOERROR(REDEFINE)
					}
					else  //定义时找到且未定义
					{
						pcu->dec = 1;
						return;
					}
						
				}
			}
			GOERROR(FUNC_TYPE)//可以抛出更精确的错误
		}
		pre = pcu;
		pcu = pcu->next;
	}
	if (sta == 1) { //定义时未找到
		GOERROR(NOT_DECLARE)
	}
	pSfunc word = (pSfunc)calloc(1,sizeof(Sfunc));
	if (word == NULL) {
		longjmp(jmpbuf, MALLOCFAIL);
	}
	strcpy(word->sp, str);
	word->next = NULL;
	word->tkr = tkco;
	memcpy(word->tk, tklist, SIZE * sizeof(int));
}
//仅查变量哈希，没找到报错，找到返回转换后的token
int search_vhash(char str[])
{
	int k = elf_hash(str);
	pSvar pcu = hash_var[k], pre = pcu;
	while (pcu) {
		if (strcmp(pcu->sp, str) == 0 && pcu->depth <= dep) {
			if (dep < pcu->depth)//未定义；未正确返回类型，待完善
			{
				GOERROR(NOT_DEFINE)
			}
			return pcu->tkcode;
		}
		pre = pcu;
		pcu = pcu->next;
	}
	return 0;
}
//仅查函数哈希，返回0表示没找到，找到后返回其返回值的token
//TODO:可能不需要，因为调用必须要验证，走func_estab_hash
pSfunc search_fhash(char str[])
{
	int k = elf_hash(str);
	pSfunc pcu = hash_func[k], pre = pcu;
	while (pcu) {
		if (strcmp(pcu->sp, str) == 0) {
			return pcu;
		}
		pre = pcu;
		pcu = pcu->next;
	}
	return &kong;
}
//外部声明
void External_dec() {   //退出外部声明时 pcur为当前声明的;或函数的} 
	if (pcur->row == 0) {//判断是否到了结尾
		longjmp(jmpbuf, SUCC);
		return;
	}
	else if (pcur->tkcod == TK_SEMICOLON) {//判断是否为分号
		//跳转到外部声明出现错误
		GOERROR(EXTER_DEC)
	}
	else if (pcur->tkcod == TK_END) {//是否是右大括号
		GOERROR(LACK_BEGIN)

	}
	else if (!IS_TYPE(pcur->tkcod)) {//判断如果不是一个int，float等类型声明符
		GOERROR(LACK_TYPE)
	}
	else {
		type_Statement();//变量，或函数声明的处理，判断是否为函数定义
	}
	NextNode();
	External_dec();
}
//外部声明为变量或函数  变量，pcur为分号退出; 函数，分号退出为声明，}退出为定义
void type_Statement() {
	if ((pcur + 1)->tkcod == TK_VAR) {//下一个成员是否为变量
		int i = pcur->tkcod;
		NextNode();//pcur指向该变量
		Variate(i);
	}
	else if ((pcur + 1)->tkcod == TK_FUNC) {
		Func();//函数声明，函数实现都会从这里进去
	}
	else {
		GOERROR(EXTER_DEC)
	}
}
//全局变量名，pcur指向变量名
//注意函数内变量定义的右值可以为函数
//注意变量类型和是否定义
void Variate(int i) {
	if (pcur->tkcod == TK_VAR)
	{
		var_estab_hash(pcur->sp, i);//建变量哈希,压栈
	}
	if ((pcur+1)->tkcod == TK_SEMICOLON)
	{
		NextNode();
		return;
	}
	else if ((pcur + 1)->tkcod == TK_COMMA) {//如果下一个是逗号
		NextNode();
		if ((pcur+1)->tkcod != TK_VAR) {
			GOERROR(VRA_WRONG)
		}
		else {
			NextNode();
			Variate(i);
		}
	}
	else if (((pcur + 1)->tkcod) == TK_ASSIGN) {//如果下一个是等号
		NextNode();
		NextNode();
		switch (i) {
		case KW_SHORT://这三个类型判断标准相同
		case KW_CHAR:
		case KW_INT:
			if (pcur->tkcod == TK_CINT || pcur->tkcod == TK_CCHAR || pcur->tkcod == TK_VAR || pcur->tkcod == TK_OPENPA||pcur->tkcod == TK_FUNC) {//是整型常量，表达式分析，读下一个继续验证
				pWord pre = pcur;//表达式开始前回退到pre
				while (pcur->tkcod != TK_SEMICOLON && pcur->tkcod != TK_COMMA)//遍历到逗号或分号，出现小数就报错
				{
					if (pcur->tkcod == TK_ASSIGN)
					{
						GOERROR(WRONGASSIGNED)
					}
					if (pcur->tkcod == TK_CFLOAT)
					{
						GOERROR(VRA_WRONG)
					}
					if (pcur->tkcod == TK_FUNC)
					{
						int i = Func_use();//此时已经走到函数末尾
						if (i == KW_VOID)//不考虑指针
						{
							GOERROR(FUNC_TYPE)
						}
						if (i == KW_FLOAT)
						{
							GOERROR(VRA_WRONG)
						}
					}
					pcur++;
				}
				pcur = pre;
				Primary();
				pcur--;
				Variate(i);
			}
			else {
				GOERROR(VRA_WRONG)
			}
			break;
		case KW_FLOAT:
			if (pcur->tkcod >= TK_CINT && pcur->tkcod <= TK_CCHAR || pcur->tkcod == TK_VAR || pcur->tkcod == TK_OPENPA || pcur->tkcod == TK_FUNC) {//表达式分析，读下一个继续验证,要有小数
				pWord pre = pcur;//表达式开始前回退到pre
				while (pcur->tkcod != TK_SEMICOLON && pcur->tkcod != TK_COMMA)//遍历到逗号或分号，不出现小数就报错
				{
					if (pcur->tkcod == TK_ASSIGN)
					{
						GOERROR(WRONGASSIGNED)
					}
					if (pcur->tkcod == TK_CFLOAT)
					{
						goto float_right;
					}
					if (pcur->tkcod == TK_FUNC)
					{
						int i = Func_use();//此时已经走到函数末尾
						if (i == KW_VOID)//不考虑指针
						{
							GOERROR(FUNC_TYPE)
						}
						if (i == KW_FLOAT)
						{
							goto float_right;
						}
					}
					pcur++;
				}
				GOERROR(VRA_WRONG)
	float_right:
				pcur = pre;
				Primary();
				pcur--;
				Variate(i);
			}
			else {
				GOERROR(VRA_WRONG)
			}
			break;
		default:
			GOERROR(VRA_WRONG)
				break;
		}
	}
	else {
		GOERROR(LACK_SEMICOLON)
	}
}
//和上面类似，区别是遇到逗号重传类型，并且只查询哈希不建立
//这类语句也可以没有等号，就写一个变量
//这类语句也可以出现在布尔表达式中
void Assign_Variate(int i,int sta) {
	if ((pcur + 1)->tkcod == TK_CLOSEPA && sta == 1)
	{
		NextNode();
		return;
	}
	if ((pcur + 1)->tkcod == TK_SEMICOLON && sta == 0)
	{
		NextNode();
		return;
	}
	if ((pcur + 1)->tkcod == TK_COMMA) {//如果是逗号
		NextNode();
		NextNode();
		int i = search_vhash(pcur->sp);
		if (pcur->tkcod != TK_VAR) {
			GOERROR(VRA_WRONG)
		}
		else if (i == 0)
		{
			GOERROR(NOT_DEFINE)
		}
		else {
			Assign_Variate(i,sta);
		}
	}
	else if (((pcur+1)->tkcod) == TK_ASSIGN) {//如果是等号，读下一个
		NextNode();
		NextNode();
		switch (i) {
		case TK_CCHAR:
		case TK_CINT:
			if (pcur->tkcod == TK_CINT || pcur->tkcod == TK_CCHAR || pcur->tkcod == TK_VAR || pcur->tkcod == TK_OPENPA || pcur->tkcod == TK_FUNC) {//是整型常量，表达式分析，读下一个继续验证
				pWord pre = pcur;//表达式开始前回退到pre
				while (pcur->tkcod != TK_SEMICOLON && pcur->tkcod != TK_COMMA&& pcur->tkcod != TK_CLOSEPA)//遍历到逗号或分号或右括号，出现小数就报错
				{
					if (pcur->tkcod == TK_ASSIGN)
					{
						GOERROR(WRONGASSIGNED)
					}
					if (pcur->tkcod == TK_CFLOAT)
					{
						GOERROR(VRA_WRONG)
					}
					if (pcur->tkcod == TK_FUNC)
					{
						int i = Func_use();//此时已经走到函数末尾，函数的右括号不会作为结束条件
						if (i == KW_VOID)//不考虑指针
						{
							GOERROR(FUNC_TYPE)
						}
						if (i == KW_FLOAT)
						{
							GOERROR(VRA_WRONG)
						}
					}
					pcur++;
				}
				pcur = pre;
				Primary();
				pcur--;
				Assign_Variate(i,sta);
			}
			else {
				GOERROR(VRA_WRONG)
			}
			break;
		case TK_CFLOAT:
			if (pcur->tkcod >= TK_CINT && pcur->tkcod <= TK_CCHAR || pcur->tkcod == TK_VAR || pcur->tkcod == TK_OPENPA || pcur->tkcod == TK_FUNC) {//表达式分析，读下一个继续验证,要有小数
				pWord pre = pcur;//表达式开始前回退到pre
				while (pcur->tkcod != TK_SEMICOLON && pcur->tkcod != TK_COMMA && pcur->tkcod != TK_CLOSEPA)//遍历到逗号或分号或右括号，不出现小数就报错
				{
					if (pcur->tkcod == TK_ASSIGN)
					{
						GOERROR(WRONGASSIGNED)
					}
					if (pcur->tkcod == TK_CFLOAT)
					{
						goto float_right;
					}
					if (pcur->tkcod == TK_FUNC)
					{
						int i = Func_use();//此时已经走到函数末尾
						if (i == KW_VOID)//不考虑指针
						{
							GOERROR(FUNC_TYPE)
						}
						if (i == KW_FLOAT)
						{
							goto float_right;
						}
					}
					pcur++;
				}
				GOERROR(VRA_WRONG)
					float_right:
				pcur = pre;
				Primary();
				pcur--;
				Assign_Variate(i,sta);
			}
			else {
				GOERROR(VRA_WRONG)
			}
			break;
		}
	}
	else {
		GOERROR(LACK_SEMICOLON)
	}
}

//是函数名  此时pcur为int，void等返回值类型
void Func() {
	FT = pcur->tkcod;//返回类型
	NextNode();//读到函数名
	char fname[100] = { 0 };
	strcpy(fname, pcur->sp);
	NextNode();//再走一次，读到左括号
	if (pcur->tkcod != TK_OPENPA) {
		GOERROR(LACK_OPENPA)
	}
	else {  //是左括号,记录形参token列表
		NextNode();
		int k = 0;
		int tlist[SIZE] = { 0 };
		while ((pcur + k)->tkcod != TK_CLOSEPA)//遍历到右括号
		{
			k++;
		}
		if ((pcur + k + 1)->tkcod == TK_SEMICOLON)
		{
			Func_Virate(tlist, 0);
			NextNode();
			func_estab_hash(fname, FT, tlist, 0);
			return;
		}else if ((pcur + k + 1)->tkcod == TK_BEGIN)
		{
			Func_Virate(tlist,1);
			NextNode();
			func_estab_hash(fname, FT, tlist, 1);
			push1(&st, pcur->sp, 0);
			dep++;
			for_dep = 1000;//初始化大深度
			Func_Body();//函数体处理
			if ((pcur + 1)->tkcod == TK_SEMICOLON) {
				NextNode();
			}
			return;
		}
		else
		{
			GOERROR(FUNC_WRONG)
		}
	}
}

//函数形参变量处理 pcur为第一个  处理到)返回 0为声明时，1为定义时
void Func_Virate(int tlist[],int sta) {
	if (pcur->tkcod == TK_CLOSEPA)
	{
		return;
	}
	if (!IS_TYPE(pcur->tkcod)) {
		GOERROR(FUNC_WRONG)
	}
	else {
		*tlist = trans_to_ctype(pcur->tkcod);
		NextNode();
		if (pcur->tkcod == TK_COMMA) {
			if (sta == 0)
			{
				NextNode();
				Func_Virate(tlist + 1, sta);
			}
			else
			{
				GOERROR(FUNC_WRONG)
			}
			
		}
		else if (pcur->tkcod == TK_VAR) {
			NextNode();
			if (pcur->tkcod == TK_CLOSEPA)
			{
				return;
			}
			else if (pcur->tkcod == TK_COMMA) {
				NextNode();
				Func_Virate(tlist + 1,sta);
			}
			else {
				GOERROR(FUNC_WRONG)
			}
		}
		else if (pcur->tkcod == TK_CLOSEPA&&sta==0)
		{
			return;
		}
		else {
			GOERROR(FUNC_WRONG)
		}
	}
}
//函数体处理 pcur为{    处理到 }退出
void Func_Body() {
	NextNode();
	if (pcur->tkcod == TK_END) {//定义为空 直接返回
		pop1(&st);
		dep--;
		return;
	}
	else {
		while (dep>0) {
			switch (pcur->tkcod)
			{
			case KW_CHAR:
			case KW_SHORT:
			case KW_INT:
			case KW_FLOAT:
				if ((pcur + 1)->tkcod == TK_VAR) {//下一个成员是否为变量
					int i = pcur->tkcod;
					NextNode();//pcur指向该变量
					Variate(i);
					NextNode();//跳一个分号
					break;
				}
				else if ((pcur + 1)->tkcod == TK_FUNC)
				{
					GOERROR(NESTED_FUNC_DECLEAR)
				}
				else {
					GOERROR(VRA_WRONG)
				}
			case KW_IF:
				If_Process();
				break;
			case KW_ELSE:
				Else_Process();
				break;
			case KW_FOR:
				For_Process();
				break;
			case KW_WHILE:
				While_Process();
				break;
			case KW_RETURN:
				NextNode();
				Return_Process();
				break;
			case TK_BEGIN:
				push1(&st, pcur->sp, 0);
				dep++;
				NextNode();
				break;
			case TK_OPENBR:
				push1(&st, pcur->sp, 0);
				break;
			case TK_CLOSEBR:
			{	
				pKNode pu = pop1(&st);
				if (strcmp(pu->sp, "[") == 0 && dep > 1)
				{
					NextNode();
					break;
				}
				else {
					GOERROR(LACK_OPENBR)
				}
			}
			case TK_END:
			{
				if (dep == for_dep)//退出最浅循环体，深度重置
				{
					for_dep = 1000;
				}
				pKNode pu = pop1(&st);
				if (pu->type == 1 && strcmp(pu->sp,"{")==0 && dep > 1)
				{
					dep--;
					dep--;
					NextNode();
					break;
				}
				if (dep > 0)
				{
					dep--;
					NextNode();
					break;
				}
				else {
					GOERROR(LACK_BEGIN)
				}
			}
			case TK_VAR:
				A_Process(0);
				NextNode();
				break;
			case TK_FUNC://函数调用
				Func_use();
				NextNode();
				if (pcur->tkcod != TK_SEMICOLON) {
					GOERROR(LACK_SEMICOLON)
				}
				NextNode();
				break;
			case KW_CONTINUE://必须在循环体内
				if (dep < for_dep)
				{
					GOERROR(NO_CYCLE)
				}
				NextNode();
				if (pcur->tkcod != TK_SEMICOLON) {
					GOERROR(CONTINUE_LACK_SEMICOLON)
				}
				NextNode();
				break;
			case KW_BREAK:
				if (dep < for_dep)
				{
					GOERROR(NO_CYCLE)
				}
				NextNode();
				if (pcur->tkcod != TK_SEMICOLON) {
					GOERROR(BREAK_LACK_SEMICOLON)
				}
				NextNode();
				break;
			default:
				GOERROR(LACK_BEGIN)
				break;
			}
		}
	}
	pcur--;//回退到右大括号
	while (!empty2(&stelse))//把多余的if弹出
		pop2(&stelse);
	//TODO:销毁所有非全局变量
}

//函数调用 返回返回类型token
int Func_use() {
	pSfunc pcu = search_fhash(pcur->sp);
	if (pcu->tkr == 0 || pcu->dec == 0)
	{
		GOERROR(NOT_DEFINE)
	}
	NextNode();
	if (pcur->tkcod != TK_OPENPA) {
		GOERROR(FUNC_WRONG)
	}
	NextNode();
		if (pcur->tkcod == TK_BEGIN) {
			GOERROR(NESTED_FUNC_DECLEAR)
		}
	else
	{
		int tklist[SIZE] = { 0 };
		call_list(tklist);
		if (memcmp(pcu->tk, tklist, SIZE * sizeof(int)) != 0)//函数实参验证
		{
			GOERROR(CALLERROR)
		}
		return pcu->tkr;
	}
}
//函数调用参数收集
void call_list(int tlist[]) 
{
	if (pcur->tkcod == TK_CLOSEPA)
	{
		return;
	}
	if (pcur->tkcod == TK_VAR) {
		int i = search_vhash(pcur->sp);
		if (i == 0)
		{
			GOERROR(NOT_DEFINE)
		}
		*tlist = i;
		NextNode();
		if (pcur->tkcod == TK_CLOSEPA)
		{
			return;
		}
		else if (pcur->tkcod == TK_COMMA) {
			NextNode();
			call_list(tlist + 1);
		}
		else {
			GOERROR(CALLERROR)
		}
	}else if (pcur->tkcod >=TK_CINT&&pcur->tkcod <=TK_CCHAR)
	{
		*tlist = pcur->tkcod;
		NextNode();
		if (pcur->tkcod == TK_CLOSEPA)
		{
			return;
		}
		else if (pcur->tkcod == TK_COMMA) {
			NextNode();
			call_list(tlist + 1);
		}
		else {
			GOERROR(CALLERROR)
		}
	}
	else if (pcur->tkcod == TK_FUNC)
	{
		int j = Func_use();
		if (j == KW_VOID)
		{
			GOERROR(FUNC_TYPE)
		}
		*tlist = trans_to_ctype(j);//函数嵌套调用
		NextNode();
		call_list(tlist + 1);
	}
	else {
		GOERROR(CALLERROR)
	}
}

void If_Process() {
	push2(&stelse,KW_IF);
	NextNode();
	if (pcur->tkcod != TK_OPENPA) {
		GOERROR(IF_LACK_OPENPA)
	}
	NextNode();
	Primary();
	if (pcur->tkcod != TK_CLOSEPA) {
		GOERROR(IF_LACK_CLOSEPA)
	}
	NextNode();
	if (pcur->tkcod == KW_ELSE) {
		GOERROR(LACK_ST)
	}
	else if (pcur->tkcod == TK_END) {
		GOERROR(LACK_ST)
	}
	else if (pcur->tkcod == TK_SEMICOLON) {
		NextNode();
	}
	else if (pcur->tkcod != TK_BEGIN)
	{
		one_line();
	}
	else if (pcur->tkcod == TK_BEGIN) {
		push1(&st, pcur->sp, 0);
		dep++;
		NextNode();
	}
	else return;
}

void Else_Process() {
	if (empty2(&stelse)) {
		GOERROR(ELSE_ERROR)
	}
	pop2(&stelse);
	NextNode();

	if (pcur->tkcod == TK_END) {
		GOERROR(LACK_ST)
	}
	else if (pcur->tkcod == TK_SEMICOLON) {
		NextNode();
	}
	else if (pcur->tkcod == KW_IF) {
		If_Process();
	}
	else if(pcur->tkcod != TK_BEGIN)
	{
		one_line();
	}
	else if (pcur->tkcod == TK_BEGIN) {
		push1(&st, pcur->sp, 0);
		dep++;
		NextNode();
	}
}

void For_Process() {
	dep++;//for开始深度加+1，遇大括号再加1，for语句块结束深度-2;
	int count = 0;
	NextNode();
	if (pcur->tkcod != TK_OPENPA) {
		GOERROR(FOR_LACK_OPENPA)
	}
	NextNode();	//第一个表达式，可能为定义，也可能为赋值
	if (pcur->tkcod == TK_SEMICOLON) {
		count++;
		NextNode();
	}
	else {
		int i = pcur->tkcod;
		if (IS_TYPE(i)) {
			NextNode();
			Variate(i);
		}
		else {
			A_Process(0);
		}
		if (pcur->tkcod != TK_SEMICOLON) {
			GOERROR(FOR_ERROR)
		}
		count++;
	}	//第二个表达式
	if ((pcur+1)->tkcod == TK_SEMICOLON) {
		count++;
		NextNode();
	}
	else {
		NextNode();
		is_bool();
		if (pcur->tkcod != TK_SEMICOLON) {
			GOERROR(FOR_ERROR)
		}
		count++;
		NextNode();
	}//第三个表达式
	if (pcur->tkcod == TK_CLOSEPA) {
		if (count != 2) {
			GOERROR(FOR_ERROR)
		}
		NextNode();
	}
	else {
		A_Process(1);
		if (pcur->tkcod != TK_CLOSEPA) {
			GOERROR(FOR_LACK_CLOSEPA)
		}
		if (count != 2) {
			GOERROR(FOR_ERROR)
		}
		NextNode();
	}// for(;;)已处理完毕 pcur为下一个字符
	if (pcur->tkcod == TK_SEMICOLON) {
		NextNode();
		return;
	}
	else if (pcur->tkcod != TK_BEGIN) {//单行语句，同第一个分号前，增加bcr，独立深度
		if (pcur->tkcod == KW_CONTINUE || pcur->tkcod == KW_BREAK)
		{
			NextNode();
			if (pcur->tkcod != TK_SEMICOLON) {
				GOERROR(LACK_SEMICOLON)
			}
			dep--;
			NextNode();
			return;

		}
		one_line();
		dep--;//for退两层
	}
	else if (pcur->tkcod == TK_BEGIN) {
		push1(&st, pcur->sp, 1);
		dep++;
		if(dep<for_dep)//记录最浅深度
		{
			for_dep = dep; 
		}
		NextNode();
	}
	else return;
}
//if、else、for后的无大括号单行语句分析
void one_line()
{
	if (pcur->tkcod == KW_CONTINUE || pcur->tkcod == KW_BREAK)
	{
		if (dep < for_dep) {
			GOERROR(NO_CYCLE)
		}
			NextNode();
			if (pcur->tkcod != TK_SEMICOLON) {
				GOERROR(LACK_SEMICOLON)
			}
			NextNode();
			return;
	}
	dep++;
	if (pcur->tkcod == KW_RETURN)
	{
		NextNode();
		Return_Process();
		dep--;
		return;
	}
	int i = pcur->tkcod;
	if (IS_TYPE(i)) {
		NextNode();
		Variate(i);
	}
	else if(i==TK_VAR){
		A_Process(0);
	}
	else if (i == TK_FUNC)
	{
		Func_use();
		NextNode();
	}
	else
	{
		GOERROR(LACK_BEGIN)
	}
	if (pcur->tkcod != TK_SEMICOLON) {
		GOERROR(LACK_SEMICOLON)
	}
	NextNode();
	dep--;
}
void While_Process() {
	NextNode();
	if (pcur->tkcod != TK_OPENPA) {
		GOERROR(WHILE_LACK_OPENPA)
	}
	NextNode();
	if (pcur->tkcod == TK_CLOSEPA) {
		GOERROR(WHILE_ERROR)
	}
	is_bool();
	if (pcur->tkcod != TK_CLOSEPA) {
		GOERROR(WHILE_ERROR)
	}
	NextNode();
	if (pcur->tkcod == KW_CONTINUE) {
		NextNode();
		if (pcur->tkcod != TK_SEMICOLON) {
			GOERROR(CONTINUE_LACK_SEMICOLON)
		}
		NextNode();
		return;
	}
	else if (pcur->tkcod == KW_BREAK) {
		NextNode();
		if (pcur->tkcod != TK_SEMICOLON) {
			GOERROR(BREAK_LACK_SEMICOLON)
		}
		NextNode();
		return;
	}
	else if (pcur->tkcod == TK_END) {
		GOERROR(LACK_ST)
	}
	else if (pcur->tkcod == TK_SEMICOLON) {
		NextNode();
	}
	else if (pcur->tkcod == TK_BEGIN) {
		push1(&st, pcur->sp, 0);
		dep++;
		if (dep < for_dep)//记录最浅深度
		{
			for_dep = dep;
		}
		NextNode();
	}
	else return;
}
void Return_Process() {
	if (FT == KW_VOID) {//当前函数返回值为void
		if (pcur->tkcod == TK_SEMICOLON) {
			NextNode();
		}
		else {
			GOERROR(RETURN_ERROR)
		}
	}
	else {
		if (Na_Variate(KW_INT) != FT)
		{
			GOERROR(RETURN_ERROR)
		}
		NextNode();
	}
}
//赋值语句入口 默认0，如果是1进入for第三段
void A_Process(int sta)
{
	if(pcur->tkcod!=TK_VAR)
	{
		GOERROR(LEFTERROR)
	}
	int i=search_vhash(pcur->sp);
	if (0 == i)
	{
		GOERROR(NOT_DEFINE)
	}
		Assign_Variate(i,sta);

}
//非赋值表达式分析,不能有等号，需要给出返回类型，用于函数实参、返回语句
int Na_Variate(int tk) {
	if (pcur->tkcod == TK_SEMICOLON)
	{
		return tk;
	}
	else if (pcur->tkcod == TK_COMMA) {//如果是逗号
		NextNode();
		Na_Variate(tk);
	}
	else if ((pcur->tkcod >= TK_CINT && pcur->tkcod <= TK_CCHAR) || pcur->tkcod == TK_OPENPA || pcur->tkcod == TK_FUNC) {//表达式分析，读下一个继续验证
		pWord pre = pcur;//开始前记录pre，后面要从头开始表达式分析
		while (pcur->tkcod != TK_SEMICOLON && pcur->tkcod != TK_COMMA)//遍历到逗号或分号，出现小数就报错
		{
			if (pcur->tkcod == TK_CFLOAT)
			{
				tk = KW_FLOAT;
				break;
			}
			if (pcur->tkcod == TK_ASSIGN)
			{
				GOERROR(WRONGASSIGNED)
			}
			if (pcur->tkcod == TK_FUNC)
			{
				int i = Func_use();//此时已经走到函数末尾
				if (i == KW_VOID)//不考虑指针
				{
					GOERROR(FUNC_TYPE)
				}
				if (i == KW_FLOAT)
				{
					tk = KW_FLOAT;
					break;
				}
			}
			pcur++;
		}
		pcur = pre;
		Primary();
		Na_Variate(tk);
	}
	else {
		GOERROR(LACK_SEMICOLON)
	}
}
void is_bool()
{
	Primary();
	if (pcur->tkcod == TK_COMMA)
	{
		is_bool();
	}
	return;
}
//初等表达式分析,直接用于布尔(，继续），间接用于验证过的其他表达式
void Primary() {
	pend = 0;
	E();//这个正是我们前面给大家详细讲过的文法分析
	if (pend == 1 && err == 0)//pend为1，代表是分号，逗号，或者右括号
		return;
	else {
		GOERROR(LACK_PRIMARY)
	}
}

void Next() {
	pcur++;
	if (strcmp(pcur->sp , ";")==0 || strcmp(pcur->sp, ",")==0 || strcmp(pcur->sp ,")")==0)
	{
		pend = 1;
	}
}

void E() {
	AO();
	if (pcur->tkcod == TK_ASSIGN) {//TODO:赋值运算先行排除
		Next();
		AO();
	}
}
void AO() {
	RE();
	if (pcur->tkcod == TK_ANDAND|| pcur->tkcod == TK_OR) {
		Next();
		RE();
	}
}	

void RE() {
	AD();
	if (pcur->tkcod >= TK_EQ && pcur->tkcod <= TK_GEQ) {
		Next();
		AD();
	}
}

void AD() {
	MU();
	if (pcur->tkcod == TK_PLUS || pcur->tkcod == TK_MINUS) {
		Next();
		MU();
	}
}

void MU() {
	UN();
	if (pcur->tkcod == TK_STAR || pcur->tkcod == TK_DIVIDE) {
		Next();
		UN();
	}
}
void UN() {
	switch (pcur->tkcod) 
	{ 
	case TK_AND:
	case TK_NOT:
	case TK_STAR:
	case TK_PLUS: 
	case TK_MINUS: Next(); UN(); break;
	case KW_SIZEOF:Next(); SE(); break;
	default: PR(); break; 
	}
}

void SE()
{ 
	if (pcur->tkcod == TK_OPENPA)
	{
		Next();
	}
	if (IS_TYPE(pcur->tkcod)|| pcur->tkcod == TK_VAR)
	{
		Next();
	}
	if (pcur->tkcod == TK_CLOSEPA) {
		Next();
		return;
	}
	else {
		GOERROR(LACK_PRIMARY)
	}
}

void PR() {
		//TODO:函数调用验证
	if (pcur->tkcod >= TK_CINT&&pcur->tkcod<=TK_CCHAR||pcur->tkcod==TK_VAR || pcur->tkcod == TK_FUNC) {
		if (pcur->tkcod == TK_VAR)
		{
			int i = search_vhash(pcur->sp);
			if (0 == i)
			{
				GOERROR(NOT_DEFINE)
			}
		}
		if (pcur->tkcod == TK_FUNC)
		{
			int i=Func_use();//仅做调用判断
		}
		Next();
	}
	else if (pcur->tkcod == TK_OPENPA) {
		Next();
		E();
		if (pcur->tkcod == TK_CLOSEPA) {
			Next();
		}
		else {
			err = -1;
		}
	}
	else if (pcur->tkcod == TK_OPENBR) {
		Next();
		E();
		if (pcur->tkcod == TK_CLOSEBR) {
			Next();
		}
		else {
			err = -1;
		}
	}
	else {
		err = -1;
		GOERROR(LACK_PRIMARY)
	}
}
