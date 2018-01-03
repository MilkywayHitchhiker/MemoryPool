#pragma once
#include <Windows.h>

struct T
{
	int Data;
};


//락프리 버전 메모리풀
class CMemPool_LF
{
#define SafeLine 0x79876C288
private :
	struct Node
	{
		__int64 SafeHead;
		T Data;
		__int64 SafeTail;

		Node *pNext;
	};
	struct Head
	{
		Node *Head;
		__int64 Cnt;
	};

	Head Top;
	int _NodeCnt;
	int _MaxCnt;
	bool _PlacementNew;
	unsigned __int64 UniqueCnt;

public :
	//생성자
	CMemPool_LF (bool Placementnew = false, int MaxCnt = 0)
	{
		Top.Head = NULL;
		Top.Cnt = 0;
		_PlacementNew = Placementnew;
		_MaxCnt = MaxCnt;
		_NodeCnt = 0;
		UniqueCnt = 0;
	}

	//파괴자
	~CMemPool_LF (void)
	{

	}

	T Alloc(void)
	{
		Head buff;

		do
		{
			buff.Head = Top.Head;
			buff.Cnt = Top.Cnt;

		} while ( InterlockedCompareExchange128(,,,) );
	}
};