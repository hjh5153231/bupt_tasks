#include <iostream>
#include <string>
#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <conio.h>
#include <Windows.h>
#include <cstdio>
#pragma comment(lib,"Ws2_32.lib")
using namespace std;

const int PORT = 8000;
#define MAX_SIZE 100
DWORD WINAPI ThreadProc(LPVOID lp){
    SOCKET* 	thisConnection = (SOCKET*)lp;
    int sendbyt = 0;
    cout<<"请输入要发送的信息和用户名(用->隔开),回车发送"<<endl;
    while(1){
        string sendInfo;
        getline(cin,sendInfo);
        sendbyt = send(*thisConnection, sendInfo.data(), MAX_SIZE, 0);
        if(sendbyt<0){
            cout << "发送失败" << endl;
        }
    }
}
int main()
{
	system("chcp 65001");
	WSADATA wsd;// 用于初始启动信息
	SOCKADDR_IN		ServerAddr;	// 存储服务端地址
	if (WSAStartup(MAKEWORD(2, 2), &wsd) != 0)
	{
		cout << "WSAStartup failed with error" << endl;
		return 0;
	}
	//初始化socket
	SOCKET SocketClient = socket(AF_INET, SOCK_STREAM, 0);
	if (SocketClient == SOCKET_ERROR)
	{
		cout << "socket failed with error" << endl;
		WSACleanup();
		return 0;
	}
	//初始化服务端地址
	ServerAddr.sin_family = AF_INET;	
	ServerAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	ServerAddr.sin_port = htons(PORT);

	//分配接收缓存
	char recvBuff[MAX_SIZE];
	memset(recvBuff, 0, sizeof(recvBuff));

	cout << "connect to server……" << endl;
	if (connect(SocketClient, (SOCKADDR*)& ServerAddr, sizeof(ServerAddr)) == SOCKET_ERROR)
	{
		cout << "connect failed with error" << endl;
		
		WSACleanup();	
		return 0;
	}
	
	
	cout << "Connection succeeded" << endl;

	//登陆,输入用户名，存储到服务端的vector中
	int recvbyt = 0;
	int sendbyt=0;
	//接收"输入用户名"提示信息
	recvbyt = recv(SocketClient, recvBuff, sizeof(recvBuff), 0);
	cout<<recvBuff<<endl;
	char username[20];
	cin.getline(username, 20);
	//发送用户名到服务端
	sendbyt=send(SocketClient, username, MAX_SIZE, 0);


	//创建线程用于发送消息
	CreateThread(NULL, 0, &ThreadProc, &SocketClient, 0, NULL);
	//主线程接收消息
    while (1)
    {
        int ret;
        ret = recv(SocketClient, recvBuff, sizeof(recvBuff), 0);
        if (recvBuff != 0)
            cout << "接收到消息" << recvBuff << endl;
        memset(recvBuff, 0, sizeof(recvBuff));

    }

	closesocket(SocketClient);
	WSACleanup();
	return 0;

}