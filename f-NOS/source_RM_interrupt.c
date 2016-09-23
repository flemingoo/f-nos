#include<lpc214x.h>
#include<math.h>

#define OS_TASKS 3
#define true 1
#define false 0

//task 1,2,3 parameters

unsigned int hp=30;
int tick=0, test;

void task1 (void);
void task2 (void); 
void task3 (void);
__irq void taskRelease(void);
int U_Test(void);
int RTA_Test(void);
void OS_Init(void);
void OS_Sched(void);

struct TCB
{
		unsigned int taskID;
		unsigned int T;			//Time period
		unsigned int C;			//Execution time
		unsigned int releaseFlag;
		unsigned int runningFlag;
		unsigned int finishFlag;
		void (*taskPointer)(void);
};

struct TCB Task[OS_TASKS]=
{
	{ 1, 10,  2, false, false, true, task1 },  // task1 parameters
	{ 2,  4,  1, false, false, true, task2 },  // task2 parameters
	{ 3,  5,  1, false, false, true, task3 }   // task3 parameters
};


int main()
{	
	PINSEL0=0;
	PINSEL1=0;
	IO0DIR=0xFFFFFFFF;
	// Time base configuration
	T0CTCR=0;
	T0TC=0;
	T0PR=0x007FFFFF;
	T0PC=0;
	T0MR0=1;
	T0MCR=0x03;
	
	// Interrupt Configuration
	VICIntSelect=0x00;
	VICVectCntl0=0x24;
	VICVectAddr0=(unsigned)taskRelease;
	VICIntEnable=0x10;
	
	T0TCR=1;
	
	OS_Init();
	while (true)
			OS_Sched();
}



void OS_Init(void)
{
		int i,j;
		// U & RTA Test
		if(U_Test())
			test=1;
		else if(RTA_Test())
			test=1;
		else 
			test=0;
		
		// Sorting task w.r.t priority
		for (i= 0; i < (OS_TASKS - 1); i++)
				for (j = 0; j < (OS_TASKS - 1 - i); j++)
				{
					struct TCB temp;
					if (Task[j].T > Task[j + 1].T)
					{
						temp = Task[j];
						Task[j] = Task[j + 1];
						Task[j + 1] = temp;
					}
				}
}

void OS_Sched(void)
{
		int k;
		IO0CLR |= 0xFFFFFFFF;
		if(test==1)
		{
			IO0SET |= 0x80000000;
		}
		for (k = 0; k < OS_TASKS; k++)
		{
			if(Task[k].releaseFlag == 1 | Task[k].runningFlag ==1)
				Task[k].taskPointer();
		}
}


__irq void taskRelease()
{
		
		int a,b;
		long int regVal;
		regVal = T0IR; //Read current IR value
		tick++;
		if(tick == hp)
			tick=0;
		for (b = 0; b < OS_TASKS; b++)
		{
				// Releases task periodically
				if (!(tick % Task[b].T))
				{	
						Task[b].releaseFlag = true;
				}
		}
		for(a=0; a < OS_TASKS ; a++)
			if (Task[a].releaseFlag == true )
			{
				Task[a].taskPointer();
				break;
			}
		T0IR = regVal; //Write back to IR to clear Interrupt Flag
		VICVectAddr = 0x0; //This is to signal end of interrupt execution
}


int U_Test()
{
	int p;
	float U=0.0,Ub=0.0;
	for(p=0; p < OS_TASKS; p++)
	{
		U = U+(Task[p].C / Task[p].T);
	}
	Ub = OS_TASKS*(pow(2.0,(1/OS_TASKS))-1);
	if(U <= Ub)
		return 1;
	else 
		return 0;
}

void task1()
{
	int l;
	Task[0].releaseFlag = false;
	Task[0].runningFlag = true;
	for(l=0;l<Task[0].C;l++)
	{
		IO0SET=0x01;
	}
	Task[0].finishFlag = true;
	Task[0].runningFlag = false;
}

void task2()
{
	int m;
	Task[1].releaseFlag = false;
	Task[1].runningFlag = true;
	for(m=0;m < Task[1].C;m++)
	{
			IO0SET=0x02;			
	}
	Task[1].finishFlag = true;
	Task[1].runningFlag = false;
}


void task3()
{
	int n;
	Task[2].releaseFlag = false;
	Task[2].runningFlag = true;
	for(n=0;n < Task[2].C;n++)
	{
			IO0SET=0x04;			
	}
	Task[2].finishFlag = true;
	Task[2].runningFlag = false;
}

int RTA_Test()
{
	int p,q,r, rtaFlag=1;
	float R0,I,K;
	float R[OS_TASKS];
	for(p=(OS_TASKS-1);p>=0;p--)
	{
		R[p] = Task[p].C;
		R0=R[p];
		for(;;)
		{
			I = 0;
			for(q=(p-1);q>=0;q--)
			{
				K=(R[p]/Task[q].T);
				I = I + (ceil(K))*Task[q].C;
			}
	
			R[p] = Task[p].C + I;
			if(R[p]==R0)
				break;
			R0 = R[p];
		}
	}
	for(r=(OS_TASKS-1);r>=0;r--)
		if(R[r] > Task[r].T)
			rtaFlag=0;
	return rtaFlag;
	
}

/*
int gcd(int a, int b)
{
    if (a<b) return gcd(b,a);
    if (a%b==0) return b;
    else return gcd(b, a%b);
}

int lcm(int a, int b)
{
    return ((a*b)/gcd(a,b));
}
*/
