#pragma once
/*========================
MemoryPool.

�޸� Ǯ Ŭ����.
Ư�� ����Ÿ(����ü,Ŭ����,����)�� �Ҵ� �� Alloc �ؼ� ����.

============================*/
#ifndef  __MEMORY_POOL__
#define  __MEMORY_POOL__
#include <assert.h>
#include <new.h>
#include <malloc.h>
#include <Windows.h>
#pragma pack(1)


#define _TEST_

/*
class DATA
{
public :
	int Test;

	DATA (void)
	{
		Test = 0x1234;
	}
	~DATA (void)
	{
		Test = 0;
	}
};
*/

namespace Hitchhiker
{


	template <class DATA>
	class CMemoryPool
	{
#define SafeLine 0x89777789

	private:

		/* **************************************************************** */
		// �� �� �տ� ���� ��� ����ü.
		//�� �հ� �ڿ��� �ش� ����ü�� ������ üũ�� ������ġ�� ����.
		/* **************************************************************** */
		struct st_BLOCK_NODE
		{
			unsigned int FrontSafeLine;
			DATA T;
			st_BLOCK_NODE *stpNextBlock;
			unsigned int LastSafeLine;
		};

		//���س��
		st_BLOCK_NODE *HeadNode;
		

		void *DestroyPointer;
		//������ �۵���ų ��ġ
		//false�� ��� �޸�Ǯ ������ �����ڸ� �ѹ��� �Ҵ���.
		bool PlacementNew;

		//�ִ�ġ�� ������ �ִ°��
		bool BlockFlag;

		//�޸�Ǯ�� �Ҵ�� ��� ��ü ����
		int MemoryPoolNodeCnt;

		//���� ������� ��� ����
		int UseSize;

	public:

		//////////////////////////////////////////////////////////////////////////
		// ������, �ı���.
		//
		// Parameters:	(int) �ִ� �� ����.
		//				(bool) ������ ȣ�� ����.
		// Return:
		//////////////////////////////////////////////////////////////////////////
		CMemoryPool (int iBlockNum = 0, bool bPlacementNew = false)
		{
			HeadNode = NULL;
			PlacementNew = bPlacementNew;

			if ( iBlockNum == 0 )
			{
				BlockFlag = false;
				return;
			}



			BlockFlag = true;

			if ( PlacementNew == false )
			{
				//���1. ������� ��ϵ��� ������ �ѹ��� ����� ���.
				st_BLOCK_NODE *NewBlock = ( st_BLOCK_NODE * )malloc (sizeof (st_BLOCK_NODE) * iBlockNum);
				DestroyPointer = NewBlock;
				for ( int cnt = iBlockNum - 1; cnt >= 0; cnt-- )
				{
					st_BLOCK_NODE *p = NewBlock + cnt;

					p->FrontSafeLine = SafeLine;
					new(&p->T) DATA ();
					p->stpNextBlock = HeadNode;
					p->LastSafeLine = SafeLine;

					HeadNode = p;
				}
				
				MemoryPoolNodeCnt = iBlockNum;

				/*
				//���2. ������ �˴� ���� �Ҵ� �ϴ� ���.
				for ( int cnt = 0; cnt < iBlockNum; cnt++ )
				{
				//���� ��带 �Ҵ����
				st_BLOCK_NODE *NewNode = new st_BLOCK_NODE;

				NewNode->FrontSafeLine = SafeLine;
				new(&p->T) DATA ();
				NewNode->stpNextBlock = HeadNode;

				NewNode->LastSafeLine = SafeLine;

				HeadNode = NewNode;

				}
				*/
				return;
			}



			////////////////////////////////////////////
			//Placement New�� true�϶�
			////////////////////////////////////////////
			st_BLOCK_NODE *NewBlock = ( st_BLOCK_NODE * )malloc (sizeof (st_BLOCK_NODE) * iBlockNum);
			DestroyPointer = NewBlock;
			for ( int cnt = iBlockNum - 1; cnt >= 0; cnt-- )
			{
				st_BLOCK_NODE *p = NewBlock + cnt;

				p->FrontSafeLine = SafeLine;
				p->stpNextBlock = HeadNode;
				p->LastSafeLine = SafeLine;

				HeadNode = p;
			}
			

			return;


		}
		virtual	~CMemoryPool ()
		{
			//�ΰ����� ������. Placement new�� true�� ��� false�ϰ��
			if ( PlacementNew == true )
			{
				//ó������ Maxġ�� �������� ���� ���
				if ( BlockFlag == true )
				{
					free (DestroyPointer);
					return;
				}

				//�ڵ��Ҵ����� ���� ���

				st_BLOCK_NODE *p;
				p = HeadNode;

				while ( p != NULL )
				{
					st_BLOCK_NODE *Delete;

					Delete = p;
					p = p->stpNextBlock;

					free (Delete);
				}

				return;

			}


			//Placement new�� false�� ���
			

			
			st_BLOCK_NODE *p;
			p = HeadNode;
			
			//ó������ Maxġ�� �������� ���� ���
			if ( BlockFlag == true )
			{
				//����� ���鼭 �ı��� ���� ȣ��
				while ( p != NULL )
				{
					p->T.~DATA ();
					p = p->stpNextBlock;
				}
				free (DestroyPointer);
				return;
			}

			//�ڵ��Ҵ����� ���� ���



			while ( p != NULL )
			{
				st_BLOCK_NODE *Delete;

				Delete = p;
				p = p->stpNextBlock;
				Delete->T.~DATA ();

				free (Delete);
			}
			return;



		}


