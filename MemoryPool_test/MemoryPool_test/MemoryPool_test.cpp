// MemoryPool_test.cpp : �ܼ� ���� ���α׷��� ���� �������� �����մϴ�.
//

#include "stdafx.h"
#include "lib\Library.h"
#include"MemoryPool.h"

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


#define dfTHREAD_ALLOC 10000
#define dfTHREAD_MAX 10
#define dfTESTLOOP_MAX 10000


CMemoryPool<st_TEST_DATA> *g_Mempool;
CMemoryPool_LF<st_TEST_DATA> *g_Mempool_LF;


LONG64 LF_MemPool_Th_TPS = 0;
LONG64 Speed_Th_TPS = 0;



void MemoryPoolTESTMain (void);
bool SpeedtestMain (void);

int main()
{
	g_Mempool = new CMemoryPool<st_TEST_DATA> (0);
	g_Mempool_LF = new CMemoryPool_LF<st_TEST_DATA> (0);

	HANDLE hThread[dfTHREAD_MAX];
	DWORD dwThreadID;

	int Cnt;

	while ( 1 )
	{
		wprintf (L"\n������ �׽�Ʈ ��� \n");
		wprintf (L"1.LF_MemPool \n");
		wprintf (L"2. \n");
		wprintf (L"3. \n");
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
			continue;
		case 3:
			continue;
		case 4:

			for ( int iCnt = 0; iCnt < 1; iCnt++ )
			{
				hThread[iCnt] = ( HANDLE )_beginthreadex (NULL, 0, SpeedTestThread, ( LPVOID )0, 0, ( unsigned int * )&dwThreadID);
			}
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
		case 4:
			EndFlag = SpeedtestMain ();
			break;
		}
		if ( EndFlag )
		{
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


bool SpeedtestMain (void)
{
	if ( Speed_Th_TPS == 0 )
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

		Sleep (2);

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

	for ( int MaxCnt = 0; MaxCnt < dfTESTLOOP_MAX * 10; MaxCnt++ )
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
	}

	return 0;
}