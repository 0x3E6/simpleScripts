//2016.06.09
//团簇分形凝聚模型
#include<stdio.h>
#include<math.h>
#include<time.h>
#include<stdlib.h>
#include<Windows.h>
//一些特别的数字    运行程序之前，注意设置前4个宏
#define L 500      //Cube每一维长度
#define N  100000   //初始微粒个数
#define INIT_FILE_NAME "Manual.txt"    //如果选择从文件初始化，这个就是文件路径
#define SEGMENT 22222
//关于宏SEGMENT        
//    这个直接影响文件个数，如果用宏定义文件个数，实在不好控制...所以用宏SEGMENT操作文件个数
//    在函数handleCollision中，根据 numOfCluster % SEGMENT == 0决定是否写入文件，
//    除去init和final会写两个文件，中间运行过程中团簇个数减少SEGMENT个就写一次文件，如果只想写两个最初和结尾两个文件，显然把SEGMENT设置成大于N即可
#define DISPLACEMENT 1        //每次移动的位移，本代码中没写位移>1的处理，所以不要改  

//数据结构
typedef struct coordinate {
	int a, b, c;
} coordinate;
typedef struct PARTICLE {
	coordinate data;
	struct PARTICLE *next;
} PARTICLE;
typedef struct CLUSTER {
	int sum;                         //本粒子所在聚团的粒子数
	int Num;                         //粒子所属聚团的编号
	struct  PARTICLE *next;                  //指向聚团的胶体粒子链表的头结点
	struct  PARTICLE *rear;                  //指向聚团的胶体粒子链表的最后一个结点
} CLUSTER;
int Cube[L][L][L];
CLUSTER cluster[N] = { 0 };
coordinate Directions[] = { { 1, 0, 0 },{ -1, 0, 0 },{ 0, 1, 0 },{ 0, -1, 0 },{ 0, 0, 1 },{ 0, 0, -1 } };
int numOfCluster = N;        //用全局变量替换函数求剩余团簇个数，减少扫描cluster数组的时间
char file_no = 'A';
CRITICAL_SECTION cs;
int Record = FALSE;        //判断是否将当前状态写入文件，由单独的一个线程根据用户输入来修改

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
	//也可以按照下面注释的那句把cluster[i].Num也写入文件，得到的文件就可以用来初始化
	//fprintf(file, "%d\t%d\t%d\t%d\n", cluster[i].Num, p->data.a, p->data.b, p->data.c);
	fclose(file);
	printf("当前团簇个数为：%d,数据已写入文件%s中。\n", numOfCluster, FileName);
	//修改下一个文件名
	if (numOfCluster != N)    //只是为了第二个文件名从A开始...
		file_no++;
}

//记录当前信息。这个是单独创建那个线程调用的函数
//如果要减少代码，这个函数还可以与上面的Write_File()合并，但为了输出特别的提示信息，且使代码更清晰，就不合并了。
void Manual_Write_File()
{
	FILE *file;
	int i;
	PARTICLE *p;
	char file_name[] = "Manual.txt";
	printf("\n正在将当前状态写入文件......\n当前团簇个数为%d......\n", numOfCluster);
	file = fopen(&file_name, "w");
	for (i = 0; i < numOfCluster; i++)
		for (p = cluster[i].next; p != NULL; p = p->next)
			fprintf(file, "%d\t%d\t%d\t%d\n", cluster[i].Num, p->data.a, p->data.b, p->data.c);
	fclose(file);
	printf("已将当前状态写入文件%s中......\n团簇继续凝聚......\n", &file_name);
}

//初始化团簇
void initCluster() {
	int x, y, z;
	int numOfParticle = 0;
	srand((unsigned)time(NULL));
	//初始化Cube
	for (x = 0; x < L; x++)
		for (y = 0; y < L; y++)
			for (z = 0; z < L; z++)
				Cube[x][y][z] = -1;
	//对N个微粒初始化
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
	Write_File();        //初始化成功打印一下或写一个文件
	fflush(stdin);
	printf("初始化成功...\n随机在边长为%d的立方中随机添加了%d个微粒\n按任意键继续...", L, N);
	getchar();
	system("cls");
}

//从程序运行过程中写的文件Manual.txt中读取数据初进行始化
//如果选择从文件初始化，要注意确保程序中的L，N等数据与所读文件的生成数据一致
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
		printf("文件不存在......\n");
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
	printf("从文件%s初始化成功，当前团簇个数为%d\n按任意键继续......\n", INIT_FILE_NAME, numOfCluster);
	getchar();
}