		//////////////////////////////////////////////////////////////////////////
		// �� �ϳ��� �Ҵ�޴´�.
		//
		// Parameters: ����.
		// Return: (DATA *) ����Ÿ �� ������.
		//////////////////////////////////////////////////////////////////////////
		DATA	*Alloc (void)
		{
			//����尡 NULL�� ��� �ΰ����� ����.
			if ( HeadNode == NULL )
			{
				//���� �ִ�ġ�� �����Ǿ����� ���
				if ( BlockFlag == true )
				{
					return NULL;
				}

				//���� �ִ�ġ�� �����Ǿ����� ���� ��� ���� �Ҵ� �޾Ƽ� �������ش�.
				st_BLOCK_NODE *p =(st_BLOCK_NODE *) malloc (sizeof (st_BLOCK_NODE));
				p->FrontSafeLine = SafeLine;
				p->stpNextBlock = NULL;
				p->LastSafeLine = SafeLine;

				MemoryPoolNodeCnt++;
				UseSize += sizeof (st_BLOCK_NODE);

				//���� �÷��̽���Ʈ ���� ������̶�� ���⼭ ���� ���� �����ڸ� ����
				if ( PlacementNew == true )
				{
					new(&p->T) DATA ();
				}

				return &p->T;

				
			}


			//p�����Ϳ� ���� ��� �ӽ�����. ��� ���� ������带 ������.
			st_BLOCK_NODE *p = HeadNode;
			HeadNode = HeadNode->stpNextBlock;
			
			//���� �÷��̽���Ʈ ���� ������̶�� ���⼭ ���� ���� �����ڸ� �������ش�.
			if ( PlacementNew == true )
			{
				new(&p->T) DATA ();
			}
			
			//�������͸� ��ȯ����.
			UseSize += sizeof (st_BLOCK_NODE);

			return &p->T;
		}

		//////////////////////////////////////////////////////////////////////////
		// ������̴� ���� �����Ѵ�.
		//
		// Parameters: (DATA *) �� ������.
		// Return: (BOOL) TRUE, FALSE.
		//////////////////////////////////////////////////////////////////////////
		bool	Free (DATA *pData)
		{
			//�� ���� �� �������� ���� ĳ����
			st_BLOCK_NODE *p = (st_BLOCK_NODE *) (( char *)pData - 4);

			//���� �����Ͱ� �޸�Ǯ���� ���� �� �����Ͱ� �´��� Ȯ��. �ƴ϶�� false��ȯ
			if ( p->FrontSafeLine != SafeLine || p->LastSafeLine != SafeLine )
			{
				return false;
			}

			//���� �����Ͱ� �޸�Ǯ�� �������Ͱ� �´ٸ� PlacementnewȮ���� �ı��� ȣ��
			if ( PlacementNew == true )
			{
				p->T.~DATA ();
			}

			//���� ���� �ؽ�Ʈ �� ������ ���� �� ��忡 �ž� �ִ´�.
			p->stpNextBlock = HeadNode;
			HeadNode = p;

			UseSize -= sizeof (st_BLOCK_NODE);
			return true;
		}


