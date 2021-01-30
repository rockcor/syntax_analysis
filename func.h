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
	/* ��������ָ��� */
	TK_PLUS,		// + �Ӻ�
	TK_MINUS,		// - ����
	TK_STAR,		// * �Ǻ�
	TK_DIVIDE,		// / ����
	TK_MOD,			// % ���������
	TK_EQ,			// == ���ں�
	TK_NEQ,			// != �����ں�
	TK_LT,			// < С�ں�
	TK_LEQ,			// <= С�ڵ��ں�
	TK_GT,			// > ���ں�
	TK_GEQ,			// >= ���ڵ��ں�
	TK_ASSIGN,		// = ��ֵ����� 
	TK_POINTSTO,	// -> ָ��ṹ���Ա�����
	TK_DOT,			// . �ṹ���Ա�����
	TK_AND,         // & ��ַ�������
	TK_OPENPA,		// ( ��Բ����
	TK_CLOSEPA,		// ) ��Բ����
	TK_OPENBR,		// [ ��������
	TK_CLOSEBR,		// ] ��������
	TK_BEGIN,		// { �������
	TK_END,			// } �Ҵ�����
	TK_FOR_END,		// } �Ҵ�����
	TK_SEMICOLON,	// ; �ֺ�    
	TK_COMMA,		// , ����
	TK_ELLIPSIS,	// ... ʡ�Ժ�
	TK_ANDAND,		//&& �߼���
	TK_OR,			//|| �߼���
	TK_NOT,			//! �߼���
	TK_EOF,			// �ļ�������

	/* ���� */
	TK_CINT,		// ���ͳ���
	TK_CFLOAT,		// �����ͳ���
	TK_CCHAR,		// �ַ�����
	TK_CSTR,		// �ַ�������

	/* �ؼ��� */
	KW_INT,			// int�ؼ���
	KW_SHORT,		// short�ؼ���
	KW_FLOAT,		// float�ؼ���
	KW_CHAR,		// char�ؼ���
	KW_VOID,		// void�ؼ���  
	KW_STRUCT,		// struct�ؼ���   
	KW_IF,			// if�ؼ���
	KW_ELSE,		// else�ؼ���
	KW_FOR,			// for�ؼ���
	KW_WHILE,		// while�ؼ���
	KW_CONTINUE,	// continue�ؼ���
	KW_BREAK,		// break�ؼ���   
	KW_RETURN,		// return�ؼ���
	KW_SIZEOF,		// sizeof�ؼ���(��ʵ���������

	/* ��ʶ�� ,�Լ�����ı����ͺ���*/
	TK_VAR,
	TK_FUNC
};
enum Error_type {
	PROGRAM_BEGIN,		//ִ���﷨����
	SUCC,				//�ɹ�
	SOMETHINGWRONG,		//δ֪����
	LACK_SEMICOLON,		//ȱ�ٷֺ�
	RETURN_ERROR,		//return���ʹ���
	BREAK_LACK_SEMICOLON,		//breakȱ�ٷֺ�
	CONTINUE_LACK_SEMICOLON,	//continueȱ�ٷֺ�
	WRONGASSIGNED,		//�Ǹ�ֵ���ʽ��ʹ�õȺ�
	NO_CYCLE,			//����ѭ������
	EXTER_DEC,			//�ⲿ���������ִ���
	LACK_TYPE,			//ȱ������
	VRA_WRONG,			//��������
	FUNC_WRONG,			//�����������������
	CALLERROR,			//�������ô���
	REDEFINE,			//���������ض���
	NOT_DEFINE,			//δ����ʹ��
	NOT_DECLARE,        //����δ����
	FUNC_TYPE,			//���������򷵻�ֵ���ʹ���
	LEFTERROR,			//��ֵ�����ֵ����
	LACK_IDENT,			//ȱ�ٱ�ʶ��
	LACK_OPENPA,		//ȱ����Բ����
	LACK_CLOSEPA,		//ȱ����Բ����
	LACK_OPENBR,		//ȱ����������
	LACK_CLOSEBR,		//ȱ����������
	LACK_BEGIN,			//ȱ��������
	LACK_END,			//ȱ���һ�����
	NESTED_FUNC_DECLEAR,//����Ƕ�׶���
	IF_LACK_OPENPA,		//ifȱ����Բ����
	IF_LACK_CLOSEPA,	//ifȱ����Բ����
	FOR_LACK_OPENPA,	//forȱ����Բ����
	FOR_LACK_CLOSEPA,	//forȱ����Բ����
	LACK_PRIMARY,		//���ȱ��ʽ����
	LACK_ST,			//ȱ�����
	CONTINUE_ERROR,		//continueʹ�ô���
	ELSE_ERROR,			//elseʹ�ô���
	FOR_ERROR,			//forʹ�ô���
	WHILE_LACK_OPENPA,	//whileȱ����Բ����
	WHILE_LACK_CLOSEPA,	//whileȱ����Բ����
	WHILE_ERROR,		//whileʹ�ô���
	MALLOCFAIL,			//�ѿռ����ʧ��
	
};
typedef enum e_TokenCode tokencode;
typedef struct Tk { //��token��ϣ��ר�ýṹ��
	int tkcode;	//token
	char spe[100];	//�ַ���
	struct Tk* next;
}TkWord, * pTkWord;

typedef struct Tk2 { //������ϣ��ר�ýṹ��
	char sp[SIZE];	//�ַ���
	int dec;		//��ǣ�Ϊ0����������δ���壬1�����Ѷ���
	int tkr;		//��������token
	int tk[SIZE];	//����token
	struct Tk2* next;
}Sfunc, * pSfunc;

typedef struct { //�洢ר�ýṹ��
	int tkcod;	//token
	char sp[100];	//�ַ���
	int row;
}words, *pWord;
/*------------------------�ʷ�������������----------------------------*/
int elf_hash(char key[]);
void estab_hash(char str[], int tkco);
void search_hash_print(char str[]);
void estab_id_hash(char str[],int tkco);
void int_or_float(char*);
void line_analyse(char* c,FILE *fp);
void Print(char str[], tokencode token);
void set_white();

/*------------------------�﷨������������----------------------------*/
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
