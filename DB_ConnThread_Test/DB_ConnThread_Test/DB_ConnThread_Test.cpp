// DB_ConnThread_Test.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"

#include "RingBuffer.h"
#include "Header.h"
#include "MemoryPool.h"
#include <Windows.h>
#include <time.h>
#include <locale.h>
#include <mysql.h>
#include <errmsg.h>
#pragma comment(lib,"../MySQL/lib/vs14/mysqlclient.lib")


CRingbuffer Queue;
Hitchhiker::CMemoryPool<st_DBQUERY_HEADER_mempool> MemPool (0);


int WorkerCnt=0;
unsigned int g_ACnt = 0;
unsigned int g_CsCnt = 0;
HANDLE WakeUp_SQL_Thread;
bool EndFlag = false;
unsigned int DBWriteCnt;

unsigned int WINAPI SQLThread (LPVOID lpparam);
unsigned int WINAPI WorkerThread (LPVOID lpparam);




int main()
{
	int retval;
	int Time;
	int OtherTime;
	HANDLE Thread[3];
	WakeUp_SQL_Thread = CreateEvent (NULL, FALSE, FALSE, NULL);

	wprintf (L"\nDB_Connect_Test Start\n");
	_wsetlocale (LC_ALL, L"korean");

	Thread[0] = ( HANDLE )_beginthreadex (NULL, 0, SQLThread, 0, 0, NULL);

	for ( int Cnt = 1; Cnt < 3; Cnt++ )
	{
		Thread[Cnt] = ( HANDLE )_beginthreadex (NULL, 0, WorkerThread, 0, 0, NULL);
	}

	Time = GetTickCount ();


	while ( 1 )
	{

		//초당 TPS(DB 저장횟수)와
		OtherTime = GetTickCount ();
		if ( OtherTime - Time >= 1000 )
		{
			wprintf (L"\nTPS = %d\n", DBWriteCnt);
			wprintf (L"QueueSize = %d\n",Queue.GetUseSize());
			wprintf (L"MemoryPoolFullSize = %d\n", MemPool.GetMemoryPoolFullCount ());
			wprintf (L"MemoryPoolUseSize = %d\n", MemPool.GetUseCount ());

			InterlockedExchange (&DBWriteCnt, 0);
			Time = OtherTime;
		}




		//종료키 확인
		if ( GetAsyncKeyState (VK_SPACE) & 0x8001 || EndFlag == true)
		{
			EndFlag = true;
			SetEvent (WakeUp_SQL_Thread);
		}

		//20ms 대기 후 모든 스레드가 종료됬는지 확인. return이 timeout이 아니라면 정상 종료.
		retval = WaitForMultipleObjects (3, Thread, TRUE, 20);
		if ( retval != WAIT_TIMEOUT )
		{
			wprintf (L"\n모든 스레드 종료\n");
			wprintf (L"QueueSize = %d\n", Queue.GetUseSize ());
			break;
		}
	}




    return 0;
}


