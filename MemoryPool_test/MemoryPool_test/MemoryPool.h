/*---------------------------------------------------------------

	MemoryPool_Ver1.1

	�޸� Ǯ Ŭ����.
	Ư�� ����Ÿ�� ������ �Ҵ� �� ��������.

	- ����.

	CMemoryPool<DATA> MemPool(300);
	DATA *pData = MemPool.Alloc();

	pData ���

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
	// �� �� �տ� ���� ��� ����ü.
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
	// ������, �ı���.
	//
	// Parameters:	(int) �ִ� �� ����.
	// Return:		����.
	========================================================================*/
	CMemoryPool (int iBlockNum)
	{
		st_BLOCK_NODE *pNode, *pPreNode;
		InitializeSRWLock (&_CS);
		/*========================================================================
		// TOP ��� �Ҵ�
		========================================================================*/
		_pTop = NULL;

		/*========================================================================
		// �޸� Ǯ ũ�� ����
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
		// DATA * ũ�⸸ ŭ �޸� �Ҵ� �� BLOCK ����
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
	// �� �ϳ��� �Ҵ�޴´�.
	//
	// Parameters: PlacementNew����.
	// Return:		(DATA *) ����Ÿ �� ������.
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
	// ������̴� ���� �����Ѵ�.
	//
	// Parameters:	(DATA *) �� ������.
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
	// ���� ������� �� ������ ��´�.
	//
	// Parameters:	����.
	// Return:		(int) ������� �� ����.
	========================================================================*/
	int		GetAllocCount (void)
	{
		return m_iAllocCount;
	}

	/*========================================================================
	// �޸�Ǯ �� ��ü ������ ��´�.
	//
	// Parameters:	����.
	// Return:		(int) ��ü �� ����.
	========================================================================*/
	int		GetFullCount (void)
	{
		return m_iBlockCount;
	}

	/*========================================================================
	// ���� �������� �� ������ ��´�.
	//
	// Parameters:	����.
	// Return:		(int) �������� �� ����.
	========================================================================*/
	int		GetFreeCount (void)
	{
		return m_iBlockCount - m_iAllocCount;
	}

private:
	/*========================================================================
	// ��� ������ ž
	========================================================================*/
	st_BLOCK_NODE *_pTop;

	/*========================================================================
	// �޸� ���� �÷���, true�� ������ �����Ҵ� ��
	========================================================================*/
	bool m_bStoreFlag;

	/*========================================================================
	// ���� ������� �� ����
	========================================================================*/
	int m_iAllocCount;

	/*========================================================================
	// ��ü �� ����
	========================================================================*/
	int m_iBlockCount;

	SRWLOCK _CS;

};

template <class DATA>
class CMemoryPool_LF
{
private:

	/*========================================================================
	// �� �� �տ� ���� ��� ����ü.
	========================================================================*/
	struct st_BLOCK_NODE
	{
		DATA data;
		st_BLOCK_NODE ()
		{
			stpNextBlock = NULL;
		}
		st_BLOCK_NODE *stpNextBlock;
	};

	/*========================================================================
	// ������ �޸� Ǯ�� ž ���
	========================================================================*/
	struct st_TOP_NODE
	{
		st_BLOCK_NODE *pTopNode;
		__int64 iUniqueNum;
	};

public:

