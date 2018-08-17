// MemoryPool_test.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include "lib\Library.h"
#include"MemoryPool.h"
#include"LockFreeStack.h"

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

#define dfTHREAD_ALLOC 10000
#define dfTHREAD_MAX 10
#define dfTESTLOOP_MAX 1000


CMemoryPool<st_TEST_DATA> *g_Mempool;
CMemoryPool_LF<st_TEST_DATA> *g_Mempool_LF;
CStack_LF<st_TEST_DATA *> *g_LF_Stack;


LONG64 LF_MemPool_Th_TPS = 0;
LONG64 Speed_Th_TPS = 0;
bool TESTEnd;


void MemoryPoolTESTMain (void);
void LF_Stack_TESTMain (void);
bool SpeedtestMain (void);
int main()
{
	
	g_Mempool = new CMemoryPool<st_TEST_DATA> (0);
	g_Mempool_LF = new CMemoryPool_LF<st_TEST_DATA> (0);
	g_LF_Stack = new CStack_LF<st_TEST_DATA *> ();

	LOG_DIRECTORY (L"LOG");
	LOG_LEVEL (LOG_DEBUG,false);
	LOG_LOG (L"main", LOG_SYSTEM, L"Start");

	HANDLE hThread[dfTHREAD_MAX];
	DWORD dwThreadID;

	int Cnt;

	while ( 1 )
	{
		wprintf (L"\n락프리 테스트 모듈 \n");
		wprintf (L"1.LF_MemPool \n");
		wprintf (L"2.LF_Stack \n");
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
			for ( int iCnt = 0; iCnt < dfTHREAD_MAX; iCnt++ )
			{
				hThread[iCnt] = ( HANDLE )_beginthreadex (NULL, 0, LF_StackTestThread, ( LPVOID )0, 0, ( unsigned int * )&dwThreadID);
			}

			break;
			continue;
		case 3:
			continue;
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
//메모리풀 테스트 스레드
//////////////////////////////////////////////////////////////////////////////////////////////////////

unsigned int __stdcall MemoryPoolThread (void *pParam)
{
	int iCnt;
	st_TEST_DATA *pDataArray[dfTHREAD_ALLOC];

	/*------------------------------------------------------------------*/
	///////  메모리풀 테스트 툴  //////////////////////////////////////////////////////////////////////////////////////////

	// 여러개의 스레드에서 일정수량의 Alloc 과 Free 를 반복적으로 함
	// 모든 데이터는 0x0000000055555555 으로 초기화 되어 있음.

	// 0. Alloc (스레드당 10000 개 x 10 개 스레드 총 10만개)
	// 1. 0x0000000055555555 이 맞는지 확인.
	// 2. Interlocked + 1 (Data + 1 / Count + 1)
	// 3. 약간대기
	// 4. 여전히 0x0000000055555556 이 맞는지 (Count == 1) 확인.
	// 5. Interlocked - 1 (Data - 1 / Count - 1)
	// 6. 약간대기
	// 7. 0x0000000055555555 이 맞는지 (Count == 0) 확인.
	// 8. Free
	// 반복.

	// 테스트 목적
	//
	// - 할당된 메모리를 또 할당 하는가 ?
	// - 잘못된 메모리를 할당 하는가 ?
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
		Sleep (2);

		InterlockedIncrement64 (( volatile LONG64 * )&LF_MemPool_Th_TPS);

	}


	return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////
//malloc,new.MemPool 속도 테스트용 스레드
//////////////////////////////////////////////////////////////////////////////////////////////////////

unsigned int __stdcall SpeedTestThread (void *pParam)
{
	/*------------------------------------------------------------------*/
	/////// 속도 테스트 툴  //////////////////////////////////////////////

	// 여러개의 스레드에서 일정수량의 Alloc 과 Free 를 반복적으로 함
	// 모든 데이터는 0x0000000055555555 으로 초기화 되어 있음.

	//각 동적할당 연산자에 대한 작업.
	// 0. Alloc (스레드당 10000 개 x 10 개 단일 스레드 총 10만개)
	// 1. 약간대기
	// 2. Free
	// LoopMax만큼 반복함.

	// 테스트 목적
	//
	// - 작성된 메모리풀과의 속도 테스트.
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
		//LOCK버전 메모리풀
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
		//LF버전 메모리풀
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
//LF_Stack 속도 테스트용 스레드
//////////////////////////////////////////////////////////////////////////////////////////////////////

unsigned int __stdcall LF_StackTestThread (void *pParam)
{
	/*------------------------------------------------------------------*/
	/////// 락프리 스택 테스트 //////////////////////////////////////////////

	// 여러개의 스레드에서 일정수량의 PUSH 와 POP을 반복적으로 함
	// 모든 데이터는 0x0000000055555555 으로 초기화 되어 있음.

	//각 동적할당 연산자에 대한 작업.
	// 0. Alloc (스레드당 10000 개 x 10 개 단일 스레드 총 10만개)
	// 1. 약간대기
	// 2. Free
	// LoopMax만큼 반복함.

	// 테스트 목적
	//
	// - 작성된 메모리풀과의 속도 테스트.
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
	//초기화

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
		//초기값 맞는지 체크.
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
		//interlocked로 증가.
		for ( iCnt = 0; iCnt < dfTHREAD_ALLOC; iCnt++ )
		{
			InterlockedIncrement64 (( volatile LONG64 * )&pDataArray[iCnt]->Data);
			InterlockedIncrement64 (( volatile LONG64 * )&pDataArray[iCnt]->Cnt);
		}

		Sleep (10);

		//================================================================
		//interlocked로 증가된값이 맞는지 체크.
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
		//interlocked로 초기값으로 감소
		for ( iCnt = 0; iCnt < dfTHREAD_ALLOC; iCnt++ )
		{
			InterlockedDecrement64 (( volatile LONG64 * )&pDataArray[iCnt]->Data);
			InterlockedDecrement64 (( volatile LONG64 * )&pDataArray[iCnt]->Cnt);
		}

	//	Sleep (10);

		//================================================================
		//interlocked로 감소된 값이 맞는지 체크.
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