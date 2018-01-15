#pragma once

#include <assert.h>
#include <new.h>
#include <malloc.h>
#pragma pack(1)

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
				new(&p->T) DATA ();
				p->stpNextBlock = NULL;
				p->LastSafeLine = SafeLine;

				MemoryPoolNodeCnt++;
				UseSize += sizeof (st_BLOCK_NODE);
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

}

#pragma pack(4)