	/*========================================================================
	// ������, �ı���.
	//
	// Parameters:	(int) �ִ� �� ����.
	// Return:		����.
	========================================================================*/
	CMemoryPool_LF (int iBlockNum)
	{
		st_BLOCK_NODE *pNode, *pPreNode;

		/*========================================================================
		// TOP ��� �Ҵ�
		========================================================================*/
		_pTop = ( st_TOP_NODE * )_aligned_malloc (sizeof (st_TOP_NODE), 16);
		_pTop->pTopNode = NULL;
		_pTop->iUniqueNum = 0;

		_iUniqueNum = 0;

		/*========================================================================
		// �޸� Ǯ ũ�� ����
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
		}

		/*========================================================================
		// DATA * ũ�⸸ ŭ �޸� �Ҵ� �� BLOCK ����
		========================================================================*/
		else
		{
			m_bStoreFlag = false;

			pNode = ( st_BLOCK_NODE * )malloc (sizeof (DATA) + sizeof (st_BLOCK_NODE));
			_pTop->pTopNode = pNode;
			pPreNode = pNode;

			for ( int iCnt = 1; iCnt < iBlockNum; iCnt++ )
			{
				pNode = ( st_BLOCK_NODE * )malloc (sizeof (DATA) + sizeof (st_BLOCK_NODE));
				pPreNode->stpNextBlock = pNode;
				pPreNode = pNode;
			}
		}
	}

	virtual	~CMemoryPool_LF ()
	{
		st_TOP_NODE *PreTop;
		st_BLOCK_NODE *pNode;


		PreTop = _pTop;
		for ( int iCnt = 0; iCnt < m_iBlockCount; iCnt++ )
		{
			pNode = PreTop->pTopNode;
			PreTop->pTopNode = PreTop->pTopNode->stpNextBlock;
			free (pNode);
		}
	}

	/*========================================================================
	// �� �ϳ��� �Ҵ�޴´�.
	//
	// Parameters: PlacementNew����.
	// Return:		(DATA *) ����Ÿ �� ������.
	========================================================================*/
	DATA	*Alloc (bool bPlacementNew = true)
	{
		st_BLOCK_NODE *stpBlock;
		st_TOP_NODE pPreTopNode;
		int iBlockCount = m_iBlockCount;
		int iAllocCnt = InterlockedIncrement64 (( LONG64 * )&m_iAllocCount);

		if ( iBlockCount < iAllocCnt )
		{
			if ( m_bStoreFlag )
			{
				stpBlock = ( st_BLOCK_NODE * )malloc (sizeof (st_BLOCK_NODE));
				InterlockedIncrement64 (( LONG64 * )&m_iBlockCount);
			}

			else
				return nullptr;
		}

		else
		{
			__int64 iUniqueNum = InterlockedIncrement64 (&_iUniqueNum);

			do
			{
				pPreTopNode.iUniqueNum = _pTop->iUniqueNum;
				pPreTopNode.pTopNode = _pTop->pTopNode;

			} while ( !InterlockedCompareExchange128 (( volatile LONG64 * )_pTop, iUniqueNum, ( LONG64 )_pTop->pTopNode->stpNextBlock, ( LONG64 * )&pPreTopNode) );

			stpBlock = pPreTopNode.pTopNode;
		}

		if ( bPlacementNew )	new (&stpBlock->data) DATA;

		return &stpBlock->data;
	}

	/*========================================================================
	// ������̴� ���� �����Ѵ�.
	//
	// Parameters:	(DATA *) �� ������.
	// Return:		(BOOL) TRUE, FALSE.
	========================================================================*/
	bool	Free (DATA *pData)
	{
		st_BLOCK_NODE *stpBlock;
		st_TOP_NODE pPreTopNode;


		stpBlock = ( st_BLOCK_NODE * )pData;

		__int64 iUniqueNum = InterlockedIncrement64 (&_iUniqueNum);

		do
		{
			pPreTopNode.iUniqueNum = _pTop->iUniqueNum;
			pPreTopNode.pTopNode = _pTop->pTopNode;

			stpBlock->stpNextBlock = _pTop->pTopNode;
		} while ( !InterlockedCompareExchange128 (( volatile LONG64 * )_pTop, iUniqueNum, ( LONG64 )stpBlock, ( LONG64 * )&pPreTopNode) );

		InterlockedDecrement64 (( LONG64 * )&m_iAllocCount);
		return true;
	}


