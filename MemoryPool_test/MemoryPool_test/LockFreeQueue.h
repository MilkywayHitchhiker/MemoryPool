#pragma once
#include "MemoryPool.h"


/*======================================================================================
//�̱� ��ũ�� ����ũ ���� ������ Queue.
=======================================================================================*/
template<class DATA>
class CQueue_LF
{
#define MaxCnt 50000
private :
	struct NODE
	{
		DATA Data;
		NODE *pNext;
	};
	struct _TOP_NODE
	{
		NODE *pNode;
		INT64 UNIQUEUE;
	};


	_TOP_NODE *_pHead;
	_TOP_NODE *_pTail;

	volatile __int64 _NodeCnt;
	volatile __int64 _MaxCnt;
public :
	CMemoryPool_LF<NODE> *_pMemPool;
	/*//////////////////////////////////////////////////////////////////////
	//������.�ı���.
	//////////////////////////////////////////////////////////////////////*/
	CQueue_LF ()
	{
		_TOP_NODE *HNode = ( _TOP_NODE * )_aligned_malloc (sizeof (_TOP_NODE), 16);
		_TOP_NODE *TNode = ( _TOP_NODE * )_aligned_malloc (sizeof (_TOP_NODE), 16);
		_pMemPool = new CMemoryPool_LF<NODE> (0);

		HNode->pNode = _pMemPool->Alloc ();
		HNode->pNode->pNext = NULL;
		HNode->UNIQUEUE = 0;
		
		TNode->pNode = HNode->pNode;
		TNode->UNIQUEUE = 0;
		_MaxCnt = MaxCnt;
		_NodeCnt = 0;

		
		_pHead = HNode;
		_pTail = TNode;


	}
	~CQueue_LF ()
	{
		DATA p;
		while ( 1 )
		{
			
			if ( Dequeue(&p) == false )
			{
				return;
			}
		}
		_pMemPool->Free (_pHead->pNode);
	}


	/*//////////////////////////////////////////////////////////////////////
	//Enqueue
	//���� : DATA
	//return : true,false
	//////////////////////////////////////////////////////////////////////*/

	bool Enqueue (DATA Data)
	{
		INT64 NodeCnt = InterlockedIncrement64 (&_NodeCnt);

		//Queue�� ��á���Ƿ� return false.
		if ( NodeCnt > _MaxCnt )
		{
			return false;
		}

		_TOP_NODE PreNode;


		NODE *pNode = _pMemPool->Alloc ();
		pNode->Data = Data;

		while ( 1 )
		{

			PreNode.UNIQUEUE = _pTail->UNIQUEUE;
			PreNode.pNode = _pTail->pNode;

			//Tail�� next�� NULL�� �ƴ� ��� �̹� �������� ���� ��带 �־����Ƿ� Tail�� �о���.

			if ( PreNode.pNode->pNext != NULL )
			{
				InterlockedCompareExchange128 (( volatile LONG64 * )_pTail, PreNode.UNIQUEUE + 1, ( LONG64 )PreNode.pNode->pNext, ( LONG64 * )&PreNode);
				continue;
			}

			//Tail�� Next�� NULL�� ��� ���� ��� ����

			pNode->pNext = NULL;
			if ( InterlockedCompareExchangePointer (( volatile PVOID * )&_pTail->pNode->pNext, pNode, NULL) == NULL )
			{
				InterlockedCompareExchange128 (( volatile LONG64 * )_pTail, PreNode.UNIQUEUE + 1, ( LONG64 )pNode, ( LONG64 * )&PreNode);

				return true;
			}
		}
	}


	/*//////////////////////////////////////////////////////////////////////
	//Dequeue
	//���� : ���� �����͸� ���� ����.
	//return : true,false
	//////////////////////////////////////////////////////////////////////*/
	bool Dequeue (DATA *pOut)
	{
		_TOP_NODE PreNode;
		_TOP_NODE TailNode;

		NODE *pNext;



		while ( 1 )
		{

			TailNode.UNIQUEUE = _pTail->UNIQUEUE;
			TailNode.pNode = _pTail->pNode;
			//Tail�� next�� NULL�� �ƴ� ��� �̹� �������� ���� ��带 �־����Ƿ� Tail�� �о���.

			if ( TailNode.pNode->pNext != NULL )
			{
				InterlockedCompareExchange128 (( volatile LONG64 * )_pTail, TailNode.UNIQUEUE + 1, ( LONG64 )TailNode.pNode->pNext, ( LONG64 * )&TailNode);
				continue;
			}


			PreNode.UNIQUEUE = _pHead->UNIQUEUE;
			PreNode.pNode = _pHead->pNode;

			pNext = PreNode.pNode->pNext;
			//����� Next��带 �̴� ���̹Ƿ� pNode�� NULL�̶�� Dequeue �Ұ���.
			if ( pNext == NULL )
			{
				pOut = NULL;
				return false;
			}
			
			*pOut = pNext->Data;
			if ( InterlockedCompareExchange128 (( volatile LONG64 * )_pHead, PreNode.UNIQUEUE + 1, ( LONG64 )pNext, ( LONG64 * )&PreNode) )
			{
				_pMemPool->Free (PreNode.pNode);
				InterlockedDecrement64 (&_NodeCnt);
				return true;
			}
		}
	}



