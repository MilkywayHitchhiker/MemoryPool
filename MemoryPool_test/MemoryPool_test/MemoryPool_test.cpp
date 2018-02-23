// MemoryPool_test.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
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


//락프리 테스트용
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


	printf ("\n\n할당 일부 해제\n\n");
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
// 0. 각 스레드에서 st_QUEUE_DATA 데이터를 일정 수치 (10000개) 생성		
// 0. 데이터 생성(확보)
// 1. iData = 0x0000000055555555 셋팅
// 1. lCount = 0 셋팅
// 2. 스택에 넣음

// 3. 약간대기  Sleep (0 ~ 3)
// 4. 내가 넣은 데이터 수 만큼 뽑음 
// 4. - 이때 뽑히는건 내가 넣은 데이터일 수도, 다른 스레드가 넣은 데이터일 수도 있음
// 5. 뽑은 전체 데이터가 초기값과 맞는지 확인. (데이터를 누가 사용하는지 확인)
// 6. 뽑은 전체 데이터에 대해 lCount Interlock + 1
// 6. 뽑은 전체 데이터에 대해 iData Interlock + 1
// 7. 약간대기
// 8. + 1 한 데이터가 유효한지 확인 (뽑은 데이터를 누가 사용하는지 확인)
// 9. 데이터 초기화 (0x0000000055555555, 0)
// 10. 뽑은 수 만큼 스택에 다시 넣음
//  3번 으로 반복.
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