	/*========================================================================
	// ���� ������� �� ������ ��´�.
	//
	// Parameters:	����.
	// Return:		(int) ������� �� ����.
	========================================================================*/
	int		GetAllocCount (void)
	{
		return m_iAllocCount;
	}

	/*========================================================================
	// �޸�Ǯ �� ��ü ������ ��´�.
	//
	// Parameters:	����.
	// Return:		(int) ��ü �� ����.
	========================================================================*/
	int		GetFullCount (void)
	{
		return m_iBlockCount;
	}

	/*========================================================================
	// ���� �������� �� ������ ��´�.
	//
	// Parameters:	����.
	// Return:		(int) �������� �� ����.
	========================================================================*/
	int		GetFreeCount (void)
	{
		return m_iBlockCount - m_iAllocCount;
	}

private:
	/*========================================================================
	// ��� ������ ž
	========================================================================*/
	st_TOP_NODE *_pTop;

	/*========================================================================
	// ž�� Unique Number
	========================================================================*/
	__int64 _iUniqueNum;

	/*========================================================================
	// �޸� ���� �÷���, true�� ������ �����Ҵ� ��
	========================================================================*/
	bool m_bStoreFlag;

	/*========================================================================
	// ���� ������� �� ����
	========================================================================*/
	int m_iAllocCount;

	/*========================================================================
	// ��ü �� ����
	========================================================================*/
	int m_iBlockCount;


};


__declspec (thread) static void *TLSThread = NULL;
template <class DATA>
class CMemoryPool_TLS
{
private:
	/*========================================================================
	// ûũ
	========================================================================*/
	template<class DATA>
	class Chunk
	{
	public:
#define SafeLane 0xff77668888
		struct st_BLOCK_NODE
		{
			DATA BLOCK;
			INT64 Safe = SafeLane;
			Chunk<DATA> *pChunk_Main;
		};
	private :


		st_BLOCK_NODE _pArray[TLS_basicChunkSize];
		CMemoryPool_TLS<DATA> *_pMain_Manager;

		int FullCnt;
		int _Top;
		int FreeCnt;
	public:
		////////////////////////////////////////////////////
		//Chunk ������
		////////////////////////////////////////////////////
		Chunk ()
		{
		}
		~Chunk ()
		{

		}

		bool ChunkSetting (CMemoryPool_TLS<DATA> *pManager)
		{
			_Top = 0;
			FreeCnt = 0;

//			FullCnt = TLS_basicChunkSize;
			_pMain_Manager = pManager;
			Chunk<DATA> *pthis = this;

			for ( int Cnt = TLS_basicChunkSize - 1; Cnt >= 0; Cnt-- )
			{
				_pArray[Cnt].pChunk_Main = pthis;
			}
			return true;
		}

		//////////////////////////////////////////////////////
		// �� �ϳ��� �Ҵ�޴´�.
		//
		// Parameters: PlacementNew����.
		// Return:		(DATA *) ����Ÿ �� ������.
		//////////////////////////////////////////////////////
		DATA	*Alloc (bool bPlacementNew = true)
		{
			int iBlockCount = ++_Top;

		//	st_BLOCK_NODE *stpBlock = &_pArray[iBlockCount - 1];
			DATA * pBLOCK = &_pArray[iBlockCount - 1].BLOCK;


			if ( bPlacementNew )
			{
				new (pBLOCK) DATA;
			}


			if ( iBlockCount == TLS_basicChunkSize )
			{
				//�޸�Ǯ�� �����ϴ� ûũ ��� ����� ���ο� ������� ����.
				_pMain_Manager->Chunk_Alloc ();
			}

			return pBLOCK;

		}

		bool Free (DATA *pData)
		{
			st_BLOCK_NODE *stpBlock = ( st_BLOCK_NODE * )pData;

			if ( stpBlock->Safe != SafeLane )
			{
				return false;
			}

			int Cnt = InterlockedIncrement (( volatile long * )&FreeCnt);

			if ( Cnt == TLS_basicChunkSize )
			{

				_pMain_Manager->Chunk_Free ();

				free (this);
			}

			return true;

		}
	};


