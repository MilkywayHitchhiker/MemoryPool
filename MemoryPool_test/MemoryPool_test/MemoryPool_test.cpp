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

unsigned int __stdcall MemoryPoolThread (void *pParam);

//������ �׽�Ʈ��
//=======================
struct st_TEST_DATA
{
	volatile LONG64	lData;
	volatile LONG64	lCount;
};

#define dfTHREAD_ALLOC 10000
#define dfTHREAD_MAX 4
#define dfTESTLOOP_MAX 100

CMemoryPool<st_TEST_DATA> *g_Mempool;
CMemoryPool_LF<st_TEST_DATA> *g_Mempool_LF;
CMemoryPool_TLS<st_TEST_DATA> *g_Mempool_TLS;

LONG64 lAllocTPS = 0;
LONG64 lFreeTPS = 0;


//=======================

int main()
{
	g_Mempool = new CMemoryPool<st_TEST_DATA> (0);
	g_Mempool_LF = new CMemoryPool_LF<st_TEST_DATA> (0);
	g_Mempool_TLS = new CMemoryPool_TLS<st_TEST_DATA> (0);

	HANDLE hThread[dfTHREAD_MAX];
	DWORD dwThreadID;

	int lAllocCount;
	int lFreeCount;
	int IFullCount;

	
	for ( int iCnt = 0; iCnt < dfTHREAD_MAX; iCnt++ )
	{
		hThread[iCnt] = ( HANDLE )_beginthreadex (NULL, 0, MemoryPoolThread, ( LPVOID )0, 0, ( unsigned int * )&dwThreadID);
	}
	/*
	while ( 1 )
	{
		lAllocCount = g_Mempool->GetAllocCount();
		lFreeCount = g_Mempool->GetFreeCount();
		IFullCount = g_Mempool->GetFullCount ();

		wprintf (L"----------------------------------------------------\n");
		wprintf (L"Alloc TPS		: %lld\n", lAllocTPS);
		wprintf (L"Free TPS		: %lld\n", lFreeTPS);
		wprintf (L"Memory Pool Alloc	: %d\n", lAllocCount);
		wprintf (L"Memory Pool Free	: %d\n", lFreeCount);
		wprintf (L"Memory Pool Full	: %d\n", IFullCount);
		wprintf (L"----------------------------------------------------\n\n");

		lAllocTPS = 0;
		lFreeTPS = 0;

		Sleep (999);
	}
	*/
	WaitForMultipleObjects (dfTHREAD_MAX, hThread, TRUE, INFINITE);

	PROFILE_KEYPROC;
	return 0;
}



unsigned int __stdcall MemoryPoolThread (void *pParam)
{
	int iCnt;
	st_TEST_DATA *pDataArray[dfTHREAD_ALLOC];

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
	/*
	while ( 1 )
	{
		for ( iCnt = 0; iCnt < dfTHREAD_ALLOC; iCnt++ )
		{
			pDataArray[iCnt] = ( st_TEST_DATA * )g_Mempool->Alloc ();
			pDataArray[iCnt]->lData = 0x0000000055555555;
			pDataArray[iCnt]->lCount = 0;
			InterlockedIncrement64 (( LONG64 * )&lAllocTPS);
		}

		for ( iCnt = 0; iCnt < dfTHREAD_ALLOC; iCnt++ )
		{
			if ( pDataArray[iCnt]->lData != 0x0000000055555555 || pDataArray[iCnt]->lCount != 0 )
				CCrashDump::Crash ();
		}

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
			InterlockedIncrement64 (( LONG64 * )&lFreeTPS);
		}
		Sleep (2);



	}
	*/
	for ( int Cnt = 0; Cnt < dfTESTLOOP_MAX; Cnt++ )
	{

		//Malloc �ӵ� �׽�Ʈ
		PROFILE_BEGIN (L"Malloc Alloc");
		for ( iCnt = 0; iCnt < dfTHREAD_ALLOC; iCnt++ )
		{
			pDataArray[iCnt] = ( st_TEST_DATA * )malloc (sizeof (st_TEST_DATA));
		}
		PROFILE_END (L"Malloc Alloc");


		PROFILE_BEGIN (L"Malloc Free");
		for ( iCnt = 0; iCnt < dfTHREAD_ALLOC; iCnt++ )
		{
			free (pDataArray[iCnt]);
		}

		PROFILE_END (L"Malloc Free");


		//New �ӵ� �׽�Ʈ

		PROFILE_BEGIN (L"New Alloc");
		for ( iCnt = 0; iCnt < dfTHREAD_ALLOC; iCnt++ )
		{
			pDataArray[iCnt] = new st_TEST_DATA;
		}			
		PROFILE_END (L"New Alloc");

		PROFILE_BEGIN (L"New Free");
		for ( iCnt = 0; iCnt < dfTHREAD_ALLOC; iCnt++ )
		{

			delete pDataArray[iCnt];

		}
		PROFILE_END (L"New Free");



		//LOCK���� �޸�Ǯ �ӵ� �׽�Ʈ
		PROFILE_BEGIN (L"LOCK Alloc");
		for ( iCnt = 0; iCnt < dfTHREAD_ALLOC; iCnt++ )
		{

			pDataArray[iCnt] = ( st_TEST_DATA * )g_Mempool->Alloc ();

		}
		PROFILE_END (L"LOCK Alloc");


		PROFILE_BEGIN (L"LOCK Free");
		for ( iCnt = 0; iCnt < dfTHREAD_ALLOC; iCnt++ )
		{

			g_Mempool->Free (pDataArray[iCnt]);

		}
		PROFILE_END (L"LOCK Free");


		//�� ���� ���� �޸�Ǯ �׽�Ʈ
		PROFILE_END (L"LOCK Free");
		for ( iCnt = 0; iCnt < dfTHREAD_ALLOC; iCnt++ )
		{
			pDataArray[iCnt] = ( st_TEST_DATA * )g_Mempool_LF->Alloc ();

		}
		PROFILE_END (L"LF Alloc");

		PROFILE_BEGIN (L"LF Free");
		for ( iCnt = 0; iCnt < dfTHREAD_ALLOC; iCnt++ )
		{

			g_Mempool_LF->Free (pDataArray[iCnt]);

		}
		PROFILE_END (L"LF Free");


		PROFILE_BEGIN (L"TLS Alloc");
		//TLS���� �޸�Ǯ �׽�Ʈ
		for ( iCnt = 0; iCnt < dfTHREAD_ALLOC; iCnt++ )
		{

			pDataArray[iCnt] = ( st_TEST_DATA * )g_Mempool_TLS->Alloc ();

		}
		PROFILE_END (L"TLS Alloc");


		PROFILE_BEGIN (L"TLS Free");
		for ( iCnt = 0; iCnt < dfTHREAD_ALLOC; iCnt++ )
		{

			g_Mempool_TLS->Free (pDataArray[iCnt]);

		}
		PROFILE_END (L"TLS Free");
	}

	return 0;
}


