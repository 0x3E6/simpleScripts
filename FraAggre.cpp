//2016.06.09
//�Ŵط�������ģ��
#include<stdio.h>
#include<math.h>
#include<time.h>
#include<stdlib.h>
#include<Windows.h>
//һЩ�ر������    ���г���֮ǰ��ע������ǰ4����
#define L 500      //Cubeÿһά����
#define N  100000   //��ʼ΢������
#define INIT_FILE_NAME "Manual.txt"    //���ѡ����ļ���ʼ������������ļ�·��
#define SEGMENT 22222
//���ں�SEGMENT        
//    ���ֱ��Ӱ���ļ�����������ú궨���ļ�������ʵ�ڲ��ÿ���...�����ú�SEGMENT�����ļ�����
//    �ں���handleCollision�У����� numOfCluster % SEGMENT == 0�����Ƿ�д���ļ���
//    ��ȥinit��final��д�����ļ����м����й������Ŵظ�������SEGMENT����дһ���ļ������ֻ��д��������ͽ�β�����ļ�����Ȼ��SEGMENT���óɴ���N����
#define DISPLACEMENT 1        //ÿ���ƶ���λ�ƣ���������ûдλ��>1�Ĵ������Բ�Ҫ��  

//���ݽṹ
typedef struct coordinate {
	int a, b, c;
} coordinate;
typedef struct PARTICLE {
	coordinate data;
	struct PARTICLE *next;
} PARTICLE;
typedef struct CLUSTER {
	int sum;                         //���������ھ��ŵ�������
	int Num;                         //�����������ŵı��
	struct  PARTICLE *next;                  //ָ����ŵĽ������������ͷ���
	struct  PARTICLE *rear;                  //ָ����ŵĽ���������������һ�����
} CLUSTER;
int Cube[L][L][L];
CLUSTER cluster[N] = { 0 };
coordinate Directions[] = { { 1, 0, 0 },{ -1, 0, 0 },{ 0, 1, 0 },{ 0, -1, 0 },{ 0, 0, 1 },{ 0, 0, -1 } };
int numOfCluster = N;        //��ȫ�ֱ����滻������ʣ���Ŵظ���������ɨ��cluster�����ʱ��
char file_no = 'A';
CRITICAL_SECTION cs;
int Record = FALSE;        //�ж��Ƿ񽫵�ǰ״̬д���ļ����ɵ�����һ���̸߳����û��������޸�

void Write_File() {
	int i;
	PARTICLE *p;
	FILE *file;
	char FileName[] = "fileN.txt";
	char *file_name = &FileName;
	file_name[4] = file_no;
	if (numOfCluster == N)    strncpy(file_name, "init_", 5);
	if (numOfCluster == 1) strncpy(file_name, "final", 5);
	file = fopen(file_name, "w");
	for (i = 0; i < numOfCluster; i++)
		for (p = cluster[i].next; p != NULL; p = p->next)
			fprintf(file, "%d\t%d\t%d\n", p->data.a, p->data.b, p->data.c);
	//Ҳ���԰�������ע�͵��Ǿ��cluster[i].NumҲд���ļ����õ����ļ��Ϳ���������ʼ��
	//fprintf(file, "%d\t%d\t%d\t%d\n", cluster[i].Num, p->data.a, p->data.b, p->data.c);
	fclose(file);
	printf("��ǰ�Ŵظ���Ϊ��%d,������д���ļ�%s�С�\n", numOfCluster, FileName);
	//�޸���һ���ļ���
	if (numOfCluster != N)    //ֻ��Ϊ�˵ڶ����ļ�����A��ʼ...
		file_no++;
}

//��¼��ǰ��Ϣ������ǵ��������Ǹ��̵߳��õĺ���
//���Ҫ���ٴ��룬��������������������Write_File()�ϲ�����Ϊ������ر����ʾ��Ϣ����ʹ������������Ͳ��ϲ��ˡ�
void Manual_Write_File()
{
	FILE *file;
	int i;
	PARTICLE *p;
	char file_name[] = "Manual.txt";
	printf("\n���ڽ���ǰ״̬д���ļ�......\n��ǰ�Ŵظ���Ϊ%d......\n", numOfCluster);
	file = fopen(&file_name, "w");
	for (i = 0; i < numOfCluster; i++)
		for (p = cluster[i].next; p != NULL; p = p->next)
			fprintf(file, "%d\t%d\t%d\t%d\n", cluster[i].Num, p->data.a, p->data.b, p->data.c);
	fclose(file);
	printf("�ѽ���ǰ״̬д���ļ�%s��......\n�Ŵؼ�������......\n", &file_name);
}

