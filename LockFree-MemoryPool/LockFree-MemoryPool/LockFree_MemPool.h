#pragma once
#include <Windows.h>
#include <new.h>

class T
{
public :
	int Data;
	T (void)
	{
		Data = 11;
	}
	~T (void)
	{
		Data = 0;
	}
};


//������ ���� �޸�Ǯ
class CMemPool_LF
{
#define SafeHead 0x99cc999dd
#define SafeTail 0x777aa77dc
private :
	struct st_Node
	{
		__int64 SafeLine_Head;
		T Data;
		__int64 SafeLine_Tail;

		st_Node *pNext;
	};
	struct Head
	{
		st_Node *Head;
		__int64 Cnt;
	};

	Head *Top;
	
	int _NodeCnt;
	int _MaxNodeCnt;
	bool limit_Node;

	bool _PlacementNew;
	
	unsigned __int64 UniqueCnt;
	
	//�ı��ڿ��� ��� �ı��� �� ���.
	void *_DestroyPointer;

public :
	//������
	CMemPool_LF (bool Placementnew = false, int MaxCnt = 0)
	{
		Top =(Head *) _aligned_malloc (sizeof (Head), 16);
		Top->Head = NULL;
		Top->Cnt = 0;
		_PlacementNew = Placementnew;
		_MaxNodeCnt = MaxCnt;
		_NodeCnt = 0;
		UniqueCnt = 0;

		if ( MaxCnt == 0 )
		{
			limit_Node = false;
			return;
		}



		limit_Node = true;
		st_Node *NewBlock = ( st_Node * )malloc (sizeof (st_Node) * MaxCnt);
		_DestroyPointer = NewBlock;

		//PlacementNew�� false�� ���. ������ �ѹ��� new�� �о���.
		if ( Placementnew == false )
		{
			for ( int Cnt = MaxCnt - 1; Cnt >= 0; Cnt-- )
			{
				st_Node *p = NewBlock + Cnt;
				p->SafeLine_Head = SafeHead;
				new(&p->Data) T ();
				p->SafeLine_Tail = SafeTail;
				p->pNext = Top->Head;
				Top->Head =  p;
			}
		}

		// PlacenemtNew�� true�� ���. �Ҵ縸 �ް� alloc������ Palcenemtnew�� ������ ȣ��.
		// free�� ȸ�������� �ı��� ȣ������.
		else
		{
			for ( int Cnt = MaxCnt - 1; Cnt >= 0; Cnt-- )
			{
				st_Node *p = NewBlock + Cnt;

				p->SafeLine_Head = SafeHead;
				p->pNext = Top->Head;
				p->SafeLine_Tail = SafeTail;
				Top->Head = p;
			}
		}

		return;
	}

	//�ı���
	~CMemPool_LF (void)
	{

	}

	T Alloc(void)
	{
		Head buff;
		st_Node *retp;
		do
		{
			buff.Head = Top->Head;
			buff.Cnt = Top->Cnt;

		} while ( InterlockedCompareExchange128(,,,) );
	}
};