	int Chunk_in_BlockCnt;
public:
	/*========================================================================
	// ������
	========================================================================*/
	CMemoryPool_TLS (int iBlockNum)
	{
		if ( iBlockNum == 0 )
		{
			iBlockNum = TLS_basicChunkSize;
		}

		Chunk_in_BlockCnt = iBlockNum;

		m_iBlockCount = 0;
		m_iAllocCount = 0;


	}
	~CMemoryPool_TLS ()
	{
		return;
	}

	/*========================================================================
	// �� �ϳ��� �Ҵ� �޴´�.
	//
	// Parameters:	PlacementNew ����.
	// Return:		(DATA *) �� ������.
	========================================================================*/
	DATA *Alloc (bool bPlacemenenew = true)
	{

		Chunk<DATA> *pChunk = (Chunk<DATA>  * )TLSThread;

		//�ش� �����忡�� ���� ����ɶ�. �ʱ�ȭ �۾�.
		if ( pChunk == NULL )
		{
			pChunk = Chunk_Alloc ();
		}

		DATA *pData = pChunk->Alloc ();
	//	InterlockedIncrement (( volatile long * )&m_iAllocCount);

		return pData;

	}

	/*========================================================================
	// ������̴� ���� �����Ѵ�.
	//
	// Parameters:	(DATA *) �� ������.
	// Return:		(BOOL) TRUE, FALSE.
	========================================================================*/
	bool Free (DATA *pDATA)
	{
		Chunk<DATA>::st_BLOCK_NODE *pNode = (Chunk<DATA>::st_BLOCK_NODE *) pDATA;

		bool chk = pNode->pChunk_Main->Free (pDATA);
//		InterlockedDecrement (( volatile long * )&m_iAllocCount);
		return chk;
	}
public:


	/*========================================================================
	// Alloc�� �ٵ� Chunk���� ��ü�Ѵ�.
	//
	// Parameters:	����
	// Return:		����
	========================================================================*/
	Chunk<DATA> *Chunk_Alloc ()
	{
		int ChunkSize = Chunk_in_BlockCnt;
		
		TLSThread = (Chunk<DATA> *)malloc (sizeof (Chunk<DATA>));

		Chunk<DATA> *pChunk = (Chunk<DATA>  *)TLSThread;
		pChunk->ChunkSetting (this);

		InterlockedIncrement (( volatile long * )&m_iBlockCount);

		return pChunk;
	}
	void Chunk_Free (void)
	{
		InterlockedDecrement (( volatile long * )&m_iBlockCount);
		return;
	}

	/*========================================================================
	// ���� ������� �� ������ ��´�.
	//
	// ! ����
	//	TLS�� ���ɻ� �Ѱ�� ���� ������ ����.
	//
	// Parameters:	����.
	// Return:		(int) ������� �� ����.
	========================================================================*/
	int		GetAllocCount (void)
	{
	//	return m_iAllocCount;
		return 0;
	}
	/*========================================================================
	// �޸�Ǯ �� ��ü ������ ��´�.
	//
	// Parameters:	����.
	// Return:		(int) ��ü �� ����.
	========================================================================*/
	int		GetFullCount (void)
	{
		return m_iBlockCount * Chunk_in_BlockCnt;
	}

	/*========================================================================
	// ���� �������� �� ������ ��´�.
	//
	// ! ����
	//	TLS�� ���ɻ� �Ѱ�� ���� ������ ����.
	//
	// Parameters:	����.
	// Return:		(int) �������� �� ����.
	========================================================================*/
	int		GetFreeCount (void)
	{
	//	return m_iBlockCount - m_iAllocCount;
		return 0;
	}

private:

	int m_iBlockCount;
	int m_iAllocCount;
};








#endif