	/*//////////////////////////////////////////////////////////////////////
	//GetUseSize
	//���� : ����
	//return : ���� Node����.
	//////////////////////////////////////////////////////////////////////*/
	INT64 GetUseSize (void)
	{
		return _NodeCnt;
	}
};






/*======================================================================================
//�̱� ��ũ�� ����ũ ���� Thread Safe SRW Lock���� Queue
=======================================================================================*/
template<class DATA>
class CQueue_LOCK
{
private:
	struct NODE
	{
		DATA Data;
		NODE *pNext;
	};
	struct _TOP_NODE
	{
		NODE *pNode;
	};


	_TOP_NODE *_pHead;
	_TOP_NODE *_pTail;

	CMemoryPool<NODE> *_pMemPool;

	volatile __int64 _NodeCnt;
	volatile __int64 _MaxCnt;

	SRWLOCK _CS;
public:

	/*//////////////////////////////////////////////////////////////////////
	//������.�ı���.
	//////////////////////////////////////////////////////////////////////*/
	CQueue_LOCK ()
	{
		InitializeSRWLock (&_CS);

		_TOP_NODE *HNode = ( _TOP_NODE * )_aligned_malloc (sizeof (_TOP_NODE), 16);
		_TOP_NODE *TNode = ( _TOP_NODE * )_aligned_malloc (sizeof (_TOP_NODE), 16);
		HNode->pNode = ( NODE * )malloc (sizeof (NODE));
		HNode->pNode->pNext = NULL;

		TNode->pNode = HNode->pNode;
		_MaxCnt = MaxCnt;
		_NodeCnt = 0;


		_pHead = HNode;
		_pTail = TNode;

		_pMemPool = new CMemoryPool<NODE> (0);
	}
	~CQueue_LOCK ()
	{
		DATA p;
		while ( 1 )
		{
			
			if ( Dequeue (&p) == false )
			{
				return;
			}
		}
	}


	/*//////////////////////////////////////////////////////////////////////
	//Enqueue
	//���� : DATA
	//return : true,false
	//////////////////////////////////////////////////////////////////////*/

	bool Enqueue (DATA Data)
	{

		//Queue�� ��á���Ƿ� return false.
		if ( _NodeCnt >= _MaxCnt )
		{
			return false;
		}

		_TOP_NODE PreNode;


		NODE *pNode = _pMemPool->Alloc ();
		pNode->Data = Data;
		pNode->pNext = NULL;

		LOCK ();

		PreNode.pNode = _pTail->pNode;

		_pTail->pNode->pNext = pNode;
		_pTail->pNode = pNode;
		
		InterlockedIncrement64 (&_NodeCnt);

		Free ();
		return true;
	}


	/*//////////////////////////////////////////////////////////////////////
	//Dequeue
	//���� : ���� �����͸� ���� ����.
	//return : true,false
	//////////////////////////////////////////////////////////////////////*/
	bool Dequeue (DATA *pOut)
	{
		_TOP_NODE PreNode;
		NODE *pNode;
		LOCK ();

		PreNode.pNode = _pHead->pNode;

		pNode = PreNode.pNode->pNext;

		//����� Next��带 �̴� ���̹Ƿ� pNode�� NULL�̶�� Dequeue �Ұ���.
		if ( pNode == NULL )
		{
			pOut = NULL;
			Free ();
			return false;
		}

		_pHead->pNode = pNode;

		*pOut = pNode->Data;
		_pMemPool->Free (PreNode.pNode);
		InterlockedDecrement64 (&_NodeCnt);

		Free ();

		return true;
	}



	/*//////////////////////////////////////////////////////////////////////
	//GetUseSize
	//���� : ����
	//return : ���� Node����.
	//////////////////////////////////////////////////////////////////////*/
	INT64 GetUseSize (void)
	{
		return _NodeCnt;
	}


	void LOCK (void)
	{
		AcquireSRWLockExclusive (&_CS);
	}
	void Free (void)
	{
		ReleaseSRWLockExclusive (&_CS);
	}
};