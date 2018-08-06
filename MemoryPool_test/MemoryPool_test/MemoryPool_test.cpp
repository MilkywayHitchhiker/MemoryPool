// MemoryPool_test.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include "lib\Library.h"
#include "MemoryPool.h"
#include "LockFreeStack.h"


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

//락프리 테스트용
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
	{
		hThread[iCnt] = ( HANDLE )_beginthreadex (NULL, 0, MemoryPoolThread, ( LPVOID )0, 0, ( unsigned int * )&dwThreadID);
	}


	while ( 1 )
	{
		Sleep (999);

		wprintf (L"LoopTPS = %lld", LOOPTPS);
		
		wprintf (L"\n");

		LOOPTPS = 0;
	}

	WaitForMultipleObjects (dfTHREAD_MAX, hThread, TRUE, INFINITE);

	return 0;
}



unsigned int __stdcall MemoryPoolThread (void *pParam)
{
	int iCnt;
	st_TEST_DATA *pDataArray[dfTHREAD_ALLOC];

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


	//락 프리 버전 메모리풀 테스트


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

		Sleep (1);

		for ( iCnt = 0; iCnt < dfTHREAD_ALLOC; iCnt++ )
		{


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
		//Malloc 속도 테스트
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




		//New 속도 테스트

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



		//LOCK버전 메모리풀 속도 테스트


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




		//TLS버전 메모리풀 테스트

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





		//락 프리 버전 메모리풀 테스트


		for ( iCnt = 0; iCnt < dfTHREAD_ALLOC; iCnt++ )
		{
			PROFILE_BEGIN (L"LF Alloc");

			pDataArray[iCnt] = ( st_TEST_DATA * )g_Mempool_LF->Alloc ();

			PROFILE_END (L"LF Alloc");

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

		for ( iCnt = 0; iCnt < dfTHREAD_ALLOC; iCnt++ )
		{
			PROFILE_BEGIN (L"LF Free");

			g_Mempool_LF->Free (pDataArray[iCnt]);

			PROFILE_END (L"LF Free");
		}

		*/
	}

	return 0;
}


