// MemoryPool_test.cpp : �ܼ� ���� ���α׷��� ���� �������� �����մϴ�.
//

#include "stdafx.h"
#include "lib\Library.h"
#include "MemoryPool.h"
#include "LockFreeStack.h"
#include "LockFreeQueue.h"
CCrashDump Dump;

#define defData 0x0000000055555555
#define defCnt 0

struct st_TEST_DATA
{
	INT64 Data;
	INT64 Cnt;
	st_TEST_DATA(void)
	{
		Data = defData;
		Cnt = defCnt;
	}
};

unsigned int __stdcall MemoryPoolThread (void *pParam);
unsigned int __stdcall SpeedTestThread (void *pParam);
unsigned int __stdcall LF_StackTestThread (void *pParam);
unsigned int __stdcall LF_Queue_InQueue_Thread (void *pParam);
unsigned int __stdcall LF_Queue_DeQueue_Thread (void *pParam);

#define dfTHREAD_ALLOC 10000
#define dfTHREAD_MAX 6
#define dfTESTLOOP_MAX 1000
#define InQueueSleep 0
#define DeQueueSleep 0

CMemoryPool<st_TEST_DATA> *g_Mempool;
CMemoryPool_LF<st_TEST_DATA> *g_Mempool_LF;
CStack_LF<st_TEST_DATA *> *g_LF_Stack;
CQueue_LF<st_TEST_DATA *> *g_LF_Queue;


LONG64 LF_MemPool_Th_TPS = 0;
LONG64 Speed_Th_TPS = 0;
LONG64 InQueue_Th_TPS = 0;
LONG64 DeQueue_Th_TPS = 0;

bool TESTEnd;


void MemoryPoolTESTMain (void);
void LF_Stack_TESTMain (void);
void LF_Queue_TESTMain (void);
bool SpeedtestMain (void);
int main()
{
	
	g_Mempool = new CMemoryPool<st_TEST_DATA> (0);
	g_Mempool_LF = new CMemoryPool_LF<st_TEST_DATA> (0);
	g_LF_Stack = new CStack_LF<st_TEST_DATA *> ();
	g_LF_Queue = new CQueue_LF<st_TEST_DATA *> (0);

	LOG_DIRECTORY (L"LOG");
	LOG_LEVEL (LOG_DEBUG,false);
	LOG_LOG (L"main", LOG_SYSTEM, L"Start");

	HANDLE hThread[dfTHREAD_MAX];
	DWORD dwThreadID;

	int Cnt;

	while ( 1 )
	{
		wprintf (L"\n������ �׽�Ʈ ��� \n");
		wprintf (L"1.LF_MemPool \n");
		wprintf (L"2.LF_Stack \n");
		wprintf (L"3.LF_Queue // ������ \n");
		wprintf (L"4. MemPool SpeedTest\n");
		wscanf_s (L"%d",&Cnt);

		switch ( Cnt )
		{
		case 1:
			for ( int iCnt = 0; iCnt < dfTHREAD_MAX; iCnt++ )
			{
				hThread[iCnt] = ( HANDLE )_beginthreadex (NULL, 0, MemoryPoolThread, ( LPVOID )0, 0, ( unsigned int * )&dwThreadID);
			}

			break;

		case 2:
			for ( int iCnt = 0; iCnt < dfTHREAD_MAX; iCnt++ )
			{
				hThread[iCnt] = ( HANDLE )_beginthreadex (NULL, 0, LF_StackTestThread, ( LPVOID )0, 0, ( unsigned int * )&dwThreadID);
			}

			break;

		case 3:
			for ( int iCnt = 0; iCnt < 2; iCnt++ )
			{
				hThread[iCnt] = ( HANDLE )_beginthreadex (NULL, 0, LF_Queue_InQueue_Thread, ( LPVOID )0, 0, ( unsigned int * )&dwThreadID);
			}

			
			for ( int iCnt = 2; iCnt < dfTHREAD_MAX ; iCnt++ )
			{
				hThread[iCnt] = ( HANDLE )_beginthreadex (NULL, 0, LF_Queue_DeQueue_Thread, ( LPVOID )0, 0, ( unsigned int * )&dwThreadID);
			}
			
			break;
		case 4:

			for ( int iCnt = 0; iCnt < 1; iCnt++ )
			{
				hThread[iCnt] = ( HANDLE )_beginthreadex (NULL, 0, SpeedTestThread, ( LPVOID )0, 0, ( unsigned int * )&dwThreadID);
			}
			TESTEnd = false;
			break;

		default:
			continue;
		}
		break;
	}


	bool EndFlag;
	while ( 1 )
	{
		Sleep (999);

		switch ( Cnt )
		{
		case 1:
			MemoryPoolTESTMain ();
			break;
		case 2:
			LF_Stack_TESTMain ();
			break;
		case 3:
			LF_Queue_TESTMain ();
			break;
		case 4:
			EndFlag = SpeedtestMain ();
			break;
		}
		if ( Cnt == 4 && EndFlag )
		{

			PROFILE_PRINT;

			break;
		}
	}

	return 0;
}


