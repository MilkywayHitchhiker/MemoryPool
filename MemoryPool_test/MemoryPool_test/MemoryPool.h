/*---------------------------------------------------------------

	MemoryPool_Ver0.5

	메모리 풀 클래스.
	특정 데이타를 일정량 할당 후 나눠쓴다.

	- 사용법.

	CMemoryPool<DATA> MemPool(300);
	DATA *pData = MemPool.Alloc();

	pData 사용

	MemPool.Free(pData);

----------------------------------------------------------------*/
#ifndef  __MEMORYPOOL__H__
#define  __MEMORYPOOL__H__
#include <assert.h>
#include "lib\Library.h"
#include <Windows.h>
#include <new.h>

#define TLS_basicChunkSize 204800



template <class DATA>
class CMemoryPool
{
#define SafeLane 0xff77668888
private:

	/*========================================================================
	// 각 블럭 앞에 사용될 노드 구조체.
	========================================================================*/
	struct st_BLOCK_NODE
	{
		DATA Data;
		INT64 Safe;

		st_BLOCK_NODE ()
		{
			stpNextBlock = NULL;
		}

		st_BLOCK_NODE *stpNextBlock;
	};


	void LOCK ()
	{
		AcquireSRWLockExclusive (&_CS);
	}
	void Release ()
	{
		ReleaseSRWLockExclusive (&_CS);
	}

public:

	/*========================================================================
	// 생성자, 파괴자.
	//
	// Parameters:	(int) 최대 블럭 개수.
	// Return:		없음.
	========================================================================*/
	CMemoryPool (int iBlockNum)
	{
		st_BLOCK_NODE *pNode, *pPreNode;
		InitializeSRWLock (&_CS);
		/*========================================================================
		// TOP 노드 할당
		========================================================================*/
		_pTop = NULL;

		/*========================================================================
		// 메모리 풀 크기 설정
		========================================================================*/
		m_iBlockCount = iBlockNum;
		m_iAllocCount = 0;
		if ( iBlockNum < 0 )
		{
			CCrashDump::Crash ();
			return;	// Dump
		}
		else if ( iBlockNum == 0 )
		{
			m_bStoreFlag = true;
			_pTop = NULL;
		}

		/*========================================================================
		// DATA * 크기만 큼 메모리 할당 후 BLOCK 연결
		========================================================================*/
		else
		{
			m_bStoreFlag = false;

			pNode = ( st_BLOCK_NODE * )malloc (sizeof (st_BLOCK_NODE));
			_pTop = pNode;
			pPreNode = pNode;

			for ( int iCnt = 1; iCnt < iBlockNum; iCnt++ )
			{
				pNode = ( st_BLOCK_NODE * )malloc (sizeof (st_BLOCK_NODE));
				pPreNode->stpNextBlock = pNode;
				pPreNode = pNode;
			}
		}
	}

	virtual	~CMemoryPool ()
	{
		st_BLOCK_NODE *pNode;

		for ( int iCnt = 0; iCnt < m_iBlockCount; iCnt++ )
		{
			pNode = _pTop;
			_pTop = _pTop->stpNextBlock;
			free (pNode);
		}
	}

	/*========================================================================
	// 블럭 하나를 할당받는다.
	//
	// Parameters: PlacementNew여부.
	// Return:		(DATA *) 데이타 블럭 포인터.
	========================================================================*/
	DATA	*Alloc (bool bPlacementNew = true)
	{
		st_BLOCK_NODE *stpBlock;
		int iBlockCount = m_iBlockCount;


		InterlockedIncrement64 (( LONG64 * )&m_iAllocCount);

		if ( iBlockCount < m_iAllocCount )
		{
			if ( m_bStoreFlag )
			{
				stpBlock = ( st_BLOCK_NODE * )malloc (sizeof (st_BLOCK_NODE));
				InterlockedIncrement64 (( LONG64 * )&m_iBlockCount);
			}

			else
			{
				return nullptr;
			}
		}

		else
		{
			LOCK ();

			stpBlock = _pTop;
			_pTop = _pTop->stpNextBlock;

			Release ();
		}

		if ( bPlacementNew )
		{
			new (( DATA * )&stpBlock->Data) DATA;
		}

		stpBlock->Safe = SafeLane;


		return &stpBlock->Data;
	}

