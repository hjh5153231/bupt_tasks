#include <iostream>
#include <string>
#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <cstdio>
#include <vector>

#pragma comment(lib,"Ws2_32.lib")
using namespace std;

const int PORT = 8000;
#define MAX_SIZE 100
vector<pair<int, string>> sockets;

void splitString(char* input, const char* delimiter, char* firstPart, char* secondPart) {
    char* token = strtok(input, delimiter);
    if (token) {
        strcpy(firstPart, token);
        token = strtok(NULL, delimiter);
        if (token) {
            strcpy(secondPart, token);
        }
    }
}

// 线程函数，接受指定用户的信息并引导用户
DWORD WINAPI ThreadProc(LPVOID lp)	//针对每个客户端和服务端的socket都开启一个线程来处理
{
	SOCKET* 	NewConnection = (SOCKET*)lp;	//类型转换
    int recvbyt = 0;
	int sendbyt = 0;
    char RecvBuffer[MAX_SIZE];
    string connectInfo="请输入你的用户名";
    sendbyt = send(*NewConnection, connectInfo.data(), MAX_SIZE, 0); //提示用户输入用户名
    recvbyt = recv(*NewConnection, RecvBuffer, sizeof(RecvBuffer),0); //接收用户的用户名
    string username=RecvBuffer;
	string onlineUser;
	for(pair<int, string> socket:sockets){
		onlineUser+=socket.second+"|";
	}
	string onlineRes="当前在线用户（用|隔开）：";
	onlineRes+=onlineUser;
	if(onlineRes=="当前在线用户（用|隔开）："){       //将当前socket存入数组前向用户展示当前聊天室的在线情况
		sendbyt = send(*NewConnection,"当前无人在线", MAX_SIZE, 0);
	}
	else{
		sendbyt = send(*NewConnection, onlineRes.data(), MAX_SIZE, 0);
	}
	sockets.push_back(make_pair(*NewConnection, username)); //将该socket和对应用户名存入vector
	cout<<"用户-"+username+"已加入"<<endl;
    for(pair<int, string> socket:sockets){
        string connectInfo="客户端加入聊天室，用户名为："+username;
        sendbyt = send(socket.first, connectInfo.data(), MAX_SIZE, 0);
    }
	while (1)	//接收该客户端的数据
	{
		recvbyt = recv(*NewConnection, RecvBuffer, sizeof(RecvBuffer),0);
		cout << "消息：" << RecvBuffer << "来自客户端：" << *NewConnection << endl;
        char SendingMessage[MAX_SIZE];
        char Destination[MAX_SIZE];
        splitString(RecvBuffer, "->", SendingMessage, Destination);
		string res="From "+username+": "+string(SendingMessage); //分割信息和目的地
		for(pair<int, string> socket:sockets){  //发送信息至在线的指定用户
            if(socket.second==string(Destination)){
                sendbyt = send(socket.first,res.data(), MAX_SIZE, 0);
                if (sendbyt < 0)
                {
                    cout << "发送失败" << endl;
                    break;
                }
            }
        }
		if (sendbyt < 0)
		{
			break;
		}
		memset(RecvBuffer, 0, sizeof(RecvBuffer));

	}
	closesocket(*NewConnection);
	free(NewConnection);
	return 0;
}

int main(int argc,char* argv[])
{
	system("chcp 65001");
	WSADATA		wsaData;	//init
	int Ret;
	SOCKET		ListeningSocket;// 服务端的套接字
	SOCKET		NewConnection;	// 客户端的套接字
	SOCKADDR_IN		ServerAddr;	// 存储服务端地址
	


	//初始化socket
	if ((Ret = WSAStartup(MAKEWORD(2, 2), &wsaData)) != 0)
	{
		cout << "WSAStartup failed with error" << endl;
		return 0;
	}


	//创建Socket，指定socket的一些参数
	if ((ListeningSocket = socket(AF_INET, SOCK_STREAM, 0)) == SOCKET_ERROR)
	{
			cout<<"socket failed with error"<<endl;
			WSACleanup();	// 与 WSAStartup 配套使用
			return 0;
	}

		//初始化服务端地址
		ServerAddr.sin_family = AF_INET;	
		ServerAddr.sin_port = htons(PORT);	
	
		ServerAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

		//bind
		if ((bind(ListeningSocket, (LPSOCKADDR)& ServerAddr, sizeof(ServerAddr)) == SOCKET_ERROR))
		{
			cout<<"bind failed with error"<<endl;
			//closesocket(ListeningSocket);	// 释放套接字
			WSACleanup();
			return 0;
		}

	//listen
	if (listen(ListeningSocket, 5) == SOCKET_ERROR)
	{
		cout << "listen failed with error" << endl;
		//closesocket(ListeningSocket);
		WSACleanup();
		return 0;
	}


	cout << "Listening..." << endl;

	while (1)//等待客户端连接，连接成功则开启一个线程进行处理
	{
		SOCKET* ClientSocket = new SOCKET;	
		ClientSocket = (SOCKET*)malloc(sizeof(SOCKET));
		int SockAddrlen = sizeof(sockaddr);
		*ClientSocket = accept(ListeningSocket, 0, 0);
		cout << "客户端已连接，socket：" << *ClientSocket << endl;
		CreateThread(NULL, 0, &ThreadProc, ClientSocket, 0, NULL);
	}
	closesocket(ListeningSocket);
	WSACleanup();
	return 0;
	
}