void MemoryPoolTESTMain (void)
{
	wprintf (L"\n");
	wprintf (L"MemPool Alloc Cnt = %lld\n", g_Mempool_LF->GetAllocCount ());
	wprintf (L"MemPool Full Cnt = %lld\n", g_Mempool_LF->GetFullCount ());
	wprintf (L"Test Thread TPS = %lld\n", LF_MemPool_Th_TPS);
	wprintf (L"\n");
	LF_MemPool_Th_TPS = 0;
}

void LF_Stack_TESTMain (void)
{
	wprintf (L"\n");
	wprintf (L"LF_Stack Full Size = %lld\n", g_LF_Stack->GetFullNode ());
	wprintf (L"LF_Stack Use Size = %lld\n", g_LF_Stack->GetUseSize ());
	wprintf (L"Test Thread TPS = %lld\n", Speed_Th_TPS);
	wprintf (L"\n");
	Speed_Th_TPS = 0;
}
void LF_Queue_TESTMain (void)
{
	wprintf (L"\n");
	wprintf (L"LF_Queue Use Size = %lld\n", g_LF_Queue->GetUseSize());
	wprintf (L"Test InQueueThread TPS = %lld\n", InQueue_Th_TPS);
	wprintf (L"Test DeQueueThread TPS = %lld\n\n", DeQueue_Th_TPS);

	wprintf (L"MemPool Alloc Cnt = %lld\n", g_Mempool_LF->GetAllocCount ());
	wprintf (L"MemPool Full Cnt = %lld\n", g_Mempool_LF->GetFullCount ());
	wprintf (L"\n");
	InQueue_Th_TPS = 0;
	DeQueue_Th_TPS = 0;
}

bool SpeedtestMain (void)
{
	if ( TESTEnd )
	{
		return true;
	}
	wprintf (L"\n");
	wprintf (L"Test Thread TPS = %lld\n", Speed_Th_TPS);
	wprintf (L"\n");
	Speed_Th_TPS = 0;
	return false;
}



//////////////////////////////////////////////////////////////////////////////////////////////////////
//�޸�Ǯ �׽�Ʈ ������
//////////////////////////////////////////////////////////////////////////////////////////////////////