		//////////////////////////////////////////////////////////////////////////
		// ���� Ȯ�� �� �� ������ ��´�. (�޸�Ǯ ������ ��ü ����)
		//
		// Parameters: ����.
		// Return: (int) �޸� Ǯ ���� ��ü ����
		//////////////////////////////////////////////////////////////////////////
		int		GetMemoryPoolFullCount (void)
		{
			return MemoryPoolNodeCnt;
		}

		//////////////////////////////////////////////////////////////////////////
		// ���� ������� �� ������ ��´�.
		//
		// Parameters: ����.
		// Return: (int) ������� �� ����.
		//////////////////////////////////////////////////////////////////////////
		int		GetUseCount (void)
		{
			return UseSize / sizeof(st_BLOCK_NODE);
		}

		///////////////////////////////////////////////////////////////////////////
		//���� ��������. ��밡���� �� ������ ��´�.
		//ps. �����ڿ� ���ڷ� Placement new�� true�� �س��� �ʾҴٸ� ��ȯ�� ��ġ���� ���� �� �� ����.
		// Parameters: ����.
		// Return: (int) ��밡�� �� ����.
		///////////////////////////////////////////////////////////////////////////
		int		GetFreeCount (void)
		{
			return MemoryPoolNodeCnt - (UseSize / sizeof(st_BLOCK_NODE));
		}

	};




	/*===================
	TLS ���� MemoryPool
	===================*/

	template <class DATA>
	class CMemoryPool_TLS
	{
	#define SafeLineTLS 0xffff89777789

	private:
	
		class Chunk;

		bool PlacementNewFlag;
		int TLSAllocNum;
		int MaxBLockNum;
		SRWLOCK _CS;

		__int64 TESTAllocCnt;
		__int64 TESTFreeCnt;
#
		public:

		//////////////////////////////////////////////////////////////////////////
		// ������, �ı���.
		//
		// Parameters:	(int) �ִ� �� ����.
		//				(bool) ������ ȣ�� ����.
		// Return:
		//////////////////////////////////////////////////////////////////////////
		CMemoryPool_TLS (int iBlockNum, bool bPlacementNew = true)
		{
			PlacementNewFlag = bPlacementNew;
			TLSAllocNum = TlsAlloc ();
			MaxBLockNum = iBlockNum;

			TESTAllocCnt = 0;
			TESTFreeCnt = 0;

			InitializeSRWLock (&_CS);
		}
		virtual	~CMemoryPool_TLS ()
		{
		

		}


		//////////////////////////////////////////////////////////////////////////
		// �� �ϳ��� �Ҵ�޴´�.
		//
		// Parameters: ����.
		// Return: (DATA *) ����Ÿ �� ������.
		//////////////////////////////////////////////////////////////////////////
		DATA	*Alloc (void)
		{
			DATA *Data;
			Chunk *p =(Chunk *) TlsGetValue (TLSAllocNum);
			
			//tls�� ������ ûũ ���� �����ö� ���ð��� ���̶�� ���� Alloc�̹Ƿ� ��������ߵ�.
			if ( p == NULL )
			{
				p = Set_Chunk_TLS ();
				Data = p->Alloc ();
			}
			else
			{
				Data = p->Alloc ();
				
				//Alloc�� �� �޾��� ��� Alloc�� �� �Ȱ��̹Ƿ� ���� ûũ ����.
				if ( Data == NULL )
				{
					p = Set_Chunk_TLS ();
					Data = p->Alloc ();
				}
			}

			//�÷��̽���Ʈ new üũ �� ������ ȣ��
			if ( PlacementNewFlag == true)
			{
				new (Data) DATA;
			}

#ifdef _TEST_
			//�׽�Ʈ�� �ڵ�. tlsƯ���� �� Alloc ī��Ʈ�� Ȯ�� �� �� �����Ƿ� ����׸�忡���� ���.
			InterlockedIncrement64 (( volatile LONG64 * )&TESTAllocCnt);
#endif

			return Data;
		}

