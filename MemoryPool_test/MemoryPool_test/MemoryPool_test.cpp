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

	st_TEST_DATA(void)
	{
		lData = 0;
		lCount = 0;
	}
	~st_TEST_DATA (void)
	{
		lData = 0;
		lCount = 0;
	}
};

#define dfTHREAD_ALLOC 10000
#define dfTHREAD_MAX 1
#define dfTESTLOOP_MAX 10000

CMemoryPool<st_TEST_DATA> *g_Mempool;
CMemoryPool_LF<st_TEST_DATA> *g_Mempool_LF;
CMemoryPool_TLS<st_TEST_DATA> *g_Mempool_TLS;

LONG64 lAllocTPS = 0;
LONG64 lFreeTPS = 0;

int main()
{
	g_Mempool = new CMemoryPool<st_TEST_DATA> (0);
	g_Mempool_LF = new CMemoryPool_LF<st_TEST_DATA> (0);
	g_Mempool_TLS = new CMemoryPool_TLS<st_TEST_DATA> (0);

	HANDLE hThread[dfTHREAD_MAX];
	DWORD dwThreadID;
	int Cnt=1;
	
	for ( int iCnt = 0; iCnt < dfTHREAD_MAX; iCnt++ )
	{
		hThread[iCnt] = ( HANDLE )_beginthreadex (NULL, 0, MemoryPoolThread, ( LPVOID )0, 0, ( unsigned int * )&dwThreadID);
	}
	/*
	while ( 1 )
	{
		Sleep (999);

		for ( int ForCnt = 0; ForCnt < Cnt; ForCnt++ )
		{
			wprintf (L"-");
		}
		wprintf (L"\n");
		Cnt++;
	}
	*/

	WaitForMultipleObjects (dfTHREAD_MAX, hThread, TRUE, INFINITE);
	PROFILE_PRINT;

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
		/*
		//Malloc �ӵ� �׽�Ʈ
		for ( iCnt = 0; iCnt < dfTHREAD_ALLOC; iCnt++ )
		{

			PROFILE_BEGIN (L"Malloc Alloc");
			pDataArray[iCnt] = ( st_TEST_DATA * )malloc (sizeof (st_TEST_DATA));

			PROFILE_END (L"Malloc Alloc");
		}

		for ( iCnt = 0; iCnt < dfTHREAD_ALLOC; iCnt++ )
		{

			PROFILE_BEGIN (L"Malloc Free");
			free (pDataArray[iCnt]);

			PROFILE_END (L"Malloc Free");
		}




		//New �ӵ� �׽�Ʈ

		for ( iCnt = 0; iCnt < dfTHREAD_ALLOC; iCnt++ )
		{
			PROFILE_BEGIN (L"New Alloc");
			pDataArray[iCnt] = new st_TEST_DATA;

			PROFILE_END (L"New Alloc");
		}


		for ( iCnt = 0; iCnt < dfTHREAD_ALLOC; iCnt++ )
		{
			PROFILE_BEGIN (L"New Free");

			delete pDataArray[iCnt];
			PROFILE_END (L"New Free");

		}



		//LOCK���� �޸�Ǯ �ӵ� �׽�Ʈ


		for ( iCnt = 0; iCnt < dfTHREAD_ALLOC; iCnt++ )
		{
			PROFILE_BEGIN (L"LOCK Alloc");

			pDataArray[iCnt] = ( st_TEST_DATA * )g_Mempool->Alloc ();

			PROFILE_END (L"LOCK Alloc");
		}

		for ( iCnt = 0; iCnt < dfTHREAD_ALLOC; iCnt++ )
		{
			PROFILE_BEGIN (L"LOCK Free");

			g_Mempool->Free (pDataArray[iCnt]);

			PROFILE_END (L"LOCK Free");
		}




		//TLS���� �޸�Ǯ �׽�Ʈ

		for ( iCnt = 0; iCnt < dfTHREAD_ALLOC; iCnt++ )
		{
			PROFILE_BEGIN (L"TLS Alloc");

			pDataArray[iCnt] = ( st_TEST_DATA * )g_Mempool_TLS->Alloc ();

			PROFILE_END (L"TLS Alloc");
		}


		for ( iCnt = 0; iCnt < dfTHREAD_ALLOC; iCnt++ )
		{
			PROFILE_BEGIN (L"TLS Free");

			g_Mempool_TLS->Free (pDataArray[iCnt]);

			PROFILE_END (L"TLS Free");
		}

		*/



		//�� ���� ���� �޸�Ǯ �׽�Ʈ


		for ( iCnt = 0; iCnt < dfTHREAD_ALLOC; iCnt++ )
		{
			PROFILE_BEGIN (L"LF Alloc");

			pDataArray[iCnt] = ( st_TEST_DATA * )g_Mempool_LF->Alloc ();

			PROFILE_END (L"LF Alloc");

		}


		for ( iCnt = 0; iCnt < dfTHREAD_ALLOC; iCnt++ )
		{
			PROFILE_BEGIN (L"LF Free");

			g_Mempool_LF->Free (pDataArray[iCnt]);

			PROFILE_END (L"LF Free");
		}


	}

	return 0;
}