	/*========================================================================
	// 사용중이던 블럭을 해제한다.
	//
	// Parameters:	(DATA *) 블럭 포인터.
	// Return:		(BOOL) TRUE, FALSE.
	========================================================================*/
	bool	Free (DATA *pData)
	{
		st_BLOCK_NODE *stpBlock;


		stpBlock = (( st_BLOCK_NODE * )pData);

		if ( stpBlock->Safe != SafeLane )
		{
			return false;
		}

		LOCK ();

		stpBlock->stpNextBlock = _pTop;
		_pTop = stpBlock;

		Release ();

		InterlockedDecrement64 (( LONG64 * )&m_iAllocCount);

		return true;
	}


	/*========================================================================
	// 현재 사용중인 블럭 개수를 얻는다.
	//
	// Parameters:	없음.
	// Return:		(int) 사용중인 블럭 개수.
	========================================================================*/
	int		GetAllocCount (void)
	{
		return m_iAllocCount;
	}

	/*========================================================================
	// 메모리풀 블럭 전체 개수를 얻는다.
	//
	// Parameters:	없음.
	// Return:		(int) 전체 블럭 개수.
	========================================================================*/
	int		GetFullCount (void)
	{
		return m_iBlockCount;
	}

	/*========================================================================
	// 현재 보관중인 블럭 개수를 얻는다.
	//
	// Parameters:	없음.
	// Return:		(int) 보관중인 블럭 개수.
	========================================================================*/
	int		GetFreeCount (void)
	{
		return m_iBlockCount - m_iAllocCount;
	}

private:
	/*========================================================================
	// 블록 스택의 탑
	========================================================================*/
	st_BLOCK_NODE *_pTop;

	/*========================================================================
	// 메모리 동적 플래그, true면 없으면 동적할당 함
	========================================================================*/
	bool m_bStoreFlag;

	/*========================================================================
	// 현재 사용중인 블럭 개수
	========================================================================*/
	int m_iAllocCount;

	/*========================================================================
	// 전체 블럭 개수
	========================================================================*/
	int m_iBlockCount;

	SRWLOCK _CS;

};

template <class DATA>
class CMemoryPool_LF
{
private:

	/*========================================================================
	// 각 블럭 앞에 사용될 노드 구조체.
	========================================================================*/
	struct st_BLOCK_NODE
	{
		DATA data;
		st_BLOCK_NODE *stpNextBlock;
	};

	/*========================================================================
	// 락프리 메모리 풀의 탑 노드
	========================================================================*/
	struct st_TOP_NODE
	{
		st_BLOCK_NODE *pTopNode;
		__int64 iUniqueNum;
	};

public:

	/*========================================================================
	// 생성자, 파괴자.
	//
	// Parameters:	(int) 최대 블럭 개수.
	// Return:		없음.
	========================================================================*/
	CMemoryPool_LF (int iBlockNum)
	{
		st_BLOCK_NODE *pNode, *pPreNode;

		/*========================================================================
		// TOP 노드 할당
		========================================================================*/
		_pTop = ( st_TOP_NODE * )_aligned_malloc (sizeof (st_TOP_NODE), 16);
		_pTop->pTopNode = NULL;
		_pTop->iUniqueNum = 0;

		_iUniqueNum = 0;

		/*========================================================================
		// 메모리 풀 크기 설정
		========================================================================*/
		m_iBlockCount = iBlockNum;
		m_iAllocCount = 0;
		if ( iBlockNum < 0 )
		{
			CCrashDump::Crash ();
			return;	// Dump
		}
		else if ( iBlockNum == 0 )
		{
			m_bStoreFlag = true;
			_pTop->pTopNode = NULL;
		}

		/*========================================================================
		// DATA * 크기만 큼 메모리 할당 후 BLOCK 연결
		========================================================================*/
		else
		{
			m_bStoreFlag = false;

			pNode = ( st_BLOCK_NODE * )malloc (sizeof (st_BLOCK_NODE));
			_pTop->pTopNode = pNode;
			pPreNode = pNode;

			for ( int iCnt = 1; iCnt < iBlockNum; iCnt++ )
			{
				pNode = ( st_BLOCK_NODE * )malloc (sizeof (st_BLOCK_NODE));
				pPreNode->stpNextBlock = pNode;
				pPreNode = pNode;
			}
		}
	}

	virtual	~CMemoryPool_LF ()
	{
		st_BLOCK_NODE *pNode;

		for ( int iCnt = 0; iCnt < m_iBlockCount; iCnt++ )
		{
			pNode = _pTop->pTopNode;
			_pTop->pTopNode = _pTop->pTopNode->stpNextBlock;
			free (pNode);
		}
	}