		//////////////////////////////////////////////////////////////////////////
		// ������̴� ���� �����Ѵ�.
		//
		// Parameters: (DATA *) �� ������.
		// Return: (BOOL) TRUE, FALSE.
		//////////////////////////////////////////////////////////////////////////
		bool	Free (DATA *pData)
		{
			Chunk::Node *pNode = (Chunk::Node *)((__int64 *)pData--);
			Chunk *pChunk = pNode->MotherChunk;
			if ( !pChunk->Free () )
			{
				delete pChunk;
			}
#ifdef _TEST_
			//�׽�Ʈ�� �ڵ�. tlsƯ���� �� Free ī��Ʈ�� Ȯ�� �� �� �����Ƿ� ����׸�忡���� ���.
			InterlockedIncrement64 (( volatile LONG64 * )&TESTFreeCnt);
#endif

			return true;
		}

		/*
		Tls�� Chunk ������ ���
		*/
		Chunk *Set_Chunk_TLS (void)
		{
			Chunk *p = new Chunk(MaxBLockNum);

			AcquireSRWLockExclusive (&_CS);
			TlsSetValue (TLSAllocNum, ( LPVOID )p);
			ReleaseSRWLockExclusive (&_CS);
			
			return p;
		}

		/*
		GetAllocCount �Լ�
		DEBUG��忡���� ��ġ�� ���� �� �ִ�.
		return : int
		*/
		int GetAllocCount (void)
		{
			return TESTAllocCnt;
		}

		/*
		GetFreeCount �Լ�
		DEBUG ��忡���� ��ġ�� ���� �� �ִ�.
		return : int
		*/
		int GetFreeCount (void)
		{
			return TESTFreeCnt;
		}
	


	private : 
		/*==========================================
		Chunk Ŭ����
		�� �����庰 TLS�� �Ҵ�. �� Alloc�� Free���.
		==========================================*/
		class Chunk
		{
		public :
			struct Node
			{
				__int64 SafeTop;
				DATA Data;
				__int64 SafeBot;
				Node *pNextNode;
				Chunk *MotherChunk;
				bool Useflag;
			};
		public :
			Node *Destroy_Top_Pointer;
			Node *TopNode;
			unsigned __int64 Alloc_Cnt;
			unsigned __int64 Free_Cnt;
			int MaxNode;


			/*
			Chunk ������.
			Parameta : Node�� Max����.
			*/
			Chunk (int MaxNode_Cnt)
			{
				MaxNode = MaxNode_Cnt;
				Alloc_Cnt = 0;
				Free_Cnt = 0;
				TopNode = NULL;
				Node *NewNode;

				for ( int Cnt = MaxNode_Cnt; Cnt > 0; Cnt-- )
				{
					NewNode = ( Node * )malloc (sizeof (Node));
					NewNode->SafeTop = SafeLineTLS;
					NewNode->SafeBot = SafeLineTLS;
					NewNode->pNextNode = TopNode;
					NewNode->MotherChunk = this;
					NewNode->Useflag = false;

					TopNode = NewNode;
				}
				Destroy_Top_Pointer = TopNode;
			};

			/*
			Chunk �ı���.
			parameta : none
			*/
			~Chunk ()
			{
				Node *pBackup_Destroyed_Node;
				Node *pTopNode = Destroy_Top_Pointer;

				int MaxNodeCnt = MaxNode;

				for ( int Cnt = MaxNodeCnt; Cnt > 0; Cnt-- )
				{
					pBackup_Destroyed_Node = pTopNode;
					pTopNode = pTopNode->pNextNode;

					free (pBackup_Destroyed_Node);
				}
				return;
			};

			/*
			Chunk Alloc �ڵ�
			return : template DATA Pointer; Alloc�� �� ���� ��� NULL�� ��ȯ.
			*/
			DATA *Alloc (void)
			{
				if ( InterlockedIncrement64 (( volatile  LONG64 * )&Alloc_Cnt) >= MaxNode )
				{
					return NULL;
				}

				Node *backup_TopNode = TopNode;
				TopNode = backup_TopNode->pNextNode;
				
				backup_TopNode->Useflag = true;

				return &backup_TopNode->Data;			
			}

