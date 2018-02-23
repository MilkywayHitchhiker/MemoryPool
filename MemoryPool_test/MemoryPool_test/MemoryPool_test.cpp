// MemoryPool_test.cpp : �ܼ� ���� ���α׷��� ���� �������� �����մϴ�.
//

#include "stdafx.h"
#include "lib\Library.h"
#include"MemoryPool.h"


class abc
{
public:
	int Test;

	abc (void)
	{
		Test = 77;
	}
	~abc (void)
	{
		Test = 0;
	}
};


//������ �׽�Ʈ��
//=======================
struct st_TEST_DATA
{
	volatile LONG64	lData;
	volatile LONG64	lCount;
};

#define dfTHREAD_ALLOC 10000
#define dfTHREAD_MAX 10

Hitchhiker::CMemoryPool_LockFree<st_TEST_DATA> *g_Mempool;

LONG64 lAllocTPS = 0;
LONG64 lFreeTPS = 0;

LONG64 lAllocCounter = 0;
LONG64 lFreeCounter = 0;
//=======================

int main()
{

	// not Thread Safe Memory pool
/*
	Hitchhiker::CMemoryPool<abc> MemPool (0);

	abc *p[1000];

	for ( int cnt = 0; cnt < 1000; cnt++ )
	{
		p[cnt] = MemPool.Alloc ();
		printf ("%d : %d \n", cnt, p[cnt]->Test);
	}

	printf ("Alloc : %d \n", MemPool.GetMemoryPoolFullCount ());
	printf ("Use : %d \n", MemPool.GetUseCount ());
	printf ("Free : %d \n", MemPool.GetFreeCount());


	printf ("\n\n�Ҵ� �Ϻ� ����\n\n");
	for ( int cnt = 0; cnt < 700; cnt++ )
	{
		MemPool.Free (p[cnt]);
	}
	printf ("Alloc : %d \n", MemPool.GetMemoryPoolFullCount ());
	printf ("Use : %d \n", MemPool.GetUseCount ());
	printf ("Free : %d \n", MemPool.GetFreeCount ());

	return 0;
	*/

	g_Mempool = new Hitchhiker::CMemoryPool_LockFree<st_TEST_DATA> (0);

	HANDLE hThread[dfTHREAD_MAX];
	DWORD dwThreadID;

	for ( int iCnt = 0; iCnt < dfTHREAD_MAX; iCnt++ )
	{
		hThread[iCnt] = ( HANDLE )_beginthreadex (
			NULL,
			0,
			MemoryPoolThread,
			( LPVOID )0,
			0,
			( unsigned int * )&dwThreadID
		);
	}

	while ( 1 )
	{
		lAllocTPS = lAllocCounter;
		lFreeTPS = lFreeCounter;

		lAllocCounter = 0;
		lFreeCounter = 0;

		wprintf (L"----------------------------------------------------\n");
		wprintf (L"Alloc TPS		: %d\n", lAllocTPS);
		wprintf (L"Free TPS		: %d\n", lFreeTPS);
		wprintf (L"Memory Block Count	: %d\n", ( LONG64 )g_Mempool->GetBlockCount ());
		wprintf (L"----------------------------------------------------\n\n");

		Sleep (999);
	}

}


/*------------------------------------------------------------------*/
// 0. �� �����忡�� st_QUEUE_DATA �����͸� ���� ��ġ (10000��) ����		
// 0. ������ ����(Ȯ��)
// 1. iData = 0x0000000055555555 ����
// 1. lCount = 0 ����
// 2. ���ÿ� ����

// 3. �ణ���  Sleep (0 ~ 3)
// 4. ���� ���� ������ �� ��ŭ ���� 
// 4. - �̶� �����°� ���� ���� �������� ����, �ٸ� �����尡 ���� �������� ���� ����
// 5. ���� ��ü �����Ͱ� �ʱⰪ�� �´��� Ȯ��. (�����͸� ���� ����ϴ��� Ȯ��)
// 6. ���� ��ü �����Ϳ� ���� lCount Interlock + 1
// 6. ���� ��ü �����Ϳ� ���� iData Interlock + 1
// 7. �ణ���
// 8. + 1 �� �����Ͱ� ��ȿ���� Ȯ�� (���� �����͸� ���� ����ϴ��� Ȯ��)
// 9. ������ �ʱ�ȭ (0x0000000055555555, 0)
// 10. ���� �� ��ŭ ���ÿ� �ٽ� ����
//  3�� ���� �ݺ�.
/*------------------------------------------------------------------*/
unsigned __stdcall MemoryPoolThread (void *pParam)
{
	int iCnt;
	st_TEST_DATA *pDataArray[dfTHREAD_ALLOC];

	while ( 1 )
	{
		for ( iCnt = 0; iCnt < dfTHREAD_ALLOC; iCnt++ )
		{
			pDataArray[iCnt] = ( st_TEST_DATA * )g_Mempool->Alloc ();
			pDataArray[iCnt]->lData = 0x0000000055555555;
			pDataArray[iCnt]->lCount = 0;
			InterlockedIncrement64 (( LONG64 * )&lAllocCounter);
		}

		for ( iCnt = 0; iCnt < dfTHREAD_ALLOC; iCnt++ )
		{
			if ( pDataArray[iCnt]->lData != 0x0000000055555555 || pDataArray[iCnt]->lCount != 0 )
				CCrashDump::Crash ();
		}

		Sleep (1);

		for ( iCnt = 0; iCnt < dfTHREAD_ALLOC; iCnt++ )
		{
			InterlockedIncrement64 (&pDataArray[iCnt]->lCount);
			InterlockedIncrement64 (&pDataArray[iCnt]->lData);
		}

		Sleep (1);

		for ( iCnt = 0; iCnt < dfTHREAD_ALLOC; iCnt++ )
		{
			if ( pDataArray[iCnt]->lData != 0x0000000055555556 || pDataArray[iCnt]->lCount != 1 )
				CCrashDump::Crash ();
		}

		Sleep (1);

		for ( iCnt = 0; iCnt < dfTHREAD_ALLOC; iCnt++ )
		{
			InterlockedDecrement64 (&pDataArray[iCnt]->lCount);
			InterlockedDecrement64 (&pDataArray[iCnt]->lData);
		}

		for ( iCnt = 0; iCnt < dfTHREAD_ALLOC; iCnt++ )
		{
			if ( pDataArray[iCnt]->lData != 0x0000000055555555 || pDataArray[iCnt]->lCount != 0 )
				CCrashDump::Crash ();
		}

		for ( iCnt = 0; iCnt < dfTHREAD_ALLOC; iCnt++ )
		{
			g_Mempool->Free (pDataArray[iCnt]);
			InterlockedIncrement64 (( LONG64 * )&lFreeCounter);
		}
	}
}