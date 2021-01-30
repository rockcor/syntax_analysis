#include "func.h"
#include "stack.h"
/*--------------------------------------ȫ�ֱ�����------------------------------------------------*/
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
extern int dep;//TODO:��ջ���ò���
extern int for_dep;//����¼��ǳѭ����
Sfunc kong;
VStack stelse;//if elseջ
KStack st;//����ջ��С���Ų�ѹ,������ûѹ
Stack expre;//���ʽ����ջ����δ�õ�
//����ջȺ�ں����н���������
/*----------------------------------------�ʷ�����------------------------------------------------------*/
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
	if (token >= TK_VAR) {//�����������ߺ�����Ϊ��ɫ
		SetConsoleTextAttribute(h, FOREGROUND_INTENSITY);
	}
	else if (token >= KW_INT) {//�ؼ���Ϊ��ɫ
		SetConsoleTextAttribute(h, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
	}
	else if (token >= TK_CINT) {//���ͳ����������ͳ������ַ��������ַ���������Ϊ��ɫ
		SetConsoleTextAttribute(h, FOREGROUND_RED | FOREGROUND_GREEN);
	}
	else {//�����Ϊ��ɫ
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
		if (c[i] == '/' && c[i + 1] == '*') //����ע�ͣ�ֱ�Ӵ򣬲��洢
		{
			flag = 0;
			goto notes; //���Բ���goto�����������жϴ���
		}
		if (c[i] == ' ' || c[i] == '\n' || c[i] == '\t') {
			printf("%c", c[i++]);//��������ɫ��ֱ�Ӵ�ӡ��Ҳ��������ڱ�ʶ����
		}
		else if (c[i] == '/' && c[i + 1] == '/') { //����\0�Ĵ��ڣ�����Խ��
			set_white();
			while (c[i]) {	//ֱ�Ӵ���һ�У����洢
				printf("%c", c[i++]);
			}
			return;
		}
		
		else if (isalpha(c[i]) || c[i] == '_') { //�������ĸ,�����ǹؼ��֣�Ҳ�����Ǳ�ʶ��,ע�ⲻ�����ֿ�ͷ
			while (isalnum(c[i]) || c[i] == '_') { //��������ĩβ,��s��¼
				s[j] = c[i];
				j++;
				i++;
			}
			int k = i;
			while (c[k] == ' ') {		//���������ո�
				k++;
			}
			if (c[k] == '(')
			{
				estab_id_hash(s, TK_FUNC);//��ϣ���Ҳ����ֹؼ��ֺͱ�ʶ��(�������������ҲҪ���֣�
			}
			else
			{
				estab_id_hash(s, TK_VAR);//����ǹؼ��ֲ���ʹ�ô����token
			}
			CLEAR
		}
		else if (isdigit(c[i])||(c[i]=='.'&&isdigit(c[i+1]))) {//��������ֻ�㿪ͷ
			while (isdigit(c[i])||c[i]=='.'){
				s[j] = c[i];
				i++;
				j++;
			}
			int_or_float(s);//�������κ͸���
			CLEAR
		}
		else if (c[i] == '\'') {//�ַ���������֧��Ƕ�׶���
			s[j] = c[i++];
			j++;
			while (c[i] != '\'') {//�����ַ��������� todo
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
		else if (c[i] == '\"') {//�ַ�������
			s[j] = c[i++];
			j++;
			while (c[i] != '\"') {
				s[j] = c[i];
				i++;
				j++;
			}
			s[j] = c[i++]; //���һ��˫����
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
			else { s[0] = c[i++]; } //�������
			search_hash_print(s);
			CLEAR
		}
	}
}

/*----------------------------------------�﷨����-----------------------------------------------------*/
//�﷨�������
//ƾ�ո�һ��main����������,Ҫ��ΪINT�ͣ��޲�
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
//����ת�������ͣ���¼�ڱ�����ϣ��,���������ж�
//��������ת���������������Ľṹ���в���
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
	if (kw == KW_SHORT)//��ʱshort��int������
	{
		return TK_CINT;
	}
	if (kw == KW_CHAR)
	{
		return TK_CCHAR;
	}
	GOERROR(SOMETHINGWRONG);//�˴�Ӧ���׳�һ������
}
//��������ϣ�����ڽ������ض�����
void var_estab_hash(char str[], int tkco)
{
	int tknew=trans_to_ctype(tkco);//�ڱ�����ϣ���д�Ϊ��������
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
				if (dep == pcu->depth)//ͬ���ض���
				{
					GOERROR(REDEFINE)
				}
				return;
				//TODO:��ͬ���ض��壬token�����ѹջ�����˳��������ڵ�ǰ���ʱ����ջ

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
//��������ϣ,���ڽ�����ʵ����֤���ض�����֤,�����������Ӧ0��1����״̬
void func_estab_hash(char str[], int tkco,int tklist[],int sta)
{
	int k = elf_hash(str);
	pSfunc pcu = hash_func[k], pre = pcu;
	if (pcu == NULL) {
		if (sta == 1){ //����ʱδ�ҵ�
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
		if (strcmp(pcu->sp, str) == 0)//��������ͬ
		{
			if (memcmp(pcu->tk, tklist, SIZE * sizeof(int)) == 0 && pcu->tkr == tkco)//���κͷ���ֵ��ͬ
			{
				if (sta == 0)return;//��ͬ�������ظ�
				if (sta == 1)
				{
					if (pcu->dec == 1)//����ʱ�ҵ����Ѷ���
					{
						GOERROR(REDEFINE)
					}
					else  //����ʱ�ҵ���δ����
					{
						pcu->dec = 1;
						return;
					}
						
				}
			}
			GOERROR(FUNC_TYPE)//�����׳�����ȷ�Ĵ���
		}
		pre = pcu;
		pcu = pcu->next;
	}
	if (sta == 1) { //����ʱδ�ҵ�
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
//���������ϣ��û�ҵ������ҵ�����ת�����token
int search_vhash(char str[])
{
	int k = elf_hash(str);
	pSvar pcu = hash_var[k], pre = pcu;
	while (pcu) {
		if (strcmp(pcu->sp, str) == 0 && pcu->depth <= dep) {
			if (dep < pcu->depth)//δ���壻δ��ȷ�������ͣ�������
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
//���麯����ϣ������0��ʾû�ҵ����ҵ��󷵻��䷵��ֵ��token
//TODO:���ܲ���Ҫ����Ϊ���ñ���Ҫ��֤����func_estab_hash
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
//�ⲿ����
void External_dec() {   //�˳��ⲿ����ʱ pcurΪ��ǰ������;������} 
	if (pcur->row == 0) {//�ж��Ƿ��˽�β
		longjmp(jmpbuf, SUCC);
		return;
	}
	else if (pcur->tkcod == TK_SEMICOLON) {//�ж��Ƿ�Ϊ�ֺ�
		//��ת���ⲿ�������ִ���
		GOERROR(EXTER_DEC)
	}
	else if (pcur->tkcod == TK_END) {//�Ƿ����Ҵ�����
		GOERROR(LACK_BEGIN)

	}
	else if (!IS_TYPE(pcur->tkcod)) {//�ж��������һ��int��float������������
		GOERROR(LACK_TYPE)
	}
	else {
		type_Statement();//���������������Ĵ����ж��Ƿ�Ϊ��������
	}
	NextNode();
	External_dec();
}
//�ⲿ����Ϊ��������  ������pcurΪ�ֺ��˳�; �������ֺ��˳�Ϊ������}�˳�Ϊ����
void type_Statement() {
	if ((pcur + 1)->tkcod == TK_VAR) {//��һ����Ա�Ƿ�Ϊ����
		int i = pcur->tkcod;
		NextNode();//pcurָ��ñ���
		Variate(i);
	}
	else if ((pcur + 1)->tkcod == TK_FUNC) {
		Func();//��������������ʵ�ֶ���������ȥ
	}
	else {
		GOERROR(EXTER_DEC)
	}
}
//ȫ�ֱ�������pcurָ�������
//ע�⺯���ڱ����������ֵ����Ϊ����
//ע��������ͺ��Ƿ���
void Variate(int i) {
	if (pcur->tkcod == TK_VAR)
	{
		var_estab_hash(pcur->sp, i);//��������ϣ,ѹջ
	}
	if ((pcur+1)->tkcod == TK_SEMICOLON)
	{
		NextNode();
		return;
	}
	else if ((pcur + 1)->tkcod == TK_COMMA) {//�����һ���Ƕ���
		NextNode();
		if ((pcur+1)->tkcod != TK_VAR) {
			GOERROR(VRA_WRONG)
		}
		else {
			NextNode();
			Variate(i);
		}
	}
	else if (((pcur + 1)->tkcod) == TK_ASSIGN) {//�����һ���ǵȺ�
		NextNode();
		NextNode();
		switch (i) {
		case KW_SHORT://�����������жϱ�׼��ͬ
		case KW_CHAR:
		case KW_INT:
			if (pcur->tkcod == TK_CINT || pcur->tkcod == TK_CCHAR || pcur->tkcod == TK_VAR || pcur->tkcod == TK_OPENPA||pcur->tkcod == TK_FUNC) {//�����ͳ��������ʽ����������һ��������֤
				pWord pre = pcur;//���ʽ��ʼǰ���˵�pre
				while (pcur->tkcod != TK_SEMICOLON && pcur->tkcod != TK_COMMA)//���������Ż�ֺţ�����С���ͱ���
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
						int i = Func_use();//��ʱ�Ѿ��ߵ�����ĩβ
						if (i == KW_VOID)//������ָ��
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
			if (pcur->tkcod >= TK_CINT && pcur->tkcod <= TK_CCHAR || pcur->tkcod == TK_VAR || pcur->tkcod == TK_OPENPA || pcur->tkcod == TK_FUNC) {//���ʽ����������һ��������֤,Ҫ��С��
				pWord pre = pcur;//���ʽ��ʼǰ���˵�pre
				while (pcur->tkcod != TK_SEMICOLON && pcur->tkcod != TK_COMMA)//���������Ż�ֺţ�������С���ͱ���
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
						int i = Func_use();//��ʱ�Ѿ��ߵ�����ĩβ
						if (i == KW_VOID)//������ָ��
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
//���������ƣ����������������ش����ͣ�����ֻ��ѯ��ϣ������
//�������Ҳ����û�еȺţ���дһ������
//�������Ҳ���Գ����ڲ������ʽ��
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
	if ((pcur + 1)->tkcod == TK_COMMA) {//����Ƕ���
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
	else if (((pcur+1)->tkcod) == TK_ASSIGN) {//����ǵȺţ�����һ��
		NextNode();
		NextNode();
		switch (i) {
		case TK_CCHAR:
		case TK_CINT:
			if (pcur->tkcod == TK_CINT || pcur->tkcod == TK_CCHAR || pcur->tkcod == TK_VAR || pcur->tkcod == TK_OPENPA || pcur->tkcod == TK_FUNC) {//�����ͳ��������ʽ����������һ��������֤
				pWord pre = pcur;//���ʽ��ʼǰ���˵�pre
				while (pcur->tkcod != TK_SEMICOLON && pcur->tkcod != TK_COMMA&& pcur->tkcod != TK_CLOSEPA)//���������Ż�ֺŻ������ţ�����С���ͱ���
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
						int i = Func_use();//��ʱ�Ѿ��ߵ�����ĩβ�������������Ų�����Ϊ��������
						if (i == KW_VOID)//������ָ��
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
			if (pcur->tkcod >= TK_CINT && pcur->tkcod <= TK_CCHAR || pcur->tkcod == TK_VAR || pcur->tkcod == TK_OPENPA || pcur->tkcod == TK_FUNC) {//���ʽ����������һ��������֤,Ҫ��С��
				pWord pre = pcur;//���ʽ��ʼǰ���˵�pre
				while (pcur->tkcod != TK_SEMICOLON && pcur->tkcod != TK_COMMA && pcur->tkcod != TK_CLOSEPA)//���������Ż�ֺŻ������ţ�������С���ͱ���
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
						int i = Func_use();//��ʱ�Ѿ��ߵ�����ĩβ
						if (i == KW_VOID)//������ָ��
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

//�Ǻ�����  ��ʱpcurΪint��void�ȷ���ֵ����
void Func() {
	FT = pcur->tkcod;//��������
	NextNode();//����������
	char fname[100] = { 0 };
	strcpy(fname, pcur->sp);
	NextNode();//����һ�Σ�����������
	if (pcur->tkcod != TK_OPENPA) {
		GOERROR(LACK_OPENPA)
	}
	else {  //��������,��¼�β�token�б�
		NextNode();
		int k = 0;
		int tlist[SIZE] = { 0 };
		while ((pcur + k)->tkcod != TK_CLOSEPA)//������������
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
			for_dep = 1000;//��ʼ�������
			Func_Body();//�����崦��
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

//�����βα������� pcurΪ��һ��  ����)���� 0Ϊ����ʱ��1Ϊ����ʱ
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
//�����崦�� pcurΪ{    ���� }�˳�
void Func_Body() {
	NextNode();
	if (pcur->tkcod == TK_END) {//����Ϊ�� ֱ�ӷ���
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
				if ((pcur + 1)->tkcod == TK_VAR) {//��һ����Ա�Ƿ�Ϊ����
					int i = pcur->tkcod;
					NextNode();//pcurָ��ñ���
					Variate(i);
					NextNode();//��һ���ֺ�
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
				if (dep == for_dep)//�˳���ǳѭ���壬�������
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
			case TK_FUNC://��������
				Func_use();
				NextNode();
				if (pcur->tkcod != TK_SEMICOLON) {
					GOERROR(LACK_SEMICOLON)
				}
				NextNode();
				break;
			case KW_CONTINUE://������ѭ������
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
	pcur--;//���˵��Ҵ�����
	while (!empty2(&stelse))//�Ѷ����if����
		pop2(&stelse);
	//TODO:�������з�ȫ�ֱ���
}

//�������� ���ط�������token
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
		if (memcmp(pcu->tk, tklist, SIZE * sizeof(int)) != 0)//����ʵ����֤
		{
			GOERROR(CALLERROR)
		}
		return pcu->tkr;
	}
}
//�������ò����ռ�
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
		*tlist = trans_to_ctype(j);//����Ƕ�׵���
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
	dep++;//for��ʼ��ȼ�+1�����������ټ�1��for����������-2;
	int count = 0;
	NextNode();
	if (pcur->tkcod != TK_OPENPA) {
		GOERROR(FOR_LACK_OPENPA)
	}
	NextNode();	//��һ�����ʽ������Ϊ���壬Ҳ����Ϊ��ֵ
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
	}	//�ڶ������ʽ
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
	}//���������ʽ
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
	}// for(;;)�Ѵ������ pcurΪ��һ���ַ�
	if (pcur->tkcod == TK_SEMICOLON) {
		NextNode();
		return;
	}
	else if (pcur->tkcod != TK_BEGIN) {//������䣬ͬ��һ���ֺ�ǰ������bcr���������
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
		dep--;//for������
	}
	else if (pcur->tkcod == TK_BEGIN) {
		push1(&st, pcur->sp, 1);
		dep++;
		if(dep<for_dep)//��¼��ǳ���
		{
			for_dep = dep; 
		}
		NextNode();
	}
	else return;
}
//if��else��for����޴����ŵ���������
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
		if (dep < for_dep)//��¼��ǳ���
		{
			for_dep = dep;
		}
		NextNode();
	}
	else return;
}
void Return_Process() {
	if (FT == KW_VOID) {//��ǰ��������ֵΪvoid
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
//��ֵ������ Ĭ��0�������1����for������
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
//�Ǹ�ֵ���ʽ����,�����еȺţ���Ҫ�����������ͣ����ں���ʵ�Ρ��������
int Na_Variate(int tk) {
	if (pcur->tkcod == TK_SEMICOLON)
	{
		return tk;
	}
	else if (pcur->tkcod == TK_COMMA) {//����Ƕ���
		NextNode();
		Na_Variate(tk);
	}
	else if ((pcur->tkcod >= TK_CINT && pcur->tkcod <= TK_CCHAR) || pcur->tkcod == TK_OPENPA || pcur->tkcod == TK_FUNC) {//���ʽ����������һ��������֤
		pWord pre = pcur;//��ʼǰ��¼pre������Ҫ��ͷ��ʼ���ʽ����
		while (pcur->tkcod != TK_SEMICOLON && pcur->tkcod != TK_COMMA)//���������Ż�ֺţ�����С���ͱ���
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
				int i = Func_use();//��ʱ�Ѿ��ߵ�����ĩβ
				if (i == KW_VOID)//������ָ��
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
//���ȱ��ʽ����,ֱ�����ڲ���(�������������������֤�����������ʽ
void Primary() {
	pend = 0;
	E();//�����������ǰ��������ϸ�������ķ�����
	if (pend == 1 && err == 0)//pendΪ1�������Ƿֺţ����ţ�����������
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
	if (pcur->tkcod == TK_ASSIGN) {//TODO:��ֵ���������ų�
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
		//TODO:����������֤
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
			int i=Func_use();//���������ж�
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