			/*
			Chunk Free �Լ�
			��� Chunk�� ��尡 ���� Free��ٸ� return false ��ȯ.
			return : bool Flag;
			*/
			bool Free (void)
			{
				//��� Chunk�� ��� ���� ���.
				if ( InterlockedIncrement64 (( volatile  LONG64 * )&Free_Cnt) >= MaxNode )
				{
					return false;
				}

				return true;
			}
		};

};

/*---------------------------------------------------------------

MemoryPool.

�޸� Ǯ Ŭ����.
Ư�� ����Ÿ(����ü,Ŭ����,����)�� ������ �Ҵ� �� ��������.

- ����.

procademy::CMemoryPool<DATA> MemPool(300, FALSE);
DATA *pData = MemPool.Alloc();

pData ���

MemPool.Free(pData);


!.	���� ���� ���Ǿ� �ӵ��� ������ �� �޸𸮶�� �����ڿ���
Lock �÷��׸� �־� ����¡ ���Ϸ� ���縦 ���� �� �ִ�.
���� �߿��� ��찡 �ƴ��̻� ��� ����.



���ǻ��� :	�ܼ��� �޸� ������� ����Ͽ� �޸𸮸� �Ҵ��� �޸� ����� �����Ͽ� �ش�.
Ŭ������ ����ϴ� ��� Ŭ������ ������ ȣ�� �� Ŭ�������� �Ҵ��� ���� ���Ѵ�.
Ŭ������ �����Լ�, ��Ӱ��谡 ���� �̷����� �ʴ´�.
VirtualAlloc ���� �޸� �Ҵ� �� memset ���� �ʱ�ȭ�� �ϹǷ� Ŭ���������� ���� ����.


----------------------------------------------------------------*/
#include <assert.h>
#include <new.h>


template <class DATA>
class CMemoryPool_LockFree
{
private:

	/* **************************************************************** */
	// �� �� �տ� ���� ��� ����ü.
	/* **************************************************************** */
	struct st_BLOCK_NODE
	{
		st_BLOCK_NODE ()
		{
			stpNextBlock = NULL;
		}
		st_BLOCK_NODE *stpNextBlock;
	};

	/* **************************************************************** */
	// ������ �޸� Ǯ�� ž ���
	/* **************************************************************** */
	struct st_TOP_NODE
	{
		st_BLOCK_NODE *pTopNode;
		__int64 iUniqueNum;
	};

public:

