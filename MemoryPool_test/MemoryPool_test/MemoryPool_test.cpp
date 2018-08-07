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

	while ( 1 )
	{
		Sleep (999);

		switch ( Cnt )
		{
		case 1:
			MemoryPoolTESTMain ();
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


