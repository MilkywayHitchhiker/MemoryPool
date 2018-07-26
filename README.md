# MemoryPool
메모리풀 컴파일
ThreadID      l Name              l Average                l MinTime                l MaxTime                l TotalCaLL l
ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ
 176          l Malloc Alloc      l 146.6203            μs l 0.0000              μs l 175090.8184         μs l 100000000  l
 176          l Malloc Free       l 200.1771            μs l 0.0000              μs l 303127.9799         μs l 100000000  l

 2196         l New Alloc         l 138.2666            μs l 0.0000              μs l 73301.2750          μs l 100000000  l
 2196         l New Free          l 219.1542            μs l 0.0000              μs l 36490.5910          μs l 100000000  l

 688          l LOCK Alloc        l 128.5698            μs l 0.0000              μs l 84824.6195          μs l 100000000  l
 688          l LOCK Free         l 215.9906            μs l 0.0000              μs l 38731.2414          μs l 100000000  l

 4016         l LF Alloc          l 665.5138            μs l 320.0929            μs l 296726.1218         μs l 100000000  l
 4016         l LF Free           l 490.7521            μs l 320.0929            μs l 2736474.2343        μs l 100000000  l

 2696         l TLS Alloc         l 104.1278            μs l 0.0000              μs l 111072.2376         μs l 100000000  l
 2696         l TLS Free          l 189.0999            μs l 0.0000              μs l 35850.4052          μs l 100000000  l
 
 
 test 스레드에서 같은 코드로 Alloc과 Free하는 부분만 바꿔가면서 1억번씩 테스트 하였습니다.
