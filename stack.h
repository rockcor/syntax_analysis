
/*----------------------------------表达式栈---------------------*/
typedef struct tag {
	char* sp;
	struct tag* next;
} Node, *pNode;

typedef struct {
	pNode phead;
	int size;
}Stack, * pStack;

void init_stack(pStack s);
void pop(pStack s);
void push(pStack s, char* val);
char *top(pStack s);
int empty(pStack s);
int stack_size(pStack s);

/*---------------------------括号栈------------------------------*/
typedef struct tag1 {
	char* sp;
	int type;//1表示是for后大括号
	struct tag1* next;
} KNode, *pKNode;

typedef struct {
	pKNode phead;
	int size;
}KStack, * pKStack;

void init_stack1(pKStack s);
pKNode pop1(pKStack s);//返回结点指针
void push1(pKStack s, char* val,int type);
int empty1(pKStack s);
int stack_size1(pKStack s);

/*-----------------------if else和重名变量栈群----------------------*/

typedef struct tag2 {
	int m_val;
	struct tag2* next;
} VNode, * pVNode;

typedef struct {
	pVNode phead;
	int size;
}VStack, * pVStack;

typedef struct Tk1 { //变量哈希表专用结构体
	int tkcode;	//token
	char sp[SIZE];	//字符串
	int depth;		//深度，判断同层重定义和何时出栈
	VStack s;		//变量栈结构体，不用malloc
	struct Tk1* next;
}Svar, * pSvar;

typedef struct Tk5 { //TODO:深度结构体,记录每一层定义了哪些变量，退出该层时，找到这些变量的栈入口弹出类型
	int tkcode;	//token
	char sp[SIZE];	//字符串
	int depth;		//深度，判断同层重定义和何时出栈
	Svar s;		//变量栈结构体，不用malloc
	struct Tk5* next;
}DStack, * pDStar;

void init_stack2(pVStack s);
void pop2(pVStack s);
void push2(pVStack s, int val);
int top2(pVStack s);
int empty2(pVStack s);
int stack_size2(pVStack s);