//��ʼ���Ŵ�
void initCluster() {
	int x, y, z;
	int numOfParticle = 0;
	srand((unsigned)time(NULL));
	//��ʼ��Cube
	for (x = 0; x < L; x++)
		for (y = 0; y < L; y++)
			for (z = 0; z < L; z++)
				Cube[x][y][z] = -1;
	//��N��΢����ʼ��
	while (numOfParticle < N) {
		x = rand() % L;
		y = rand() % L;
		z = rand() % L;
		if (Cube[x][y][z] == -1 && \
			Cube[(x - 1 + L) % L][y][z] == -1 && Cube[(x + 1) % L][y][z] == -1 && \
			Cube[x][(y - 1 + L) % L][z] == -1 && Cube[x][(y + 1) % L][z] == -1 && \
			Cube[x][y][(z - 1 + L) % L] == -1 && Cube[x][y][(z + 1) % L] == -1) {
			Cube[x][y][z] = numOfParticle;
			cluster[numOfParticle].Num = numOfParticle;
			cluster[numOfParticle].sum = 1;
			PARTICLE *p = (PARTICLE *)malloc(sizeof(PARTICLE));
			p->data.a = x;
			p->data.b = y;
			p->data.c = z;
			p->next = NULL;
			cluster[numOfParticle].next = p;
			cluster[numOfParticle].rear = p;
			numOfParticle++;
		}
	}
	Write_File();        //��ʼ���ɹ���ӡһ�»�дһ���ļ�
	fflush(stdin);
	printf("��ʼ���ɹ�...\n����ڱ߳�Ϊ%d����������������%d��΢��\n�����������...", L, N);
	getchar();
	system("cls");
}

//�ӳ������й�����д���ļ�Manual.txt�ж�ȡ���ݳ�����ʼ��
//���ѡ����ļ���ʼ����Ҫע��ȷ�������е�L��N�������������ļ�����������һ��
void Init_From_File()
{
	int num, x, y, z;
	PARTICLE *p;
	FILE *file = NULL;
	for (x = 0; x < L; x++)
		for (y = 0; y < L; y++)
			for (z = 0; z < L; z++)
				Cube[x][y][z] = -1;
	for (num = 0; num < N; num++)
	{
		cluster[num].sum = 0;
		cluster[num].Num = num;
		cluster[num].next = cluster[num].rear = NULL;
	}
	file = fopen(INIT_FILE_NAME, "r");
	if (!file)
	{
		printf("�ļ�������......\n");
		exit(0);
	}
	while (!feof(file))
	{
		fscanf(file, "%d\t%d\t%d\t%d\n", &num, &x, &y, &z);
		Cube[x][y][z] = num;
		cluster[num].sum++;
		p = (PARTICLE *)malloc(sizeof(PARTICLE));
		p->data.a = x;
		p->data.b = y;
		p->data.c = z;
		p->next = NULL;
		if (cluster[num].next == NULL)
			cluster[num].next = cluster[num].rear = p;
		else
		{
			numOfCluster--;
			cluster[num].rear->next = p;
			cluster[num].rear = p;
		}
	}
	fflush(stdin);
	printf("���ļ�%s��ʼ���ɹ�����ǰ�Ŵظ���Ϊ%d\n�����������......\n", INIT_FILE_NAME, numOfCluster);
	getchar();
}

// �����Ŵأ�aΪ�ƶ���΢�������Ŵأ�bΪ��ײ΢�������Ŵ�
//��΢�����ٵ��Ŵؼӵ�΢��������Ŵغ��棬�����޸�΢����������ֵ��ʱ��
void updateCluster(int aCluster, int bCluster) {
	CLUSTER *g = (cluster[aCluster].sum >= cluster[bCluster].sum) ? &cluster[aCluster] : &cluster[bCluster];
	CLUSTER *l = (cluster[aCluster].sum < cluster[bCluster].sum) ? &cluster[aCluster] : &cluster[bCluster];
	PARTICLE *temp = l->next;
	g->rear->next = l->next;
	g->rear = l->rear;
	g->sum += l->sum;
	for (; temp != NULL; temp = temp->next)
		Cube[temp->data.a][temp->data.b][temp->data.c] = g->Num;
	l->sum = 0;
	l->next = NULL;
	l->rear = NULL;
	//û��΢�����Ŵض��������棬����cluster[numOfCluster - 1]������cluster�����cluster[numOfCluster]��ʼ�����涼��΢��Ϊ�յ��Ŵ�
	//��΢����Ϊ�յ��Ŵػ���cluster���������棬�´����ѡ���Ŵ�ʱ��rand()%numOfCluster�õ���ʼ������΢�����Ŵ�
	l->sum = cluster[numOfCluster - 1].sum;
	l->next = cluster[numOfCluster - 1].next;
	l->rear = cluster[numOfCluster - 1].rear;
	cluster[numOfCluster - 1].sum = 0;
	cluster[numOfCluster - 1].Num = l->Num;
	cluster[numOfCluster - 1].next = cluster[numOfCluster - 1].rear = NULL;
	//������΢�����Ŵ��ƶ���ǰ�棬����΢����Cube�е�ֵҲ�޸ĳ�ǰ������Ŵغţ���ΪhandleCollision()�������ݵĲ�����Cube�е�ֵ
	for (temp = l->next; temp != NULL; temp = temp->next)
		Cube[temp->data.a][temp->data.b][temp->data.c] = l->Num;
	numOfCluster--;    //�����Ŵط����ۺϣ��Ŵظ�����1
}