	//////////////////////////////////////////////////////////////////////////
	// ������, �ı���.
	//
	// Parameters:	(int) �ִ� �� ����.
	//				(bool) �޸� Lock �÷��� - �߿��ϰ� �ӵ��� �ʿ�� �Ѵٸ� Lock.
	// Return:
	//////////////////////////////////////////////////////////////////////////
	CMemoryPool_LockFree (int iBlockNum, bool bLockFlag = false)
	{
		st_BLOCK_NODE *pNode, *pPreNode;

		////////////////////////////////////////////////////////////////
		// TOP ��� �Ҵ�
		////////////////////////////////////////////////////////////////
		_pTop = ( st_TOP_NODE * )_aligned_malloc (sizeof (st_TOP_NODE), 16);
		_pTop->pTopNode = NULL;
		_pTop->iUniqueNum = 0;

		_iUniqueNum = 0;

		////////////////////////////////////////////////////////////////
		// �޸� Ǯ ũ�� ����
		////////////////////////////////////////////////////////////////
		m_iBlockCount = iBlockNum;
		if ( iBlockNum < 0 )	return;	// Dump

		else if ( iBlockNum == 0 )
		{
			m_bStoreFlag = true;
			_pTop->pTopNode = NULL;
		}

		////////////////////////////////////////////////////////////////
		// DATA * ũ�⸸ ŭ �޸� �Ҵ� �� BLOCK ����
		////////////////////////////////////////////////////////////////
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

	virtual	~CMemoryPool_LockFree ()
	{
		st_BLOCK_NODE *pNode;

		for ( int iCnt = 0; iCnt < m_iBlockCount; iCnt++ )
		{
			pNode = _pTop->pTopNode;
			_pTop->pTopNode = _pTop->pTopNode->stpNextBlock;
			free (pNode);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// �� �ϳ��� �Ҵ�޴´�.
	//
	// Parameters: ����.
	// Return: (DATA *) ����Ÿ �� ������.
	//////////////////////////////////////////////////////////////////////////
	DATA	*Alloc (bool bPlacementNew = true)
	{
		st_BLOCK_NODE *stpBlock;
		st_TOP_NODE pPreTopNode;
		int iBlockCount = m_iBlockCount;
		InterlockedIncrement64 (( LONG64 * )&m_iAllocCount);

		if ( iBlockCount < m_iAllocCount )
		{
			if ( m_bStoreFlag )
			{
				stpBlock = ( st_BLOCK_NODE * )malloc (sizeof (DATA) + sizeof (st_BLOCK_NODE));
				stpBlock->stpNextBlock = NULL;
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

			} while ( !InterlockedCompareExchange128 (( volatile LONG64 * )_pTop,
				iUniqueNum,
				( LONG64 )_pTop->pTopNode->stpNextBlock,
				( LONG64 * )&pPreTopNode) );

			stpBlock = pPreTopNode.pTopNode;
		}

		if ( bPlacementNew )	new (( DATA * )(stpBlock + 1)) DATA;

		return ( DATA * )(stpBlock + 1);
	}

	//////////////////////////////////////////////////////////////////////////
	// ������̴� ���� �����Ѵ�.
	//
	// Parameters: (DATA *) �� ������.
	// Return: (BOOL) TRUE, FALSE.
	//////////////////////////////////////////////////////////////////////////
	bool	Free (DATA *pData)
	{
		st_BLOCK_NODE *stpBlock;
		st_TOP_NODE pPreTopNode;

		__int64 iUniqueNum = InterlockedIncrement64 (&_iUniqueNum);

		do
		{
			pPreTopNode.iUniqueNum = _pTop->iUniqueNum;
			pPreTopNode.pTopNode = _pTop->pTopNode;

			stpBlock = (( st_BLOCK_NODE * )pData - 1);
			stpBlock->stpNextBlock = _pTop->pTopNode;
		} while ( !InterlockedCompareExchange128 (( volatile LONG64 * )_pTop, iUniqueNum, ( LONG64 )stpBlock, ( LONG64 * )&pPreTopNode) );

		InterlockedDecrement64 (( LONG64 * )&m_iAllocCount);
		return true;
	}


	//////////////////////////////////////////////////////////////////////////
	// ���� ������� �� ������ ��´�.
	//
	// Parameters: ����.
	// Return: (int) ������� �� ����.
	//////////////////////////////////////////////////////////////////////////
	int		GetAllocCount (void)
	{
		return m_iAllocCount;
	}

	int		GetBlockCount (void)
	{
		return m_iBlockCount;
	}
private:
	//////////////////////////////////////////////////////////////////////////
	// ��� ������ ž
	//////////////////////////////////////////////////////////////////////////
	st_TOP_NODE *_pTop;

	//////////////////////////////////////////////////////////////////////////
	// ž�� Unique Number
	//////////////////////////////////////////////////////////////////////////
	__int64 _iUniqueNum;

	//////////////////////////////////////////////////////////////////////////
	// �޸� Lock �÷���
	//////////////////////////////////////////////////////////////////////////
	bool m_bLockFlag;

	//////////////////////////////////////////////////////////////////////////
	// �޸� ���� �÷���, true�� ������ �����Ҵ� ��
	//////////////////////////////////////////////////////////////////////////
	bool m_bStoreFlag;

	//////////////////////////////////////////////////////////////////////////
	// ���� ������� �� ����
	//////////////////////////////////////////////////////////////////////////
	int m_iAllocCount;

	//////////////////////////////////////////////////////////////////////////
	// ��ü �� ����
	//////////////////////////////////////////////////////////////////////////
	int m_iBlockCount;
};

}

#pragma pack(4)

#endif