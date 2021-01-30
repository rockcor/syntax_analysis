
/*----------------------------------���ʽջ---------------------*/
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

/*---------------------------����ջ------------------------------*/
typedef struct tag1 {
	char* sp;
	int type;//1��ʾ��for�������
	struct tag1* next;
} KNode, *pKNode;

typedef struct {
	pKNode phead;
	int size;
}KStack, * pKStack;

void init_stack1(pKStack s);
pKNode pop1(pKStack s);//���ؽ��ָ��
void push1(pKStack s, char* val,int type);
int empty1(pKStack s);
int stack_size1(pKStack s);

/*-----------------------if else����������ջȺ----------------------*/

typedef struct tag2 {
	int m_val;
	struct tag2* next;
} VNode, * pVNode;

typedef struct {
	pVNode phead;
	int size;
}VStack, * pVStack;

typedef struct Tk1 { //������ϣ��ר�ýṹ��
	int tkcode;	//token
	char sp[SIZE];	//�ַ���
	int depth;		//��ȣ��ж�ͬ���ض���ͺ�ʱ��ջ
	VStack s;		//����ջ�ṹ�壬����malloc
	struct Tk1* next;
}Svar, * pSvar;

typedef struct Tk5 { //TODO:��Ƚṹ��,��¼ÿһ�㶨������Щ�������˳��ò�ʱ���ҵ���Щ������ջ��ڵ�������
	int tkcode;	//token
	char sp[SIZE];	//�ַ���
	int depth;		//��ȣ��ж�ͬ���ض���ͺ�ʱ��ջ
	Svar s;		//����ջ�ṹ�壬����malloc
	struct Tk5* next;
}DStack, * pDStar;

void init_stack2(pVStack s);
void pop2(pVStack s);
void push2(pVStack s, int val);
int top2(pVStack s);
int empty2(pVStack s);
int stack_size2(pVStack s);

