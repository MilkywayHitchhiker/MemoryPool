#pragma once
#include "MemoryPool.h"

template <class DATA>
class CStack_LF
{
public:

	struct st_NODE
	{
		DATA	Data;
		st_NODE *pNext;
	};

	struct st_TOP_NODE
	{
		st_NODE *pTopNode;
		__int64 iUniqueNum;
	};

public:

	/////////////////////////////////////////////////////////////////////////
	// ������
	//
	// Parameters: ����.
	// Return: ����.
	/////////////////////////////////////////////////////////////////////////
	CStack_LF ()
	{
		_lUseSize = 0;
		_iUniqueNum = 0;

		_pTop = (st_TOP_NODE *)_aligned_malloc(sizeof(st_TOP_NODE), 16);
		_pTop->pTopNode = NULL;
		_pTop->iUniqueNum = 0;

		_pMemoryPool = new CMemoryPool_LF<st_NODE>(0);
	}

	/////////////////////////////////////////////////////////////////////////
	// �ı���
	//
	// Parameters: ����.
	// Return: ����.
	/////////////////////////////////////////////////////////////////////////
	virtual		~CStack_LF ()
	{
		st_NODE *pNode;
		while (_pTop->pTopNode != NULL)
		{
			pNode = _pTop->pTopNode;
			_pTop->pTopNode = _pTop->pTopNode->pNext;
			_pMemoryPool->Free(pNode);
		}
	}

	/////////////////////////////////////////////////////////////////////////
	// ���� ������� �뷮 ���.
	//
	// Parameters: ����.
	// Return: (int)������� �뷮.
	/////////////////////////////////////////////////////////////////////////
	__int64			GetUseSize(void){ return _lUseSize; }

	/////////////////////////////////////////////////////////////////////////
	// �� ��� ���� ���
	//
	// Parameters: ����.
	// Return: (int)������ ������ �޸�Ǯ ��� FullCount.
	/////////////////////////////////////////////////////////////////////////
	__int64			GetFullNode (void)
	{
		return _pMemoryPool->GetFullCount();
	}
	/////////////////////////////////////////////////////////////////////////
	// �����Ͱ� ����°� ?
	//
	// Parameters: ����.
	// Return: (bool)true, false
	/////////////////////////////////////////////////////////////////////////
	bool			isEmpty(void)
	{
		if ( _pTop->pTopNode == NULL )
		{
			return true;
		}

		return false;
	}


	/////////////////////////////////////////////////////////////////////////
	// CPacket ������ ����Ÿ ����.
	//
	// Parameters: (DATA)����Ÿ.
	// Return: (bool) true, false
	/////////////////////////////////////////////////////////////////////////
	bool			Push(DATA Data)
	{
		st_TOP_NODE pPreTopNode;

		__int64 iUniqueNum = InterlockedIncrement64(&_iUniqueNum);
		st_NODE *pNode = _pMemoryPool->Alloc ();
		pNode->Data = Data;

		do {
			pPreTopNode.iUniqueNum = _pTop->iUniqueNum;
			pPreTopNode.pTopNode = _pTop->pTopNode;


			pNode->pNext = pPreTopNode.pTopNode;
		} while (!InterlockedCompareExchange128((volatile LONG64 *)_pTop, iUniqueNum, (LONG64)pNode, (LONG64 *)&pPreTopNode));
		InterlockedIncrement64 ((volatile LONG64 * )&_lUseSize);
		return true;
	}

	/////////////////////////////////////////////////////////////////////////
	// ����Ÿ ���� ������.
	//
	// Parameters: (DATA *) ���� ������ �־��� ������
	// Return: (bool) true, false
	/////////////////////////////////////////////////////////////////////////
	bool			Pop(DATA *pOutData)
	{
		st_TOP_NODE pPreTopNode;

		InterlockedDecrement64 (( volatile LONG64 * )&_lUseSize);

		__int64 iUniqueNum = InterlockedIncrement64(&_iUniqueNum);

		do
		{
			pPreTopNode.iUniqueNum = _pTop->iUniqueNum;
			pPreTopNode.pTopNode = _pTop->pTopNode;

			if ( pPreTopNode.pTopNode == NULL )
			{
				*pOutData = NULL;
				return false;
			}

		} while (!InterlockedCompareExchange128((volatile LONG64 *)_pTop, iUniqueNum, (LONG64)pPreTopNode.pTopNode->pNext, (LONG64 *)&pPreTopNode));
		*pOutData = pPreTopNode.pTopNode->Data;
		_pMemoryPool->Free(pPreTopNode.pTopNode);

		return true;
	}

private:
	CMemoryPool_LF<st_NODE>	*_pMemoryPool;
	st_TOP_NODE				*_pTop;

	__int64					_iUniqueNum;

	INT64						_lUseSize;
};