// 更新团簇，a为移动的微粒所在团簇，b为被撞微粒所在团簇
//将微粒数少的团簇加到微粒数多的团簇后面，减少修改微粒所在数组值的时间
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
	//没有微粒的团簇都换到后面，换到cluster[numOfCluster - 1]，这样cluster数组从cluster[numOfCluster]开始，后面都是微粒为空的团簇
	//将微粒数为空的团簇换到cluster数组的最后面，下次随机选择团簇时用rand()%numOfCluster得到的始终是有微粒的团簇
	l->sum = cluster[numOfCluster - 1].sum;
	l->next = cluster[numOfCluster - 1].next;
	l->rear = cluster[numOfCluster - 1].rear;
	cluster[numOfCluster - 1].sum = 0;
	cluster[numOfCluster - 1].Num = l->Num;
	cluster[numOfCluster - 1].next = cluster[numOfCluster - 1].rear = NULL;
	//后面有微粒的团簇移动到前面，将该微粒的Cube中的值也修改成前面这个团簇号，因为handleCollision()函数传递的参数是Cube中的值
	for (temp = l->next; temp != NULL; temp = temp->next)
		Cube[temp->data.a][temp->data.b][temp->data.c] = l->Num;
	numOfCluster--;    //两个团簇发生聚合，团簇个数减1
}

//对移动后的团簇检测碰撞，发生了碰撞则调用updateCluster更新数据
void handleCollision(int runner) {
	int direct;
	int x, y, z;
	PARTICLE *p = cluster[runner].next;
	for (; p != NULL; p = p->next) {
		//检测碰撞
		for (direct = 0; direct < 6; direct++) {
			x = (p->data.a + Directions[direct].a + L) % L;
			y = (p->data.b + Directions[direct].b + L) % L;
			z = (p->data.c + Directions[direct].c + L) % L;
			if (Cube[x][y][z] != -1 && Cube[x][y][z] != Cube[p->data.a][p->data.b][p->data.c]) {  //!!!!!!!!!
																								  //    printf("当前团簇个数为：%d\n", numOfCluster);
																								  //如果下面的判断放在其他地方，如果选到空团簇，或未碰撞，则numOfCluster不变，会产生几个重复文件...
																								  //放在这里的话，写入一次，马上就会调用updateCluster函数，numOfCluster肯定会变,不会产生重复文件...
				if (numOfCluster % SEGMENT == 0)
					Write_File();
				updateCluster(Cube[p->data.a][p->data.b][p->data.c], Cube[x][y][z]);
			}
		}
	}
}

//随机选择团簇及其移动的方向，通过循环不断移动团簇，直到团簇总个数为1
//根据团簇号和移动方向号，团簇中的每一个微粒做出移动,然后调用handle...函数处理碰撞
void run()
{
	int runner, direction;
	PARTICLE *node;
	printf("团簇正在凝聚....\n\r您可以输入下面选项进行相应的操作:\n\
                                 \rw.将当前状态写入文件\n\
                           \rp.显示当前团簇个数\n\
                                      \re.结束程序\n");
	while (numOfCluster != 1)
	{
		//关于线程间访问临界资源record互斥的代码
		EnterCriticalSection(&cs);
		if (Record == TRUE) { Manual_Write_File(); Record = FALSE; }
		LeaveCriticalSection(&cs);

		runner = rand() % numOfCluster;
		direction = rand() % 6;
		//先将每个微粒所在位置的Cube数组值置为-1，必须单独一个循环！！！
		for (node = cluster[runner].next; node != NULL; node = node->next) {
			Cube[node->data.a][node->data.b][node->data.c] = -1;
		}
		//每个团簇中的所有微粒一起移动，然后修改新位置的Cube数组值
		for (node = cluster[runner].next; node != NULL; node = node->next) {
			node->data.a = (node->data.a + Directions[direction].a + L) % L;  //( { 0～L-1 } + {-1,0,1} + L ) % L
			node->data.b = (node->data.b + Directions[direction].b + L) % L;
			node->data.c = (node->data.c + Directions[direction].c + L) % L;
			Cube[node->data.a][node->data.b][node->data.c] = cluster[runner].Num;
		}
		handleCollision(runner);
	}
	Write_File();
	printf("凝聚完成...\n");
}

//返回类型 调用类型 函数名 (LPVOID指针类型)
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
			printf("当前团簇个数为：%d\n", numOfCluster);
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
		printf("选择初始化方式:\n\
                              \ra.直接初始化运行程序\n\
                                          \rb.读取文件中的数据初始化\n\
                                                      \r输入序号[a/b]:");
		choose = getchar();
		if (choose != 'a' && choose != 'b') { printf("选择a或b...\n"); system("pause && cls"); continue; }
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
	printf("凝聚用时%fs\n", difftime(finish, start));
	DeleteCriticalSection(&cs);
	return 0;
}

