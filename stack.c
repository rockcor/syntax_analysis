#include "func.h"
#include "stack.h"
extern jmp_buf jmpbuf;
extern int row_num;
extern pWord pcur;

/*----------------------------------if else和表达式栈---------------------*/
void init_stack(pStack s)
{
	memset(s, 0, sizeof(Stack));
}
void push(pStack s, char* val)
{
	pNode pnew = (pNode)calloc(1, sizeof(Node));
	if (pnew == NULL) {
		longjmp(jmpbuf, MALLOCFAIL);
	}
	pnew->sp = val;
	if (!s->phead)
	{
		s->phead = pnew;
	}
	else {
		pnew->next = s->phead;
		s->phead = pnew;
	}
	s->size++;
}

char* top(pStack s)
{
	return s->phead->sp;
}

void pop(pStack s)
{
	pNode pcu = s->phead;
	if (!s->size)
	{
		printf("stack is empty\n");
		return;
	}
	s->phead = pcu->next;
	s->size--;
	free(pcu);
}

int empty(pStack s)
{
	return !s->size;
}

int stack_size(pStack s)
{
	return s->size;
}
/*---------------------------括号栈------------------------------*/
void init_stack1(pKStack s)
{
	memset(s, 0, sizeof(KStack));
}
void push1(pKStack s, char* val,int ty)
{
	pKNode pnew = (pKNode)calloc(1, sizeof(KStack));
	if (pnew == NULL) {
		longjmp(jmpbuf, MALLOCFAIL);
	}
	pnew->sp = val;
	pnew->type = ty;
	if (!s->phead)
	{
		s->phead = pnew;
	}
	else {
		pnew->next = s->phead;
		s->phead = pnew;
	}
	s->size++;
}

int top1(pKStack s)
{
	return s->phead->type;
}

pKNode pop1(pKStack s)
{
	pKNode pcu = s->phead;
	if (!s->size)
	{
		GOERROR(LACK_BEGIN) //TODO:应该区分中括号和大括号
	}
	s->phead = pcu->next;
	s->size--;
	return pcu;
	free(pcu);
}

int empty1(pKStack s)
{
	return !s->size;
}

int stack_size1(pKStack s)
{
	return s->size;
}

/*---------------------------重名变量栈群----------------------*/
void init_stack2(pVStack s)
{
	memset(s, 0, sizeof(VStack));
}
void push2(pVStack s, int val)
{
	pVNode pnew = (pVNode)calloc(1, sizeof(VStack));
	if (pnew == NULL) {
		longjmp(jmpbuf, MALLOCFAIL);
	}
	pnew->m_val = val;
	if (!s->phead)
	{
		s->phead = pnew;
	}
	else {
		pnew->next = s->phead;
		s->phead = pnew;
	}
	s->size++;
}

int top2(pVStack s)
{
	return s->phead->m_val;
}

void pop2(pVStack s)
{
	pVNode pcu = s->phead;
	if (!s->size)
	{
		printf("stack is empty\n");
		return;
	}
	s->phead = pcu->next;
	s->size--;
	free(pcu);
}

int empty2(pVStack s)
{
	return !s->size;
}

int stack_size2(pVStack s)
{
	return s->size;
}