#include<lpc214x.h>
#include<math.h>

#define OS_TASKS 4
#define true 1
#define false 0

//task 1,2,3 parameters

unsigned int hp=68;
unsigned int tick, test;

struct TCB
{
		unsigned int taskID;
		unsigned int T;			//Time period
		unsigned int C;			//Execution time
		unsigned int releaseFlag;
		unsigned int runningFlag;
		unsigned int finishFlag;
};

struct TCB Task[OS_TASKS]=
{
	{ 1, 17,  4, false, false, true },  // task1 parameters
	{ 2, 34,  6, false, false, true },  // task2 parameters
	{ 3, 68, 12, false, false, true },   // task3 parameters
	{ 3, 69, 16, false, false, true }   // task4 parameters
};

void task1 (void);
void task2 (void); 
void task3 (void);
void task4 (void);
__irq void tickupdate(void);
void taskRelease(void);
int U_Test(void);
int RTA_Test(void);
void OS_Init(void);
void OS_Sched(void); 
//void OS_CreateTask();
//void OS_CreateMsg();


int main()
{
	PINSEL0=0;
	PINSEL1=0;
	IO0DIR=0xFFFFFFFF;
	T0CTCR=0;
	T0TC=0;
	T0PR=0x000AFFFF;
	T0PC=0;
	T0MR0=hp;
	T0MCR=0x03;
	T0TCR=1;
	
	// Interrupt Configuration
	VICIntSelect=0x00;
	VICVectCntl0=0x24;
	VICVectAddr0=(unsigned)taskRelease;
	VICIntEnable=0x10;
	
	OS_Init();
	while (true)
			OS_Sched();
}

__irq void tickupdate(void)
{
		long int regVal;
		regVal = T0IR; //Read current IR value
		tick=T0TC;
		T0IR = regVal; //Write back to IR to clear Interrupt Flag
		VICVectAddr = 0x0; //This is to signal end of interrupt execution
}

void task1()
{
	int l;
	Task[0].releaseFlag = false;
	Task[0].runningFlag = true;
	IO0CLR=0x0F;
	for(l=0;l<Task[0].C;)
	{
		
		if(tick != T0TC)
		{
			//tick = T0TC;
			l++;
			//T0IR=1;
		}
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
	IO0CLR=0x0F;
	for(m=0;m < Task[1].C;)
	{
			
			if(tick != T0TC)
			{
				tick = T0TC;
				m++;
				//T0IR=1;
				taskRelease();
				if ((Task[0].releaseFlag == true) && (Task[0].runningFlag == false))
				{
					task1();
					IO0CLR=0x0F;
				}
					
			}
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
	IO0CLR=0x0F;
	for(n=0;n < Task[2].C;)
	{
			
			if(tick != T0TC)
			{
				tick = T0TC;
				n++;
				//T0IR=1;
				taskRelease();
				if ((Task[0].releaseFlag == true) && (Task[0].runningFlag == false))
				{
					task1();
					IO0CLR=0x0F;
				}
				else if ((Task[1].releaseFlag == true) && (Task[1].runningFlag == false))
				{
					task2();
					IO0CLR=0x0F;
				}
			}
			IO0SET=0x04;
	}
	Task[2].finishFlag = true;
	Task[2].runningFlag = false;
}

void task4()
{
	int g;
	Task[3].releaseFlag = false;
	Task[3].runningFlag = true;
	IO0CLR=0x0F;
	for(g=0;g < Task[2].C;)
	//for(;T0TC<Task[0].C;)
	{
			if(tick != T0TC)
			{
				tick = T0TC;
				g++;
				//T0IR=1;
				taskRelease();
				if ((Task[0].releaseFlag == true) && (Task[0].runningFlag == false))
				{
					task1();
					IO0CLR=0x0F;
				}
				else if ((Task[1].releaseFlag == true) && (Task[1].runningFlag == false))
				{
					task2();
					IO0CLR=0x0F;
				}
				else if ((Task[2].releaseFlag == true) && (Task[2].runningFlag == false))
				{
					task3();
					IO0CLR=0x0F;
				}
				IO0SET=0x08;
			}
	}
	Task[3].finishFlag = true;
	Task[3].runningFlag = false;
}

void OS_Init(void)
{
		int i,j;
		//hp = 20; //hyperPeriod();
	
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
		tick=T0TC;
		IO0CLR |= 0x80000000;
		if(test==0)
		{
			IO0SET |= 0x80000000;
		}
		for (;T0TC <= hp;)
		{
			taskRelease();
			IO0CLR=0x0F;
			if ((Task[0].releaseFlag == true) && (Task[0].runningFlag == false))
			{
				IO0CLR=0x0F;
				task1();
			}
			if((Task[1].releaseFlag == true) && (Task[1].runningFlag == false))
			{
				IO0CLR=0x0F;
				task2();
			}
			if((Task[2].releaseFlag == true) && (Task[2].runningFlag == false))
			{
				IO0CLR=0x0F;
				task3();
			}
			if((Task[3].releaseFlag == true) && (Task[3].runningFlag == false))
			{
				IO0CLR=0x0F;
				task4();
			}
			
		}
}

void taskRelease()
{
		
			int k;
			for (k = 0; k < OS_TASKS; k++)
			{
					// Releases task periodically
					if (!(T0TC % Task[k].T))
					{
						if (Task[k].finishFlag == true)
								Task[k].releaseFlag = true;
						//else
								//printf("Deadline Missed for task %d", Task[j].taskID);
					}
			}
}

/*void hyperPeriod()
{
	//find lcm of T1,T2,T3
}
*/


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