	/*========================================================================
	// 블럭 하나를 할당받는다.
	//
	// Parameters: PlacementNew여부.
	// Return:		(DATA *) 데이타 블럭 포인터.
	========================================================================*/
	DATA	*Alloc (bool bPlacementNew = true)
	{
		st_BLOCK_NODE *stpBlock;
		st_TOP_NODE pPreTopNode;
		INT64 iBlockCount = m_iBlockCount;
		INT64 iAllocCount = InterlockedIncrement64 (( volatile LONG64 * )&m_iAllocCount);

		if ( iBlockCount <= iAllocCount )
		{
			if ( m_bStoreFlag )
			{
				stpBlock = ( st_BLOCK_NODE * )malloc (sizeof (st_BLOCK_NODE));
				InterlockedIncrement64 (( volatile LONG64 * )&m_iBlockCount);
			}

			else
				return nullptr;
		}

		else
		{
			INT64 iUniqueNum = InterlockedIncrement64 (( volatile LONG64 * )&_iUniqueNum);

			do
			{
				pPreTopNode.iUniqueNum = _pTop->iUniqueNum;
				pPreTopNode.pTopNode = _pTop->pTopNode;

				if ( pPreTopNode.pTopNode != NULL )
				{
					continue;
				}

			} while ( !InterlockedCompareExchange128 (( volatile LONG64 * )_pTop, iUniqueNum, ( LONG64 )pPreTopNode.pTopNode->stpNextBlock, ( LONG64 * )&pPreTopNode) );

			stpBlock = pPreTopNode.pTopNode;
		}

		if ( bPlacementNew )
		{
			new ((DATA *)&stpBlock->data) DATA;
		}


		return &stpBlock->data;
	}

	/*========================================================================
	// 사용중이던 블럭을 해제한다.
	//
	// Parameters:	(DATA *) 블럭 포인터.
	// Return:		(BOOL) TRUE, FALSE.
	========================================================================*/
	bool	Free (DATA *pData)
	{
		st_BLOCK_NODE *stpBlock;
		st_TOP_NODE pPreTopNode;

		stpBlock = (( st_BLOCK_NODE * )pData);

		InterlockedDecrement64 (( volatile LONG64 * )&m_iAllocCount);

		INT64 iUniqueNum = InterlockedIncrement64 (( volatile LONG64 * )&_iUniqueNum);

		do
		{
			pPreTopNode.iUniqueNum = _pTop->iUniqueNum;
			pPreTopNode.pTopNode = _pTop->pTopNode;
	
			stpBlock->stpNextBlock = pPreTopNode.pTopNode;
		} while ( !InterlockedCompareExchange128 (( volatile LONG64 * )_pTop, iUniqueNum, ( LONG64 )stpBlock, ( LONG64 * )&pPreTopNode) );


		return true;
	}


	/*========================================================================
	// 현재 사용중인 블럭 개수를 얻는다.
	//
	// Parameters:	없음.
	// Return:		(INT64) 사용중인 블럭 개수.
	========================================================================*/
	INT64		GetAllocCount (void)
	{
		return m_iAllocCount;
	}

	/*========================================================================
	// 메모리풀 블럭 전체 개수를 얻는다.
	//
	// Parameters:	없음.
	// Return:		(INT64) 전체 블럭 개수.
	========================================================================*/
	INT64		GetFullCount (void)
	{
		return m_iBlockCount;
	}

	/*========================================================================
	// 현재 보관중인 블럭 개수를 얻는다.
	//
	// Parameters:	없음.
	// Return:		(INT64) 보관중인 블럭 개수.
	========================================================================*/
	INT64		GetFreeCount (void)
	{
		return m_iBlockCount - m_iAllocCount;
	}

private:
	/*========================================================================
	// 블록 스택의 탑
	========================================================================*/
	st_TOP_NODE *_pTop;

	/*========================================================================
	// 탑의 Unique Number
	========================================================================*/
	INT64 _iUniqueNum;

	/*========================================================================
	// 메모리 동적 플래그, true면 없으면 동적할당 함
	========================================================================*/
	bool m_bStoreFlag;

	/*========================================================================
	// 현재 사용중인 블럭 개수
	========================================================================*/
	INT64 m_iAllocCount;

	/*========================================================================
	// 전체 블럭 개수
	========================================================================*/
	INT64 m_iBlockCount;


};







#endif