unsigned int WINAPI SQLThread (LPVOID lpparam)
{
	int retval;
	
	MYSQL conn;
	MYSQL *connection = NULL;
	char SQLString[128];
	
	int query_stat;


	wprintf (L"SQLThread Start\n");

	//초기화
	mysql_init (&conn);

	//MySQL 서버와 connection 연결 작업
	connection = mysql_real_connect (&conn, "127.0.0.1", "root", "dhwjddnr1!", "test", 3306, ( char* )NULL, 0);

	//Connection이 NULL일 경우
	if ( connection == NULL )
	{
		printf ("\n====================Mysql Connection Error : %s==================\n", mysql_error (&conn));
		return 1;
	}

	//한글사용을위해추가.
	mysql_set_character_set (connection, "utf8");



	st_DBQUERY_HEADER_mempool *Packet;



	while ( 1 )
	{
		//스레드 깨어날지 확인
		retval = WaitForSingleObject (WakeUp_SQL_Thread, INFINITE);

		while ( 1 )
		{
			//메세지 Q사이즈가 헤더 사이즈 보다 작다면 데이터가 없는것으로 스레드 대기로 빠짐.
			if ( Queue.GetUseSize () < sizeof (8) )
			{
				break;
			}

			//헤더를 얻어옴.
			Queue.Get (( char * )&Packet, sizeof (Packet));

			//헤더의 타입 확인 후 해당 컨텐츠 실행.
			switch ( Packet->Type )
			{
			case df_DBQUERY_MSG_NEW_ACCOUNT:
			{
				st_DBQUERY_MSG_NEW_ACCOUNT *Account = ( st_DBQUERY_MSG_NEW_ACCOUNT *)Packet->buff;
				
				//스트링으로 만들기.
				sprintf (SQLString, "INSERT INTO `test`.`account` (`ID`,`Password`) VALUES ('%s','%s');", Account->szID,Account->szPassword);


				//DB에 전송.
				query_stat = mysql_query (connection, SQLString);

				//0이 아니라면 에러.
				if ( query_stat != 0 )
				{
					printf ("\n=============MySQL query error : %s", mysql_error (&conn));
					printf ("\n=============MySQL query error : %s", SQLString);
					MemPool.Free (Packet);
					EndFlag = true;
				}

				InterlockedIncrement (( volatile long * )&DBWriteCnt);

				break;
			}
			case df_DBQUERY_MSG_STAGE_CLEAR:
			{
				st_DBQUERY_MSG_STAGE_CLEAR *stage = ( st_DBQUERY_MSG_STAGE_CLEAR * )Packet->buff;

				//스트링으로 만들기.
				sprintf (SQLString, "INSERT INTO `test`.`stageclear` (`stage`,`accountno`) VALUES ('%d','%lld');", stage->iStageID, stage->iAccountNo);


				//DB에 전송.
				query_stat = mysql_query (connection, SQLString);

				//0이 아니라면 에러.
				if ( query_stat != 0 )
				{
					printf ("\n=============MySQL query error : %s", mysql_error (&conn));
					printf ("\n=============MySQL query error : %s", SQLString);
					MemPool.Free (Packet);
					EndFlag = true;
				}

				InterlockedIncrement (( volatile long * )&DBWriteCnt);

				break;
			}
			default : 
				break;
			}
			

			MemPool.Free (Packet);

		}


		if ( EndFlag == true && Queue.GetUseSize() <= 0 && WorkerCnt == 0)
		{
			mysql_close (connection);

			wprintf (L"\n MySQL Connection Close \n");
			break;
		}
	}
	wprintf (L"SQLThread End\n");
	return 1;
}





unsigned int WINAPI WorkerThread (LPVOID lpparam)
{
	int Swt;

	unsigned int Cnt;
	wprintf (L"WorkerThread Start\n");
	
	InterlockedIncrement (( volatile long * )&WorkerCnt);
	
	srand ((unsigned int)time (NULL));
	
	while ( 1 )
	{
		MemPool.Enter ();
		st_DBQUERY_HEADER_mempool *Packet = MemPool.Alloc();
		MemPool.Leave ();
		Swt = rand () % 2;
		Packet->Type = Swt;


		switch ( Swt )
		{
		case df_DBQUERY_MSG_NEW_ACCOUNT:
		{
			

			st_DBQUERY_MSG_NEW_ACCOUNT *Account = ( st_DBQUERY_MSG_NEW_ACCOUNT * )Packet->buff;
			Packet->Size = sizeof (st_DBQUERY_MSG_NEW_ACCOUNT);

			Cnt = InterlockedIncrement (( volatile long * )&g_ACnt);
			sprintf (Account->szID, "%d", Cnt);
			sprintf (Account->szPassword, "%d", Cnt);
			
			break;
		}
		case df_DBQUERY_MSG_STAGE_CLEAR:
		{
			st_DBQUERY_MSG_STAGE_CLEAR * CStage = ( st_DBQUERY_MSG_STAGE_CLEAR * )Packet->buff;
			Packet->Size = sizeof (st_DBQUERY_MSG_STAGE_CLEAR);
			Cnt = InterlockedIncrement (( volatile long * )&g_CsCnt);

			CStage->iAccountNo = Cnt;
			CStage->iStageID = Cnt;
			
			break;
		}
		case 2 :
		{
			break;
		}
		}

		Queue.Lock ();
		Queue.Put (( char * )&Packet, sizeof (Packet));
		Queue.Free ();

		SetEvent (WakeUp_SQL_Thread);

		if ( EndFlag == true )
		{
			break;
		}
		Sleep (10);
	}
	wprintf (L"WorkerThread End\n");

	InterlockedDecrement (( volatile long * )&WorkerCnt);
	return 0;
}