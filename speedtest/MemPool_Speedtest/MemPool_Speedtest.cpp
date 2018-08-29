// MemPool_Speedtest.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include "lib\Library.h"
#include"MemoryPool.h"
CCrashDump Dump;


#define ThreadMax 3
#define LoopMax 10000
#define AllocMax 10000

unsigned int WINAPI TestWorkerThread (LPVOID pVoid);

struct TESTStruct
{
	int a;
	int b;
};

CMemoryPool_LF<TESTStruct> *MemLF;
CMemoryPool_TLS<TESTStruct> *MemTLS;

INT64 LoopCnt;




int main()
{
	MemLF = new CMemoryPool_LF<TESTStruct> (0);
	MemTLS = new CMemoryPool_TLS<TESTStruct> (0);

	HANDLE hThread[ThreadMax];
	DWORD DWait;
	for ( int iCnt = 0; iCnt < ThreadMax; iCnt++ )
	{
		hThread[iCnt] = ( HANDLE )_beginthreadex (NULL, 0, TestWorkerThread, ( LPVOID )0, 0, NULL);
	}

	while ( 1 )
	{
		wprintf (L"\n");
		wprintf (L"MemPool_LF Full Size = %lld\n", MemLF->GetFullCount ());
		wprintf (L"MemPool_LF Use Size = %lld\n", MemLF->GetAllocCount ());
		wprintf (L"MemPool_TLS Full Size = %lld\n", MemTLS->GetFullCount ());
		wprintf (L"MemPool_TLS Use Size = %lld\n", MemTLS->GetAllocCount ());
		wprintf (L"Test Thread TPS = %lld\n", LoopCnt);
		wprintf (L"\n");
		
		LoopCnt = 0;
		
		
		DWait = WaitForMultipleObjects (ThreadMax, hThread, TRUE, 990);
		if ( DWait == WAIT_OBJECT_0 )
		{
			PROFILE_PRINT;
			break;
		}

	}
    return 0;
}

unsigned int WINAPI TestWorkerThread (LPVOID pVoid)
{
	TESTStruct *pArray[AllocMax];

	for ( int Cnt = LoopMax; Cnt > 0; Cnt-- )
	{
		//malloc
		for ( int AllocCnt = AllocMax-1; AllocCnt >= 0; AllocCnt-- )
		{
			PROFILE_BEGIN (L"malloc_Alloc");
			pArray[AllocCnt] = ( TESTStruct * )malloc (sizeof (TESTStruct));
			PROFILE_END (L"malloc_Alloc");
		}
		for ( int AllocCnt = AllocMax - 1; AllocCnt >= 0; AllocCnt-- )
		{
			PROFILE_BEGIN (L"malloc_Free");
			free (pArray[AllocCnt]);
			PROFILE_END (L"malloc_Free");
		}

		//new
		for ( int AllocCnt = AllocMax - 1; AllocCnt >= 0; AllocCnt-- )
		{
			PROFILE_BEGIN (L"new_Alloc");
			pArray[AllocCnt] = new TESTStruct;
			PROFILE_END (L"new_Alloc");
		}
		for ( int AllocCnt = AllocMax - 1; AllocCnt >= 0; AllocCnt-- )
		{
			PROFILE_BEGIN (L"new_Free");
			delete pArray[AllocCnt];
			PROFILE_END (L"new_Free");
		}



		//LF
		for ( int AllocCnt = AllocMax - 1; AllocCnt >= 0; AllocCnt-- )
		{
			PROFILE_BEGIN (L"MemPoolLF_Alloc");
			pArray[AllocCnt] = MemLF->Alloc ();
			PROFILE_END (L"MemPoolLF_Alloc");
		}
		for ( int AllocCnt = AllocMax - 1; AllocCnt >= 0; AllocCnt-- )
		{
			PROFILE_BEGIN (L"MemPoolLF_Free");
			MemLF->Free (pArray[AllocCnt]);
			PROFILE_END (L"MemPoolLF_Free");
		}



		//TLS
		for ( int AllocCnt = AllocMax - 1; AllocCnt >= 0; AllocCnt-- )
		{
			PROFILE_BEGIN (L"MemPoolTLS_Alloc");
			pArray[AllocCnt] = MemTLS->Alloc ();
			PROFILE_END (L"MemPoolTLS_Alloc");
		}
		for ( int AllocCnt = AllocMax - 1; AllocCnt >= 0; AllocCnt-- )
		{
			PROFILE_BEGIN (L"MemPoolTLS_Free");
			MemTLS->Free (pArray[AllocCnt]);
			PROFILE_END (L"MemPoolTLS_Free");
		}
		InterlockedIncrement64 (( volatile LONG64 * )&LoopCnt);
		Sleep (5);
	}


}