unsigned int __stdcall MemoryPoolThread (void *pParam)
{
	int iCnt;
	st_TEST_DATA *pDataArray[dfTHREAD_ALLOC];

	/*------------------------------------------------------------------*/
	///////  �޸�Ǯ �׽�Ʈ ��  //////////////////////////////////////////////////////////////////////////////////////////

	// �������� �����忡�� ���������� Alloc �� Free �� �ݺ������� ��
	// ��� �����ʹ� 0x0000000055555555 ���� �ʱ�ȭ �Ǿ� ����.

	// 0. Alloc (������� 10000 �� x 10 �� ������ �� 10����)
	// 1. 0x0000000055555555 �� �´��� Ȯ��.
	// 2. Interlocked + 1 (Data + 1 / Count + 1)
	// 3. �ణ���
	// 4. ������ 0x0000000055555556 �� �´��� (Count == 1) Ȯ��.
	// 5. Interlocked - 1 (Data - 1 / Count - 1)
	// 6. �ణ���
	// 7. 0x0000000055555555 �� �´��� (Count == 0) Ȯ��.
	// 8. Free
	// �ݺ�.

	// �׽�Ʈ ����
	//
	// - �Ҵ�� �޸𸮸� �� �Ҵ� �ϴ°� ?
	// - �߸��� �޸𸮸� �Ҵ� �ϴ°� ?
	/*------------------------------------------------------------------*/


	while ( 1 )
	{


		for ( iCnt = 0; iCnt < dfTHREAD_ALLOC; iCnt++ )
		{
			pDataArray[iCnt] = g_Mempool_LF->Alloc ();
			if ( pDataArray[iCnt] == NULL )
			{
				CCrashDump::Crash ();
			}
		}


		for ( iCnt = 0; iCnt < dfTHREAD_ALLOC; iCnt++ )
		{
			if ( pDataArray[iCnt]->Data != defData )
			{
				CCrashDump::Crash ();
			}
			if ( pDataArray[iCnt]->Cnt != defCnt )
			{
				CCrashDump::Crash ();
			}
	
		}



		for ( iCnt = 0; iCnt < dfTHREAD_ALLOC; iCnt++ )
		{
			InterlockedIncrement64 ((volatile LONG64 *)&pDataArray[iCnt]->Data);
			InterlockedIncrement64 (( volatile LONG64 * )&pDataArray[iCnt]->Cnt);
		}

	//	Sleep (2);

		for ( iCnt = 0; iCnt < dfTHREAD_ALLOC; iCnt++ )
		{
			if ( pDataArray[iCnt]->Data != defData + 1 || pDataArray[iCnt]->Cnt != defCnt + 1 )
				CCrashDump::Crash ();
		}


		for ( iCnt = 0; iCnt < dfTHREAD_ALLOC; iCnt++ )
		{
			InterlockedDecrement64 (( volatile LONG64 * )&pDataArray[iCnt]->Data);
			InterlockedDecrement64 (( volatile LONG64 * )&pDataArray[iCnt]->Cnt);
		}

		for ( iCnt = 0; iCnt < dfTHREAD_ALLOC; iCnt++ )
		{
			if ( pDataArray[iCnt]->Data != defData )
			{
				CCrashDump::Crash ();
			} 
			if ( pDataArray[iCnt]->Cnt != defCnt )
			{
				CCrashDump::Crash ();
			}
		}

		for ( iCnt = 0; iCnt < dfTHREAD_ALLOC; iCnt++ )
		{
			if ( g_Mempool_LF->Free (pDataArray[iCnt]) == false )
			{
				CCrashDump::Crash ();
			}
		}
		Sleep (5);

		InterlockedIncrement64 (( volatile LONG64 * )&LF_MemPool_Th_TPS);

	}


	return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////
//malloc,new.MemPool �ӵ� �׽�Ʈ�� ������
//////////////////////////////////////////////////////////////////////////////////////////////////////

unsigned int __stdcall SpeedTestThread (void *pParam)
{
	/*------------------------------------------------------------------*/
	/////// �ӵ� �׽�Ʈ ��  //////////////////////////////////////////////

	// �������� �����忡�� ���������� Alloc �� Free �� �ݺ������� ��
	// ��� �����ʹ� 0x0000000055555555 ���� �ʱ�ȭ �Ǿ� ����.

	//�� �����Ҵ� �����ڿ� ���� �۾�.
	// 0. Alloc (������� 10000 �� x 10 �� ���� ������ �� 10����)
	// 1. �ణ���
	// 2. Free
	// LoopMax��ŭ �ݺ���.

	// �׽�Ʈ ����
	//
	// - �ۼ��� �޸�Ǯ���� �ӵ� �׽�Ʈ.
	/*------------------------------------------------------------------*/

	int iCnt;

	st_TEST_DATA *pDataArray[dfTHREAD_ALLOC];

	for ( int Cnt = 0; Cnt < dfTESTLOOP_MAX * 10; Cnt++ )
	{

		//================================================================
		//malloc
		for ( iCnt = 0; iCnt < dfTHREAD_ALLOC; iCnt++ )
		{
			PROFILE_BEGIN (L"malloc_Alloc");
			pDataArray[iCnt] = ( st_TEST_DATA * )malloc (sizeof (st_TEST_DATA));
			PROFILE_END (L"malloc_Alloc");
			if ( pDataArray[iCnt] == NULL )
			{
				CCrashDump::Crash ();
			}
		}

		for ( iCnt = 0; iCnt < dfTHREAD_ALLOC; iCnt++ )
		{
			PROFILE_BEGIN (L"malloc_Free");
			free (pDataArray[iCnt]);
			PROFILE_END (L"malloc_Free");
		}
		//================================================================
		//new
		for ( iCnt = 0; iCnt < dfTHREAD_ALLOC; iCnt++ )
		{
			PROFILE_BEGIN (L"new_Alloc");
			pDataArray[iCnt] = new(st_TEST_DATA);
			PROFILE_END (L"new_Alloc");
			if ( pDataArray[iCnt] == NULL )
			{
				CCrashDump::Crash ();
			}
		}

		for ( iCnt = 0; iCnt < dfTHREAD_ALLOC; iCnt++ )
		{
			PROFILE_BEGIN (L"new_delete");
			delete pDataArray[iCnt];
			PROFILE_END (L"new_delete");
		}

		//================================================================
		//LOCK���� �޸�Ǯ
		for ( iCnt = 0; iCnt < dfTHREAD_ALLOC; iCnt++ )
		{
			PROFILE_BEGIN (L"MemPoolLock_Alloc");
			pDataArray[iCnt] = g_Mempool->Alloc ();
			PROFILE_END (L"MemPoolLock_Alloc");
			if ( pDataArray[iCnt] == NULL )
			{
				CCrashDump::Crash ();
			}
		}

		for ( iCnt = 0; iCnt < dfTHREAD_ALLOC; iCnt++ )
		{
			PROFILE_BEGIN (L"MemPoolLock_Free");
			g_Mempool->Free (pDataArray[iCnt]);
			PROFILE_END (L"MemPoolLock_Free");
		}

		//================================================================
		//LF���� �޸�Ǯ
		for ( iCnt = 0; iCnt < dfTHREAD_ALLOC; iCnt++ )
		{
			PROFILE_BEGIN (L"MemPoolLF_Alloc");
			pDataArray[iCnt] = g_Mempool_LF->Alloc ();
			PROFILE_END (L"MemPoolLF_Alloc");
			if ( pDataArray[iCnt] == NULL )
			{
				CCrashDump::Crash ();
			}
		}

		for ( iCnt = 0; iCnt < dfTHREAD_ALLOC; iCnt++ )
		{
			PROFILE_BEGIN (L"MemPoolLF_Free");
			g_Mempool_LF->Free (pDataArray[iCnt]);
			PROFILE_END (L"MemPoolLF_Free");
		}
		InterlockedIncrement64 (( volatile LONG64 * )&Speed_Th_TPS);
		Sleep (2);
	}

	TESTEnd = true;

	return 0;
}




//////////////////////////////////////////////////////////////////////////////////////////////////////
//LF_Stack �׽�Ʈ�� ������
//////////////////////////////////////////////////////////////////////////////////////////////////////

unsigned int __stdcall LF_StackTestThread (void *pParam)
{
	/*------------------------------------------------------------------*/
	/////// ������ ���� �׽�Ʈ //////////////////////////////////////////////

	// �������� �����忡�� ���������� PUSH �� POP�� �ݺ������� ��
	// ��� �����ʹ� 0x0000000055555555 ���� �ʱ�ȭ �Ǿ� ����.

	//�� �����Ҵ� �����ڿ� ���� �۾�.
	// 0. Alloc (������� 10000 �� x 10 �� ���� ������ �� 10����)
	// 1. �ణ���
	// 2. Free
	// LoopMax��ŭ �ݺ���.

	// �׽�Ʈ ����
	//
	// - �ۼ��� �޸�Ǯ���� �ӵ� �׽�Ʈ.
	/*------------------------------------------------------------------*/

	int iCnt;

	st_TEST_DATA *pDataArray[dfTHREAD_ALLOC];


	for ( iCnt = 0; iCnt < dfTHREAD_ALLOC; iCnt++ )
	{
		pDataArray[iCnt] = new st_TEST_DATA ();
		if ( pDataArray[iCnt] == NULL )
		{
			CCrashDump::Crash ();
		}
	}

	//================================================================
	//�ʱ�ȭ

	for ( iCnt = 0; iCnt < dfTHREAD_ALLOC; iCnt++ )
	{
		pDataArray[iCnt]->Cnt = 1;
		pDataArray[iCnt]->Data = 0x0000000055555555;
	}

	while(1)
	{
		//================================================================
		//PUSH
		for ( iCnt = 0; iCnt < dfTHREAD_ALLOC; iCnt++ )
		{
			g_LF_Stack->Push (pDataArray[iCnt]);

		}

		Sleep (10);


		//================================================================
		//POP
		for ( iCnt = 0; iCnt < dfTHREAD_ALLOC; iCnt++ )
		{
			if ( g_LF_Stack->Pop (&pDataArray[iCnt]) ==false)
			{
				CCrashDump::Crash ();
			}
		}

		//================================================================
		//�ʱⰪ �´��� üũ.
		for ( iCnt = 0; iCnt < dfTHREAD_ALLOC; iCnt++ )
		{
			if ( pDataArray[iCnt]->Data != 0x0000000055555555 )
			{
				CCrashDump::Crash ();
			}
			if ( pDataArray[iCnt]->Cnt != 1 )
			{
				CCrashDump::Crash ();
			}
		}


		//================================================================
		//interlocked�� ����.
		for ( iCnt = 0; iCnt < dfTHREAD_ALLOC; iCnt++ )
		{
			InterlockedIncrement64 (( volatile LONG64 * )&pDataArray[iCnt]->Data);
			InterlockedIncrement64 (( volatile LONG64 * )&pDataArray[iCnt]->Cnt);
		}

		Sleep (10);

		//================================================================
		//interlocked�� �����Ȱ��� �´��� üũ.
		for ( iCnt = 0; iCnt < dfTHREAD_ALLOC; iCnt++ )
		{
			if ( pDataArray[iCnt]->Data != 0x0000000055555556 )
			{
				CCrashDump::Crash ();
			}
			if ( pDataArray[iCnt]->Cnt != 2 )
			{
				CCrashDump::Crash ();
			}
		}

		//================================================================
		//interlocked�� �ʱⰪ���� ����
		for ( iCnt = 0; iCnt < dfTHREAD_ALLOC; iCnt++ )
		{
			InterlockedDecrement64 (( volatile LONG64 * )&pDataArray[iCnt]->Data);
			InterlockedDecrement64 (( volatile LONG64 * )&pDataArray[iCnt]->Cnt);
		}

	//	Sleep (10);

		//================================================================
		//interlocked�� ���ҵ� ���� �´��� üũ.
		for ( iCnt = 0; iCnt < dfTHREAD_ALLOC; iCnt++ )
		{
			if ( pDataArray[iCnt]->Data != 0x0000000055555555 )
			{
				CCrashDump::Crash ();
			}
			if ( pDataArray[iCnt]->Cnt != 1 )
			{
				CCrashDump::Crash ();
			}
		}

		InterlockedIncrement64 (( volatile LONG64 * )&Speed_Th_TPS);
	}

	for ( iCnt = 0; iCnt < dfTHREAD_ALLOC; iCnt++ )
	{
	 delete pDataArray[iCnt];
	}

	TESTEnd = true;

	return 0;
}




//////////////////////////////////////////////////////////////////////////////////////////////////////
//LF_Queue �׽�Ʈ�� InQueue������
//////////////////////////////////////////////////////////////////////////////////////////////////////
//������
unsigned int __stdcall LF_Queue_InQueue_Thread (void *pParam)
{
	/*------------------------------------------------------------------*/
	/////// ������ ť ��ť �׽�Ʈ //////////////////////////////////////////////

	// �������� �����忡�� ���ѹݺ� PUSH�� �ݺ������� ��
	// ��� �����ʹ� 0x0000000055555555 ���� �ʱ�ȭ �Ǿ� ����.

	//�� �����Ҵ� �����ڿ� ���� �۾�.
	// 0. Alloc (������� 10000 �� x 10 �� ���� ������ �� 10����)
	// 1. �ణ���
	//���ѹݺ�

	// �׽�Ʈ ����
	//
	// -	����� InQueue�� �Ǵ��� Ȯ��.
	/*------------------------------------------------------------------*/

	st_TEST_DATA *pDataArray;

	while ( 1 )
	{
		pDataArray = NULL;
		pDataArray = g_Mempool_LF->Alloc();
		if ( pDataArray == NULL )
		{
			CCrashDump::Crash ();
		}




		//�ʱ�ȭ
		pDataArray->Data = defData;
		pDataArray->Cnt = defCnt;




		//ť Maxġ ����.

		if ( g_LF_Queue->Enqueue (pDataArray) == false )
		{
			g_Mempool_LF->Free (pDataArray);
			Sleep (5);
			continue;
		}

		InterlockedIncrement64 (( volatile LONG64 * )&InQueue_Th_TPS);

		Sleep (InQueueSleep);

	}

	TESTEnd = true;

	return 0;
}





//////////////////////////////////////////////////////////////////////////////////////////////////////
//LF_Queue �׽�Ʈ�� DeQueue������
//////////////////////////////////////////////////////////////////////////////////////////////////////
//������
unsigned int __stdcall LF_Queue_DeQueue_Thread (void *pParam)
{
	/*------------------------------------------------------------------*/
	/////// ������ ť ��ť �׽�Ʈ //////////////////////////////////////////////

	// �������� �����忡�� ���ѹݺ� POP�� �ݺ������� ��
	// ��� �����ʹ� 0x0000000055555555 ���� �ʱ�ȭ �Ǿ� ����.

	//�� �����Ҵ� �����ڿ� ���� �۾�.
	// 0. Alloc (������� 10000 �� ���� ������ �� 10����)
	// 1. �ణ���
	//���ѹݺ�

	// �׽�Ʈ ����
	//
	// -	����� InQueue�� �Ǵ��� Ȯ��.
	/*------------------------------------------------------------------*/

	st_TEST_DATA *pDataArray;


	while ( 1 )
	{
		if ( g_LF_Queue->Dequeue (&pDataArray) == false )
		{
			//���� ������ ����. sleep���� ���� �ٽ� ����.
			Sleep (DeQueueSleep + DeQueueSleep);
			continue;
		}

		if ( pDataArray->Data != defData )
		{
			CCrashDump::Crash ();
		}
		if ( pDataArray->Cnt != defCnt )
		{
			CCrashDump::Crash ();
		}

		InterlockedIncrement64 (( volatile LONG64 * )&pDataArray->Data);
		InterlockedIncrement64 (( volatile LONG64 * )&pDataArray->Cnt);

		if ( pDataArray->Data != defData+1 )
		{
			CCrashDump::Crash ();
		}

		if ( pDataArray->Cnt != defCnt+1 )
		{
			CCrashDump::Crash ();
		}

		g_Mempool_LF->Free (pDataArray);

		InterlockedIncrement64 (( volatile LONG64 * )&DeQueue_Th_TPS);

		//Sleep (DeQueueSleep);

	}


	TESTEnd = true;

	return 0;
}