//���ƶ�����Ŵؼ����ײ����������ײ�����updateCluster��������
void handleCollision(int runner) {
	int direct;
	int x, y, z;
	PARTICLE *p = cluster[runner].next;
	for (; p != NULL; p = p->next) {
		//�����ײ
		for (direct = 0; direct < 6; direct++) {
			x = (p->data.a + Directions[direct].a + L) % L;
			y = (p->data.b + Directions[direct].b + L) % L;
			z = (p->data.c + Directions[direct].c + L) % L;
			if (Cube[x][y][z] != -1 && Cube[x][y][z] != Cube[p->data.a][p->data.b][p->data.c]) {  //!!!!!!!!!
																								  //    printf("��ǰ�Ŵظ���Ϊ��%d\n", numOfCluster);
																								  //���������жϷ��������ط������ѡ�����Ŵأ���δ��ײ����numOfCluster���䣬����������ظ��ļ�...
																								  //��������Ļ���д��һ�Σ����Ͼͻ����updateCluster������numOfCluster�϶����,��������ظ��ļ�...
				if (numOfCluster % SEGMENT == 0)
					Write_File();
				updateCluster(Cube[p->data.a][p->data.b][p->data.c], Cube[x][y][z]);
			}
		}
	}
}

//���ѡ���Ŵؼ����ƶ��ķ���ͨ��ѭ�������ƶ��Ŵأ�ֱ���Ŵ��ܸ���Ϊ1
//�����Ŵغź��ƶ�����ţ��Ŵ��е�ÿһ��΢�������ƶ�,Ȼ�����handle...����������ײ
void run()
{
	int runner, direction;
	PARTICLE *node;
	printf("�Ŵ���������....\n\r��������������ѡ�������Ӧ�Ĳ���:\n\
                                 \rw.����ǰ״̬д���ļ�\n\
                           \rp.��ʾ��ǰ�Ŵظ���\n\
                                      \re.��������\n");
	while (numOfCluster != 1)
	{
		//�����̼߳�����ٽ���Դrecord����Ĵ���
		EnterCriticalSection(&cs);
		if (Record == TRUE) { Manual_Write_File(); Record = FALSE; }
		LeaveCriticalSection(&cs);

		runner = rand() % numOfCluster;
		direction = rand() % 6;
		//�Ƚ�ÿ��΢������λ�õ�Cube����ֵ��Ϊ-1�����뵥��һ��ѭ��������
		for (node = cluster[runner].next; node != NULL; node = node->next) {
			Cube[node->data.a][node->data.b][node->data.c] = -1;
		}
		//ÿ���Ŵ��е�����΢��һ���ƶ���Ȼ���޸���λ�õ�Cube����ֵ
		for (node = cluster[runner].next; node != NULL; node = node->next) {
			node->data.a = (node->data.a + Directions[direction].a + L) % L;  //( { 0��L-1 } + {-1,0,1} + L ) % L
			node->data.b = (node->data.b + Directions[direction].b + L) % L;
			node->data.c = (node->data.c + Directions[direction].c + L) % L;
			Cube[node->data.a][node->data.b][node->data.c] = cluster[runner].Num;
		}
		handleCollision(runner);
	}
	Write_File();
	printf("�������...\n");
}

//�������� �������� ������ (LPVOIDָ������)
DWORD WINAPI Console_Input(LPVOID lpParamter)
{
	char input;
	while (1)
	{
		fflush(stdin);
		input = getchar();
		if (input == 'w')
		{
			EnterCriticalSection(&cs);
			Record = TRUE;
			Sleep(50);
			LeaveCriticalSection(&cs);
		}
		else if (input == 'p')
		{
			printf("��ǰ�Ŵظ���Ϊ��%d\n", numOfCluster);
		}
		else if (input == 'e')
			exit(0);
		fflush(stdin);
	}
}

int main(int argc, char *argv[]) {

	double start, finish;
	HANDLE Thread;
	char choose;
	while (1)
	{
		fflush(stdin);
		printf("ѡ���ʼ����ʽ:\n\
                              \ra.ֱ�ӳ�ʼ�����г���\n\
                                          \rb.��ȡ�ļ��е����ݳ�ʼ��\n\
                                                      \r�������[a/b]:");
		choose = getchar();
		if (choose != 'a' && choose != 'b') { printf("ѡ��a��b...\n"); system("pause && cls"); continue; }
		else if (choose == 'a') { initCluster(); break; }
		else if (choose == 'b') { Init_From_File(); break; }
	}
	fflush(stdin);
	system("cls");
	Thread = CreateThread(NULL, 0, Console_Input, NULL, 0, NULL);
	CloseHandle(Thread);
	InitializeCriticalSection(&cs);
	start = time(NULL);
	run();
	finish = time(NULL);
	printf("������ʱ%fs\n", difftime(finish, start));
	DeleteCriticalSection(&cs);
	return 0;
}

