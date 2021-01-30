#include "func.h"
extern jmp_buf jmpbuf;
//��ÿһ����֪���ŷ����ϣ��Ľṹ���в���ϣ��ʼ��
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

	{TK_CINT,"���γ���",NULL},
	{TK_CFLOAT,"�����ͳ���",NULL},
	{TK_CCHAR,"�ַ�����",NULL},
	{TK_CSTR,"�ַ�������",NULL},

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
pTkWord hashtable[MAXKEY] = { NULL };//�ʷ���token��ϣ��
pSfunc hash_func[MAXKEY] = { NULL };//��������ϣ��
words wordlist[MAXLEN] = { 0 };//ȫ�ִ洢��
pWord pcur;
int dep,for_dep;
int wpos = 0;//ȫ�ִ洢���±�
int JmpJud;
int row_num;//ȫ������
int flag = 1;//����ע����
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
	}//��ʼ����ϣ��
	while (fgets(buf, sizeof(buf), fp) != NULL)
	{
		row_num++;
		line_analyse(buf,fp);
	}
	fclose(fp);
	/*-----------------------------------------�﷨���������------------------------------------*/
	char *res;
	JmpJud = setjmp(jmpbuf);
	set_white();
	switch (JmpJud)
	{
	case SOMETHINGWRONG:res = "����ʧ�ܣ�δ֪����"; break;
	case NOT_DECLARE:res = "����ʧ�ܣ�����δ������"; break;
	case CALLERROR:res = "����ʧ�ܣ��������ô���"; break;
	case NOT_DEFINE:res = "����ʧ�ܣ���������δ���壡"; break;
	case REDEFINE:res = "����ʧ�ܣ����������ض��壡"; break;
	case NO_CYCLE:res = "����ʧ�ܣ�����ѭ����򿪹�����У�"; break;
	case FUNC_TYPE:res = "����ʧ��,���������򷵻�ֵ���ʹ���"; break;
	case MALLOCFAIL:res = "�ѿռ����ʧ�ܣ�"; break;
	case LACK_SEMICOLON:res = "����ʧ�ܣ�ȱ��';'��"; break;
	case EXTER_DEC:res = "����ʧ�ܣ��ⲿ������������"; break;
	case LEFTERROR:res = "����ʧ�ܣ���ֵ�����ֵ����"; break;
	case LACK_TYPE:res = "����ʧ�ܣ�ȱ�����ͷ���"; break;
	case WRONGASSIGNED:res = "����ʧ�ܣ��Ǹ�ֵ���ʽ��ʹ�õȺţ�"; break;
	case VRA_WRONG:res = "����ʧ�ܣ���������"; break;
	case FUNC_WRONG:res = "����ʧ�ܣ����������������"; break;
	case LACK_IDENT:res = "����ʧ�ܣ�ȱ�ٱ�ʶ����"; break;
	case LACK_OPENPA:res = "����ʧ�ܣ�ȱ��'('��"; break;
	case LACK_CLOSEPA:res = "����ʧ�ܣ�ȱ��' '��"; break;
	case LACK_OPENBR:res = "����ʧ�ܣ�ȱ��'['��"; break;
	case LACK_CLOSEBR:res = "����ʧ�ܣ�ȱ��']'��"; break;
	case LACK_BEGIN:res = "����ʧ�ܣ�ȱ��'{'��"; break;
	case LACK_END:res = "����ʧ�ܣ�ȱ��'}'��"; break;
	case NESTED_FUNC_DECLEAR:res = "����ʧ�ܣ�����Ƕ�׶��壡"; break;
	case LACK_PRIMARY:res = "����ʧ�ܣ����ȱ��ʽ����"; break;
	case RETURN_ERROR:res = "����ʧ�ܣ�return���ʹ���"; break;
	case BREAK_LACK_SEMICOLON:res = "����ʧ�ܣ�breakȱ��';'��"; break;
	case CONTINUE_LACK_SEMICOLON:res = "����ʧ�ܣ�continueȱ��';'��"; break;
	case IF_LACK_OPENPA:res = "����ʧ�ܣ�if��ȱ��'('��"; break;
	case IF_LACK_CLOSEPA:res = "����ʧ�ܣ�if��ȱ��')'!"; break;
	case FOR_LACK_OPENPA:res = "����ʧ�ܣ�forȱ��'('��"; break;
	case FOR_LACK_CLOSEPA:res = "����ʧ�ܣ�forȱ��')'��"; break;
	case LACK_ST:res = "����ʧ�ܣ�ȱ����䣡"; break;
	case CONTINUE_ERROR:res = "����ʧ�ܣ�continueʹ�ô���"; break;
	case ELSE_ERROR:res = "����ʧ�ܣ�elseʹ�ô���"; break;
	case FOR_ERROR:res = "����ʧ�ܣ�forʹ�ô���"; break;
	case WHILE_LACK_OPENPA:res = "����ʧ�ܣ�whileȱ����Բ���ţ�"; break;
	case WHILE_LACK_CLOSEPA:res = "����ʧ�ܣ�whileȱ����Բ���ţ�"; break;
	case WHILE_ERROR:res = "����ʧ�ܣ�whileʹ�ô���"; break;
	case PROGRAM_BEGIN:Compile();
	default:
		res = "\n================�ɹ� 1 ����ʧ�� 0 �������� 0 ��==============\n";
		break;
	}
	if (SUCC == JmpJud) {
		printf("\n\n================�������Ŀcode.c�ı��빤��===================");
		printf("\n%s", res);
		return 0;
	}
	else {
		printf("\n\n============%s============\n", res);
		printf("\n============== ����������%d ===============\n", row_num);

	}
	return 0;
}