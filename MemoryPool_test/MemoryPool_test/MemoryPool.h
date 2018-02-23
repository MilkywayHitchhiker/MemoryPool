#pragma once
/*========================
MemoryPool.

메모리 풀 클래스.
특정 데이타(구조체,클래스,변수)를 할당 후 Alloc 해서 쓴다.

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
		// 각 블럭 앞에 사용될 노드 구조체.
		//맨 앞과 뒤에는 해당 구조체의 영역을 체크할 안전장치를 마련.
		/* **************************************************************** */
		struct st_BLOCK_NODE
		{
			unsigned int FrontSafeLine;
			DATA T;
			st_BLOCK_NODE *stpNextBlock;
			unsigned int LastSafeLine;
		};

		//기준노드
		st_BLOCK_NODE *HeadNode;
		

		void *DestroyPointer;
		//생성자 작동시킬 위치
		//false일 경우 메모리풀 생성시 생성자를 한번에 할당함.
		bool PlacementNew;

		//최대치가 정해져 있는경우
		bool BlockFlag;

		//메모리풀에 할당된 노드 전체 갯수
		int MemoryPoolNodeCnt;

		//현재 사용중인 노드 갯수
		int UseSize;

	public:

		//////////////////////////////////////////////////////////////////////////
		// 생성자, 파괴자.
		//
		// Parameters:	(int) 최대 블럭 개수.
		//				(bool) 생성자 호출 여부.
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
				//방법1. 만들려는 블록들을 통으로 한번에 만드는 방법.
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
				//방법2. 노드들을 죄다 따로 할당 하는 방법.
				for ( int cnt = 0; cnt < iBlockNum; cnt++ )
				{
				//새로 노드를 할당받음
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
			//Placement New가 true일때
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
			//두가지로 나뉜다. Placement new가 true일 경우 false일경우
			if ( PlacementNew == true )
			{
				//처음부터 Max치를 고정으로 갔을 경우
				if ( BlockFlag == true )
				{
					free (DestroyPointer);
					return;
				}

				//자동할당으로 갔을 경우

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


			//Placement new가 false일 경우
			

			
			st_BLOCK_NODE *p;
			p = HeadNode;
			
			//처음부터 Max치를 고정으로 갔을 경우
			if ( BlockFlag == true )
			{
				//멤버를 돌면서 파괴자 전부 호출
				while ( p != NULL )
				{
					p->T.~DATA ();
					p = p->stpNextBlock;
				}
				free (DestroyPointer);
				return;
			}

			//자동할당으로 갔을 경우



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
		// 블럭 하나를 할당받는다.
		//
		// Parameters: 없음.
		// Return: (DATA *) 데이타 블럭 포인터.
		//////////////////////////////////////////////////////////////////////////
		DATA	*Alloc (void)
		{
			//헤드노드가 NULL일 경우 두가지로 나뉨.
			if ( HeadNode == NULL )
			{
				//블럭의 최대치가 지정되어있을 경우
				if ( BlockFlag == true )
				{
					return NULL;
				}

				//블럭의 최대치가 지정되어있지 않은 경우 새로 할당 받아서 리턴해준다.
				st_BLOCK_NODE *p =(st_BLOCK_NODE *) malloc (sizeof (st_BLOCK_NODE));
				p->FrontSafeLine = SafeLine;
				p->stpNextBlock = NULL;
				p->LastSafeLine = SafeLine;

				MemoryPoolNodeCnt++;
				UseSize += sizeof (st_BLOCK_NODE);

				//만약 플레이스먼트 뉴를 사용중이라면 여기서 블럭에 대한 생성자를 셋팅
				if ( PlacementNew == true )
				{
					new(&p->T) DATA ();
				}

				return &p->T;

				
			}


			//p포인터에 현재 노드 임시저장. 헤드 노드는 다음노드를 가져감.
			st_BLOCK_NODE *p = HeadNode;
			HeadNode = HeadNode->stpNextBlock;
			
			//만약 플레이스먼트 뉴를 사용중이라면 여기서 블럭에 대한 생성자를 셋팅해준다.
			if ( PlacementNew == true )
			{
				new(&p->T) DATA ();
			}
			
			//블럭포인터를 반환해줌.
			UseSize += sizeof (st_BLOCK_NODE);

			return &p->T;
		}

		//////////////////////////////////////////////////////////////////////////
		// 사용중이던 블럭을 해제한다.
		//
		// Parameters: (DATA *) 블럭 포인터.
		// Return: (BOOL) TRUE, FALSE.
		//////////////////////////////////////////////////////////////////////////
		bool	Free (DATA *pData)
		{
			//블럭 검증 및 노드셋팅을 위한 캐스팅
			st_BLOCK_NODE *p = (st_BLOCK_NODE *) (( char *)pData - 4);

			//받은 포인터가 메모리풀에서 나간 블럭 포인터가 맞는지 확인. 아니라면 false반환
			if ( p->FrontSafeLine != SafeLine || p->LastSafeLine != SafeLine )
			{
				return false;
			}

			//받은 포인터가 메모리풀의 블럭포인터가 맞다면 Placementnew확인후 파괴자 호출
			if ( PlacementNew == true )
			{
				p->T.~DATA ();
			}

			//받은 블럭의 넥스트 블럭 포인터 셋팅 후 헤드에 꼽아 넣는다.
			p->stpNextBlock = HeadNode;
			HeadNode = p;

			UseSize -= sizeof (st_BLOCK_NODE);
			return true;
		}


		//////////////////////////////////////////////////////////////////////////
		// 현재 확보 된 블럭 개수를 얻는다. (메모리풀 내부의 전체 개수)
		//
		// Parameters: 없음.
		// Return: (int) 메모리 풀 내부 전체 개수
		//////////////////////////////////////////////////////////////////////////
		int		GetMemoryPoolFullCount (void)
		{
			return MemoryPoolNodeCnt;
		}

		//////////////////////////////////////////////////////////////////////////
		// 현재 사용중인 블럭 개수를 얻는다.
		//
		// Parameters: 없음.
		// Return: (int) 사용중인 블럭 개수.
		//////////////////////////////////////////////////////////////////////////
		int		GetUseCount (void)
		{
			return UseSize / sizeof(st_BLOCK_NODE);
		}

		///////////////////////////////////////////////////////////////////////////
		//현재 보관중인. 사용가능한 블럭 개수를 얻는다.
		//ps. 생성자에 인자로 Placement new를 true로 해놓지 않았다면 반환된 수치보다 많이 쓸 수 있음.
		// Parameters: 없음.
		// Return: (int) 사용가능 블럭 개수.
		///////////////////////////////////////////////////////////////////////////
		int		GetFreeCount (void)
		{
			return MemoryPoolNodeCnt - (UseSize / sizeof(st_BLOCK_NODE));
		}

	};




	/*===================
	TLS 버전 MemoryPool
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
		// 생성자, 파괴자.
		//
		// Parameters:	(int) 최대 블럭 개수.
		//				(bool) 생성자 호출 여부.
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
		// 블럭 하나를 할당받는다.
		//
		// Parameters: 없음.
		// Return: (DATA *) 데이타 블럭 포인터.
		//////////////////////////////////////////////////////////////////////////
		DATA	*Alloc (void)
		{
			DATA *Data;
			Chunk *p =(Chunk *) TlsGetValue (TLSAllocNum);
			
			//tls에 저장한 청크 블럭을 가져올때 셋팅값이 널이라면 최초 Alloc이므로 셋팅해줘야됨.
			if ( p == NULL )
			{
				p = Set_Chunk_TLS ();
				Data = p->Alloc ();
			}
			else
			{
				Data = p->Alloc ();
				
				//Alloc을 못 받았을 경우 Alloc이 다 된것이므로 새로 청크 셋팅.
				if ( Data == NULL )
				{
					p = Set_Chunk_TLS ();
					Data = p->Alloc ();
				}
			}

			//플레이스먼트 new 체크 및 생성자 호출
			if ( PlacementNewFlag == true)
			{
				new (Data) DATA;
			}

#ifdef _TEST_
			//테스트용 코드. tls특성상 총 Alloc 카운트를 확인 할 수 없으므로 디버그모드에서만 사용.
			InterlockedIncrement64 (( volatile LONG64 * )&TESTAllocCnt);
#endif

			return Data;
		}

		//////////////////////////////////////////////////////////////////////////
		// 사용중이던 블럭을 해제한다.
		//
		// Parameters: (DATA *) 블럭 포인터.
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
			//테스트용 코드. tls특성상 총 Free 카운트를 확인 할 수 없으므로 디버그모드에서만 사용.
			InterlockedIncrement64 (( volatile LONG64 * )&TESTFreeCnt);
#endif

			return true;
		}

		/*
		Tls에 Chunk 포인터 등록
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
		GetAllocCount 함수
		DEBUG모드에서만 수치를 얻을 수 있다.
		return : int
		*/
		int GetAllocCount (void)
		{
			return TESTAllocCnt;
		}

		/*
		GetFreeCount 함수
		DEBUG 모드에서만 수치를 얻을 수 있다.
		return : int
		*/
		int GetFreeCount (void)
		{
			return TESTFreeCnt;
		}
	


	private : 
		/*==========================================
		Chunk 클래스
		각 스레드별 TLS로 할당. 및 Alloc과 Free담당.
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
			Chunk 생성자.
			Parameta : Node의 Max갯수.
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
			Chunk 파괴자.
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
			Chunk Alloc 코드
			return : template DATA Pointer; Alloc할 수 없는 경우 NULL을 반환.
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
			Chunk Free 함수
			모든 Chunk내 노드가 전부 Free됬다면 return false 반환.
			return : bool Flag;
			*/
			bool Free (void)
			{
				//모든 Chunk내 노드 전부 사용.
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

메모리 풀 클래스.
특정 데이타(구조체,클래스,변수)를 일정량 할당 후 나눠쓴다.

- 사용법.

procademy::CMemoryPool<DATA> MemPool(300, FALSE);
DATA *pData = MemPool.Alloc();

pData 사용

MemPool.Free(pData);


!.	아주 자주 사용되어 속도에 영향을 줄 메모리라면 생성자에서
Lock 플래그를 주어 페이징 파일로 복사를 막을 수 있다.
아주 중요한 경우가 아닌이상 사용 금지.



주의사항 :	단순히 메모리 사이즈로 계산하여 메모리를 할당후 메모리 블록을 리턴하여 준다.
클래스를 사용하는 경우 클래스의 생성자 호출 및 클래스정보 할당을 받지 못한다.
클래스의 가상함수, 상속관계가 전혀 이뤄지지 않는다.
VirtualAlloc 으로 메모리 할당 후 memset 으로 초기화를 하므로 클래스정보는 전혀 없다.


----------------------------------------------------------------*/
#include <assert.h>
#include <new.h>


template <class DATA>
class CMemoryPool_LockFree
{
private:

	/* **************************************************************** */
	// 각 블럭 앞에 사용될 노드 구조체.
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
	// 락프리 메모리 풀의 탑 노드
	/* **************************************************************** */
	struct st_TOP_NODE
	{
		st_BLOCK_NODE *pTopNode;
		__int64 iUniqueNum;
	};

public:

	//////////////////////////////////////////////////////////////////////////
	// 생성자, 파괴자.
	//
	// Parameters:	(int) 최대 블럭 개수.
	//				(bool) 메모리 Lock 플래그 - 중요하게 속도를 필요로 한다면 Lock.
	// Return:
	//////////////////////////////////////////////////////////////////////////
	CMemoryPool_LockFree (int iBlockNum, bool bLockFlag = false)
	{
		st_BLOCK_NODE *pNode, *pPreNode;

		////////////////////////////////////////////////////////////////
		// TOP 노드 할당
		////////////////////////////////////////////////////////////////
		_pTop = ( st_TOP_NODE * )_aligned_malloc (sizeof (st_TOP_NODE), 16);
		_pTop->pTopNode = NULL;
		_pTop->iUniqueNum = 0;

		_iUniqueNum = 0;

		////////////////////////////////////////////////////////////////
		// 메모리 풀 크기 설정
		////////////////////////////////////////////////////////////////
		m_iBlockCount = iBlockNum;
		if ( iBlockNum < 0 )	return;	// Dump

		else if ( iBlockNum == 0 )
		{
			m_bStoreFlag = true;
			_pTop->pTopNode = NULL;
		}

		////////////////////////////////////////////////////////////////
		// DATA * 크기만 큼 메모리 할당 후 BLOCK 연결
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
	// 블럭 하나를 할당받는다.
	//
	// Parameters: 없음.
	// Return: (DATA *) 데이타 블럭 포인터.
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
	// 사용중이던 블럭을 해제한다.
	//
	// Parameters: (DATA *) 블럭 포인터.
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
	// 현재 사용중인 블럭 개수를 얻는다.
	//
	// Parameters: 없음.
	// Return: (int) 사용중인 블럭 개수.
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
	// 블록 스택의 탑
	//////////////////////////////////////////////////////////////////////////
	st_TOP_NODE *_pTop;

	//////////////////////////////////////////////////////////////////////////
	// 탑의 Unique Number
	//////////////////////////////////////////////////////////////////////////
	__int64 _iUniqueNum;

	//////////////////////////////////////////////////////////////////////////
	// 메모리 Lock 플래그
	//////////////////////////////////////////////////////////////////////////
	bool m_bLockFlag;

	//////////////////////////////////////////////////////////////////////////
	// 메모리 동적 플래그, true면 없으면 동적할당 함
	//////////////////////////////////////////////////////////////////////////
	bool m_bStoreFlag;

	//////////////////////////////////////////////////////////////////////////
	// 현재 사용중인 블럭 개수
	//////////////////////////////////////////////////////////////////////////
	int m_iAllocCount;

	//////////////////////////////////////////////////////////////////////////
	// 전체 블럭 개수
	//////////////////////////////////////////////////////////////////////////
	int m_iBlockCount;
};

}

#pragma pack(4)

#endif