// MemoryPool_test.cpp : �ܼ� ���� ���α׷��� ���� �������� �����մϴ�.
//

#include "stdafx.h"
#include "lib\Library.h"
#include "MemoryPool.h"
#include "LockFreeStack.h"

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

<<<<<<< HEAD


#define dfTHREAD_ALLOC 10000
#define dfTHREAD_MAX 6
#define dfTESTLOOP_MAX 10000

CMemoryPool_LF<st_TEST_DATA> *g_Mempool_LF;


LONG64 LF_MemPool_Th_TPS = 0;

void MemoryPoolTESTMain (void);

int main()
{
	g_Mempool_LF = new CMemoryPool_LF<st_TEST_DATA> (0);
	int Cnt;
	while ( 1 )
=======
//������ �׽�Ʈ��
//=======================
struct st_TEST_DATA
{
	unsigned int	lData;
	unsigned int	lCount;

	st_TEST_DATA(void)
	{
		lData = 0x0000000055555555;
		lCount = 0;
	}
};

#define dfTHREAD_ALLOC 10000
#define dfTHREAD_MAX 20
#define dfTESTLOOP_MAX 10000

CStack_LF<st_TEST_DATA *> LF_Stack;

LONG64 LOOPTPS = 0;

int main()
{
	LOG_DIRECTORY (L"LOG");
	LOG_LEVEL (LOG_DEBUG, true);

	HANDLE hThread[dfTHREAD_MAX];
	DWORD dwThreadID;
	int Cnt=1;
	
	for ( int iCnt = 0; iCnt < dfTHREAD_MAX; iCnt++ )
>>>>>>> fa8a10bd36bd3a041b471dba7ef9559ac196e531
	{
		wprintf (L"\n������ �׽�Ʈ ��� \n");
		wprintf (L"1.LF_MemPool \n");
		wprintf (L"2. \n");
		wprintf (L"3. \n");
		wscanf_s (L"%d",&Cnt);

		switch ( Cnt )
		{
		case 1:
			HANDLE hThread[dfTHREAD_MAX];
			DWORD dwThreadID;

			for ( int iCnt = 0; iCnt < dfTHREAD_MAX; iCnt++ )
			{
				hThread[iCnt] = ( HANDLE )_beginthreadex (NULL, 0, MemoryPoolThread, ( LPVOID )0, 0, ( unsigned int * )&dwThreadID);
			}

			break;
		case 2:
		case 3:
		default:
			continue;
		}
		break;
	}

<<<<<<< HEAD
=======

>>>>>>> fa8a10bd36bd3a041b471dba7ef9559ac196e531
	while ( 1 )
	{
		Sleep (999);

<<<<<<< HEAD
		switch ( Cnt )
		{
		case 1:
			MemoryPoolTESTMain ();
			break;
		}

	}
=======
		wprintf (L"LoopTPS = %lld", LOOPTPS);
		
		wprintf (L"\n");

		LOOPTPS = 0;
	}

	WaitForMultipleObjects (dfTHREAD_MAX, hThread, TRUE, INFINITE);
>>>>>>> fa8a10bd36bd3a041b471dba7ef9559ac196e531

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
<<<<<<< HEAD


	while ( 1 )
	{
=======


	//�� ���� ���� �޸�Ǯ �׽�Ʈ


	for ( iCnt = 0; iCnt < dfTHREAD_ALLOC; iCnt++ )
	{

		pDataArray[iCnt] = new st_TEST_DATA;
		LF_Stack.Push (pDataArray[iCnt]);
	}


	while ( 1 )
	{
		for ( iCnt = 0; iCnt < dfTHREAD_ALLOC; iCnt++ )
		{
			LF_Stack.Pop (&pDataArray[iCnt]);
		}

		Sleep (1);

		for ( iCnt = 0; iCnt < dfTHREAD_ALLOC; iCnt++ )
		{

			if ( pDataArray[iCnt]->lData != 0x0000000055555555 )
			{
				LOG_LOG (L"LFStack",LOG_WARNING,L"Pop Data Error %d\n", pDataArray[iCnt]->lData);
				CCrashDump::Crash ();
			}

			if ( pDataArray[iCnt]->lCount != 0 )
			{
				LOG_LOG (L"LFStack", LOG_WARNING, L"Pop Count Error %d\n", pDataArray[iCnt]->lCount);
				CCrashDump::Crash ();
			}
		}


		for ( iCnt = 0; iCnt < dfTHREAD_ALLOC; iCnt++ )
		{
			InterlockedIncrement(&pDataArray[iCnt]->lCount);
			InterlockedIncrement (&pDataArray[iCnt]->lData);
		}
>>>>>>> fa8a10bd36bd3a041b471dba7ef9559ac196e531


		for ( iCnt = 0; iCnt < dfTHREAD_ALLOC; iCnt++ )
		{
<<<<<<< HEAD
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

=======


			if ( pDataArray[iCnt]->lData != 0x0000000055555556 )
			{
				LOG_LOG (L"LFStack", LOG_WARNING, L"Add Data Error %d\n", pDataArray[iCnt]->lData);
				CCrashDump::Crash ();
			}

			if ( pDataArray[iCnt]->lCount != 1 )
			{
				LOG_LOG (L"LFStack", LOG_WARNING, L"Add Count Error %d\n", pDataArray[iCnt]->lCount);
				CCrashDump::Crash ();
			}
		}

		for ( iCnt = 0; iCnt < dfTHREAD_ALLOC; iCnt++ )
		{
			InterlockedDecrement (&pDataArray[iCnt]->lCount);
			InterlockedDecrement (&pDataArray[iCnt]->lData);
		}



		for ( iCnt = 0; iCnt < dfTHREAD_ALLOC; iCnt++ )
		{
			LF_Stack.Push (pDataArray[iCnt]);
		}

		Sleep (1);

		InterlockedIncrement64 ((volatile LONG64 *)&LOOPTPS);


	}

	for ( iCnt = 0; iCnt < dfTHREAD_ALLOC; iCnt++ )
	{
		LF_Stack.Pop (&pDataArray[iCnt]);
		delete pDataArray[iCnt];
	}

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


>>>>>>> fa8a10bd36bd3a041b471dba7ef9559ac196e531


		for ( iCnt = 0; iCnt < dfTHREAD_ALLOC; iCnt++ )
		{
			InterlockedIncrement64 ((volatile LONG64 *)&pDataArray[iCnt]->Data);
			InterlockedIncrement64 (( volatile LONG64 * )&pDataArray[iCnt]->Cnt);
		}

		Sleep (2);

		for ( iCnt = 0; iCnt < dfTHREAD_ALLOC; iCnt++ )
		{
			if ( pDataArray[iCnt]->Data != defData + 1 || pDataArray[iCnt]->Cnt != defCnt + 1 )
				CCrashDump::Crash ();
		}
<<<<<<< HEAD
=======


>>>>>>> fa8a10bd36bd3a041b471dba7ef9559ac196e531


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

<<<<<<< HEAD
=======




		//�� ���� ���� �޸�Ǯ �׽�Ʈ


>>>>>>> fa8a10bd36bd3a041b471dba7ef9559ac196e531
		for ( iCnt = 0; iCnt < dfTHREAD_ALLOC; iCnt++ )
		{
			if ( g_Mempool_LF->Free (pDataArray[iCnt]) == false )
			{
				CCrashDump::Crash ();
			}
		}

		for ( iCnt = 0; iCnt < dfTHREAD_ALLOC; iCnt++ )
		{
			pDataArray[iCnt]->lCount += 1;
			pDataArray[iCnt]->lData += 10;
		}


		for ( iCnt = 0; iCnt < dfTHREAD_ALLOC; iCnt++ )
		{
			if ( pDataArray[iCnt]->lCount != 1 )
			{
				CCrashDump::Crash ();
			}

			if ( pDataArray[iCnt]->lData != 10 )
			{
				CCrashDump::Crash ();
			}
		}

		Sleep (10);

<<<<<<< HEAD
		InterlockedIncrement64 (( volatile LONG64 * )&LF_MemPool_Th_TPS);
=======
		for ( iCnt = 0; iCnt < dfTHREAD_ALLOC; iCnt++ )
		{
			PROFILE_BEGIN (L"LF Free");

			g_Mempool_LF->Free (pDataArray[iCnt]);

			PROFILE_END (L"LF Free");
		}
>>>>>>> fa8a10bd36bd3a041b471dba7ef9559ac196e531

		*/
	}


	return 0;
}


