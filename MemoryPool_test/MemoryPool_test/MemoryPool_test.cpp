// MemoryPool_test.cpp : �ܼ� ���� ���α׷��� ���� �������� �����մϴ�.
//

#include "stdafx.h"
#include"MemoryPool.h"


class abc
{
public:
	int Test;

	abc (void)
	{
		Test = 77;
	}
	~abc (void)
	{
		Test = 0;
	}
};

int main()
{
	Hitchhiker::CMemoryPool<abc> MemPool (1000);

	abc *p[1000];

	for ( int cnt = 0; cnt < 1000; cnt++ )
	{
		p[cnt] = MemPool.Alloc ();
		printf ("%d : %d \n", cnt, p[cnt]->Test);
	}

	printf ("Alloc : %d \n", MemPool.GetMemoryPoolFullCount ());
	printf ("Use : %d \n", MemPool.GetUseCount ());
	printf ("Free : %d \n", MemPool.GetFreeCount());


	printf ("\n\n�Ҵ� �Ϻ� ����\n\n");
	for ( int cnt = 0; cnt < 700; cnt++ )
	{
		MemPool.Free (p[cnt]);
	}
	printf ("Alloc : %d \n", MemPool.GetMemoryPoolFullCount ());
	printf ("Use : %d \n", MemPool.GetUseCount ());
	printf ("Free : %d \n", MemPool.GetFreeCount ());

	return 0;
}

