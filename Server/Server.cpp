#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#include <iostream>

#pragma comment(lib, "ws2_32")

using namespace std;
char Buffer[1024] = { 0, };


//blocking, synchronous, multiplexing(polling)
int main()
{
	cout << "-------- server --------" << endl;

	WSAData wsaData;

	WSAStartup(MAKEWORD(2, 2), &wsaData);

	SOCKET ListenSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	SOCKADDR_IN	ListenSockAddr;
	memset(&ListenSockAddr, 0 , sizeof(ListenSocket));
	ListenSockAddr.sin_family = AF_INET;
	ListenSockAddr.sin_addr.s_addr = INADDR_ANY;
	ListenSockAddr.sin_port = htons(35000);

	bind(ListenSocket, (SOCKADDR*)&ListenSockAddr, sizeof(ListenSockAddr));

	listen(ListenSocket, SOMAXCONN);

	//blocking 함수, 동기화 작업을 위해서는 (CallBack이 필요하다 그래서 (thread를 사용한다.))
	//blocking 함수, synchronous(TimeOut 기법) -> 성능은 떨어지지만 호환성이 좋다. 대부분에 운영체제에서 사용 가능.
	TIMEVAL TimeOut; //이 시간동안 기다리게 하기.
	TimeOut.tv_sec = 0;
	TimeOut.tv_usec = 500000;
	
	//fd_set -> 소켓의 집합이라고 생각하자.
	fd_set ReadSockets;
	fd_set CopyReadSockets;
	FD_ZERO(&ReadSockets);
	FD_SET(ListenSocket, &ReadSockets);

	while (true)
	{	
		//원본을 복사본에 저장해둠.
		CopyReadSockets = ReadSockets;

		//복사본에서 변화가 있는지 체크
		int ChangeCount = select(0, &CopyReadSockets, 0, 0, &TimeOut);

		if (ChangeCount <= 0)
		{
			//0.5초에 한번씩 서버 작업을 함.
			continue;
		}

		//여기로 오는 것은 뭔가 자료가 있다는 의미
		for (int i = 0; i < (int)ReadSockets.fd_count; ++i)
		{
			if (FD_ISSET(ReadSockets.fd_array[i], &CopyReadSockets))
			{
				if (ReadSockets.fd_array[i] == ListenSocket)
				{
					SOCKADDR_IN	ClientSockAddr;
					memset(&ClientSockAddr, 0, sizeof(ClientSockAddr));
					int ClientSockLength = sizeof(ClientSockAddr);

					//blocking 함수, 
					SOCKET ClientSocket = accept(ListenSocket, (SOCKADDR*)&ClientSockAddr, &ClientSockLength);

					//inet_ntoa() -> 네트워크를 문자열로 변환해서 반환
					std::cout << "connect client" << inet_ntoa(ClientSockAddr.sin_addr) <<std::endl;

					FD_SET(ClientSocket, &ReadSockets);
				}
				else
				{
					//DataReceive
					int RecvBytes = recv(ReadSockets.fd_array[i], Buffer, sizeof(Buffer), 0);

					if (RecvBytes <= 0)
					{
						SOCKADDR_IN	ClosedSockAddr;
						memset(&ClosedSockAddr, 0, sizeof(ClosedSockAddr));
						int ClosedSockLength = sizeof(ClosedSockAddr);

						SOCKET CloseSocket = ReadSockets.fd_array[i];
						getpeername(CloseSocket, (SOCKADDR*)&ClosedSockAddr, &ClosedSockLength);
						std::cout << "disconnect client" << inet_ntoa(ClosedSockAddr.sin_addr) << std::endl;

						FD_CLR(ReadSockets.fd_array[i], &ReadSockets);
						closesocket(CloseSocket);
					}
					else
					{
						SOCKADDR_IN	ClientSockAddr;
						memset(&ClientSockAddr, 0, sizeof(ClientSockAddr));
						int ClientSockLength = sizeof(ClientSockAddr);

						getpeername(ReadSockets.fd_array[i], (SOCKADDR*)&ClientSockAddr, &ClientSockLength);
						std::cout << "Client(" << inet_ntoa(ClientSockAddr.sin_addr); 
						std::cout << ")" << Buffer << " Send" << std::endl;

						for (int i = 0; i < (int)ReadSockets.fd_count; ++i)
						{
							if (ReadSockets.fd_array[i] == ListenSocket)
							{
								int SendBytes = send(ReadSockets.fd_array[i], Buffer, sizeof(Buffer), 0);
								if (SendBytes <= 0)
								{
									SOCKADDR_IN	ClosedSockAddr;
									memset(&ClosedSockAddr, 0, sizeof(ClosedSockAddr));
									int ClosedSockLength = sizeof(ClosedSockAddr);

									SOCKET CloseSocket = ReadSockets.fd_array[i];
									getpeername(CloseSocket, (SOCKADDR*)&ClosedSockAddr, &ClosedSockLength);
									std::cout << "send fail." << std::endl;
									std::cout << "disconnect client" << inet_ntoa(ClosedSockAddr.sin_addr) << std::endl;

									FD_CLR(ReadSockets.fd_array[i], &ReadSockets);
									closesocket(CloseSocket);
								}
							}
						}
					}
				}
			}
		}
	}


	closesocket(ListenSocket);
	WSACleanup();

	return 0;
}