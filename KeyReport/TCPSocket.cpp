// TCPServer.cpp: implementation of the CTCPServer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TCPSocket.h"

#ifdef UNDER_CE
#pragma comment(lib,"ws2.lib")
#endif
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTCPServer::CTCPServer(void* pOwner):m_pOwner(pOwner)
{
	m_exitThreadEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
	m_OnClientConnect = NULL;
	m_OnClientClose = NULL;
	m_OnClientRead = NULL;
	m_OnClientError = NULL;
	m_OnServerError = NULL;
	m_serverThreadHandle = NULL;
}

CTCPServer::~CTCPServer()
{
	if (m_serverThreadHandle)
	{
		Close();
	}
	CloseHandle(m_exitThreadEvent);
}

DWORD CTCPServer::SocketThreadFunc(LPVOID lParam)
{
	CTCPServer* pSocket = (CTCPServer*)lParam;
	fd_set fdRead;
	int ret;
	TIMEVAL aTime;
	aTime.tv_sec = 1;
	aTime.tv_usec = 1;

	while (TRUE)
	{
		if (WaitForSingleObject(pSocket->m_exitThreadEvent,100) == WAIT_OBJECT_0)
		{
			break;
		}

		FD_ZERO(&fdRead);
		FD_SET(pSocket->m_serverSocket,&fdRead);
		ret = select(0,&fdRead,NULL,NULL,&aTime);

		if (ret == SOCKET_ERROR)
		{
			int iErrorCode = WSAGetLastError();

			if (pSocket->m_OnServerError)
			{
				pSocket->m_OnServerError(pSocket->m_pOwner,pSocket,iErrorCode);
			}

			shutdown(pSocket->m_serverSocket,SD_BOTH);
			closesocket(pSocket->m_serverSocket);
			break;
		}
		else if (ret == 0)//socket timeout
		{
		}
		else if (ret > 0)
		{
			if (FD_ISSET(pSocket->m_serverSocket,&fdRead))
			{
				SOCKADDR_IN clientAddr;
				CTCPCustom* pClientSocket = new CTCPCustom();
				int len = sizeof (clientAddr);
				pClientSocket->m_socket = accept(pSocket->m_serverSocket,(SOCKADDR*)&clientAddr,&len);

				if (pClientSocket->m_socket != INVALID_SOCKET)
				{
					strcpy(pClientSocket->m_remoteHost,inet_ntoa(clientAddr.sin_addr));
					pClientSocket->m_remotePort = ntohs(clientAddr.sin_port);
					pClientSocket->Associate(pSocket);

					if (pSocket->m_OnClientConnect)
					{
						pSocket->m_OnClientConnect(pSocket->m_pOwner,pClientSocket);
					}

 					pSocket->m_ListClientSocket.AddTail(pClientSocket);
				}
				else
				{
					delete pClientSocket;
					pClientSocket = NULL;
				}
			}
		}
	}
#ifndef UNDER_CE 
	//TRACE("TCPServer thread exit\n");
#endif
	return 0UL;
}

void CTCPServer::RemoveClient(CTCPCustom *pClient)
{
	POSITION posPrior;
	POSITION pos = m_ListClientSocket.GetHeadPosition();

	while (pos != NULL)
	{
		posPrior = pos;
		CTCPCustom* pTcpCustom = (CTCPCustom*)m_ListClientSocket.GetNext(pos);

		if (pTcpCustom == pClient)
		{
			delete pTcpCustom;
			pTcpCustom = NULL;
			m_ListClientSocket.RemoveAt(posPrior);
#ifndef UNDER_CE 
			//TRACE("has been removed on client\n");
#endif
			break;
		}
	}
}

int CTCPServer::Open(int nLocalPort)
{
	WSADATA wsa;

	if (WSAStartup(MAKEWORD(2,2),&wsa) != 0)
	{
		return -1;
	}

	if ((m_serverSocket = socket(AF_INET,SOCK_STREAM,0)) < 0)
	{
		return -2;
	}

	SOCKADDR_IN serverAddr;
	ZeroMemory(&serverAddr,sizeof (serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(nLocalPort);
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(m_serverSocket,(SOCKADDR*)&serverAddr,sizeof (serverAddr)) < 0)
	{
		return -3;
	}

	if (listen(m_serverSocket,8) !=0)
	{
		return -3;
	}

	DWORD ul = 1;
	ioctlsocket(m_serverSocket,FIONBIO,&ul);
	ResetEvent(m_exitThreadEvent);
	m_serverThreadHandle = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)SocketThreadFunc,this,0,NULL);

	if (m_serverThreadHandle == NULL)
	{
		shutdown(m_serverSocket,SD_BOTH);
		closesocket(m_serverSocket);
		return -1;
	}

	return 1;
}

int CTCPServer::Close()
{
	SetEvent(m_exitThreadEvent);

	if (WaitForSingleObject(m_serverThreadHandle,1000) == WAIT_TIMEOUT)
	{
		TerminateThread(m_serverThreadHandle,0);
#ifndef UNDER_CE 
		//TRACE("Terminate server thread\n");
#endif
	}

	CloseHandle(m_serverThreadHandle);
	m_serverThreadHandle = NULL;
	shutdown(m_serverSocket,SD_BOTH);
	closesocket(m_serverSocket);

	POSITION pos = m_ListClientSocket.GetHeadPosition();

	while (pos != NULL)
	{
		CTCPCustom* pTcpCustom = (CTCPCustom*)m_ListClientSocket.GetNext(pos);

		if (!pTcpCustom->Close())
		{
#ifndef UNDER_CE 
			//TRACE("close client socket error\n");
#endif
		}

		delete pTcpCustom;
		pTcpCustom = NULL;
	}

	m_ListClientSocket.RemoveAll();
	return 1;
}

BOOL CTCPServer::SendData(CTCPCustom *pCustom, const char *buf, int len)
{
	BOOL bResult = FALSE;
	BOOL bExisted = FALSE;

	if (pCustom == NULL)
	{
		return FALSE;
	}

	POSITION pos = m_ListClientSocket.GetHeadPosition();

	while (pos != NULL)
	{
		CTCPCustom* pTcpCustom = (CTCPCustom*)m_ListClientSocket.GetNext(pos);

		if (pTcpCustom == pCustom)
		{
			bExisted = TRUE;
			break;
		}
	}

	if (!bExisted)
	{
		return FALSE;
	}

	bResult = pCustom->SendData(buf,len);

	if (!bResult)
	{
		RemoveClient(pCustom);
	}

	return bResult;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTCPCustom::CTCPCustom()
{
	m_pTCPServer = NULL;
	memset(m_remoteHost,0,sizeof(m_remoteHost));
	m_remotePort = 0;
	m_socket = 0;
	m_tcpThreadHandle = NULL;
	m_exitThreadEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
}

CTCPCustom::~CTCPCustom()
{
	if (m_tcpThreadHandle)
	{
		Close();
	}
	CloseHandle(m_exitThreadEvent);
}


DWORD CTCPCustom::SocketThreadFunc(LPVOID lParam)
{
	CTCPCustom* pSocket = (CTCPCustom*)lParam;
	fd_set fdRead;
	int ret;
	TIMEVAL aTime;
	aTime.tv_sec = 1;
	aTime.tv_usec = 0;

	while (TRUE)
	{
		if (WaitForSingleObject(pSocket->m_exitThreadEvent,100) == WAIT_OBJECT_0)
		{
			break;
		}

		FD_ZERO(&fdRead);
		FD_SET(pSocket->m_socket,&fdRead);
		ret = select(0,&fdRead,NULL,NULL,&aTime);

		if (ret == SOCKET_ERROR)
		{
			if (pSocket->m_pTCPServer->m_OnClientError)
			{
				pSocket->m_pTCPServer->m_OnClientError(pSocket->m_pTCPServer->m_pOwner,pSocket,1);
			}

			shutdown(pSocket->m_socket,SD_BOTH);
			closesocket(pSocket->m_socket);
			break;
		}
		else if (ret == 0)//socket timeout
		{
		}
		else if (ret > 0)
		{
			if (FD_ISSET(pSocket->m_socket,&fdRead))
			{
				char recvBuf[1024];
				int recvLen;
				ZeroMemory(recvBuf,1024);
				recvLen = recv(pSocket->m_socket,recvBuf,1024,0);

				if (recvLen == SOCKET_ERROR)
				{
					int nErrorCode = WSAGetLastError();

					if (pSocket->m_pTCPServer->m_OnClientError)
					{
						pSocket->m_pTCPServer->m_OnClientError(pSocket->m_pTCPServer->m_pOwner,pSocket,nErrorCode);
					}

					if (pSocket->m_pTCPServer->m_OnClientClose)
					{
						pSocket->m_pTCPServer->m_OnClientClose(pSocket->m_pTCPServer->m_pOwner,pSocket);
					}

					shutdown(pSocket->m_socket,SD_BOTH);
					closesocket(pSocket->m_socket);
					pSocket->m_pTCPServer->RemoveClient(pSocket);
					break;
				}
				else if (recvLen == 0)//socket closed
				{
					if (pSocket->m_pTCPServer->m_OnClientClose)
					{
						pSocket->m_pTCPServer->m_OnClientClose(pSocket->m_pTCPServer->m_pOwner,pSocket);
					}

					shutdown(pSocket->m_socket,SD_BOTH);
					closesocket(pSocket->m_socket);
					pSocket->m_pTCPServer->RemoveClient(pSocket);
					break;
				}
				else
				{
					if (pSocket->m_pTCPServer->m_OnClientRead)
					{
						pSocket->m_pTCPServer->m_OnClientRead(pSocket->m_pTCPServer->m_pOwner,pSocket,recvBuf,recvLen);
					}
				}
			}
		}
	}
#ifndef UNDER_CE 
	//TRACE("TCPCustom thread exit\n");
#endif
	return 0UL;
}

BOOL CTCPCustom::Associate(CTCPServer* pTCPServer)
{
	ResetEvent(m_exitThreadEvent);
	m_tcpThreadHandle = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)SocketThreadFunc,this,0,NULL);

	if (m_tcpThreadHandle == NULL)
	{
		shutdown(m_socket,SD_BOTH);
		closesocket(m_socket);
		return FALSE;
	}

	DWORD ul = 1;
	ioctlsocket(m_socket,FIONBIO,&ul);
	m_pTCPServer = pTCPServer;
	return TRUE;
}

BOOL CTCPCustom::Close()
{
	SetEvent(m_exitThreadEvent);

	if(WaitForSingleObject(m_tcpThreadHandle,1000) == WAIT_TIMEOUT)
	{
		TerminateThread(m_tcpThreadHandle,0);
#ifndef UNDER_CE 
		//TRACE("Terminate thread\n");
#endif
	}

	CloseHandle(m_tcpThreadHandle);
	m_tcpThreadHandle = NULL;
	shutdown(m_socket,SD_BOTH);
	closesocket(m_socket);

	return TRUE;
}

BOOL CTCPCustom::SendData(const char *buf, int len)
{
	int nBytes = 0;
	int nSendBytes = 0;

	while (nSendBytes < len)
	{
		nBytes = send(m_socket,buf+nSendBytes,len-nSendBytes,0);

		if (nBytes == SOCKET_ERROR)
		{
			int iErrorCode = WSAGetLastError();

			if (m_pTCPServer->m_OnClientError)
			{
				m_pTCPServer->m_OnClientError(m_pTCPServer->m_pOwner,this,iErrorCode);
			}

			if (m_pTCPServer->m_OnClientClose)
			{
				m_pTCPServer->m_OnClientClose(m_pTCPServer->m_pOwner,this);
			}

			Close();
			return FALSE;
		}

		nSendBytes += nBytes;

		if (nSendBytes < len)
		{
			Sleep(1000);
		}
	}
	return TRUE;
}

char* CTCPCustom::GetHostIp()
{
	return m_remoteHost;
}

SOCKET CTCPCustom::GetSocket()
{
	return m_socket;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTCPClient::CTCPClient(void* pOwner):m_pOwner(pOwner)
{
	ZeroMemory(m_recvBuf,4096);
	m_OnDisconnect = NULL;
	m_OnRead = NULL;
	m_OnError = NULL;
	m_exitThreadEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
	m_tcpThreadHandle = NULL;
}

CTCPClient::~CTCPClient()
{
	if (m_tcpThreadHandle)
	{
		Close();
	}
	CloseHandle(m_exitThreadEvent);
}

DWORD CTCPClient::SocketThreadFunc(LPVOID lParam)
{
	CTCPClient* pSocket = (CTCPClient*)lParam;

	fd_set fdRead;
	TIMEVAL aTime;
	int ret;
	aTime.tv_sec = 1;
	aTime.tv_usec = 0;

	while (TRUE)
	{
		if (WaitForSingleObject(pSocket->m_exitThreadEvent,100) == WAIT_OBJECT_0)
		{
			break;
		}

		FD_ZERO(&fdRead);
		FD_SET(pSocket->m_socket,&fdRead);
		ret = select(0,&fdRead,NULL,NULL,&aTime);

		if (ret == SOCKET_ERROR)
		{
			if (pSocket->m_OnError)
			{
				pSocket->m_OnError(pSocket->m_pOwner,1);
			}

			if (pSocket->m_OnDisconnect)
			{
				pSocket->m_OnDisconnect(pSocket->m_pOwner);
			}
			shutdown(pSocket->m_socket,SD_BOTH);
			closesocket(pSocket->m_socket);
			break;
		}

		if (ret > 0)
		{
			if (FD_ISSET(pSocket->m_socket,&fdRead))
			{
				ZeroMemory(pSocket->m_recvBuf,4096);
				int recvLen = recv(pSocket->m_socket,pSocket->m_recvBuf,4096,0);

				if (recvLen == SOCKET_ERROR)
				{
					int iError = WSAGetLastError();

					if (pSocket->m_OnError)
					{
						pSocket->m_OnError(pSocket->m_pOwner,iError);
					}

					if (pSocket->m_OnDisconnect)
					{
						pSocket->m_OnDisconnect(pSocket->m_pOwner);
					}
					shutdown(pSocket->m_socket,SD_BOTH);
					closesocket(pSocket->m_socket);
					break;
				}
				else if (recvLen == 0)
				{
					if(pSocket->m_OnDisconnect)
					{
						pSocket->m_OnDisconnect(pSocket->m_pOwner);
					}
					shutdown(pSocket->m_socket,SD_BOTH);
					closesocket(pSocket->m_socket);
					break;
				}
				else
				{
					if (pSocket->m_OnRead)
					{
						pSocket->m_OnRead(pSocket->m_pOwner,pSocket->m_recvBuf,recvLen);
					}
				}
			}
		}
	}

#ifndef UNDER_CE 
	//TRACE("TCPClient thread exit\n");
#endif
	return 0UL;
}

BOOL CTCPClient::Open()
{
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2,2),&wsa) != 0)
	{
		return FALSE;
	}

	ResetEvent(m_exitThreadEvent);
	m_socket = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);

	if (m_socket == SOCKET_ERROR)
	{
		return FALSE;
	}

	m_tcpThreadHandle = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)SocketThreadFunc,this,0,NULL);

	if (m_tcpThreadHandle == NULL)
	{
		shutdown(m_socket,SD_BOTH);
		closesocket(m_socket);
		return FALSE;
	}

	return TRUE;
}

BOOL CTCPClient::Close()
{
	SetEvent(m_exitThreadEvent);

	if(WaitForSingleObject(m_tcpThreadHandle,1000) == WAIT_TIMEOUT)
	{
		TerminateThread(m_tcpThreadHandle,0);
#ifndef UNDER_CE 
		//TRACE("Terminate thread\n");
#endif
	}

	CloseHandle(m_tcpThreadHandle);
	m_tcpThreadHandle = NULL;
	shutdown(m_socket,SD_BOTH);
	closesocket(m_socket);
	return TRUE;
}

BOOL CTCPClient::Connect(char* m_remoteHost,int nPort)
{
	struct sockaddr_in addr;
	int err;
	DWORD ul = 0;

	ioctlsocket(m_socket,FIONBIO,&ul);//blocking mode
	addr.sin_family = AF_INET;
	addr.sin_port = htons(nPort);
	addr.sin_addr.s_addr = inet_addr(m_remoteHost);
	err = connect(m_socket,(struct sockaddr*)&addr,sizeof (addr));

	if (err == SOCKET_ERROR)
	{
		return FALSE;
	}

	ioctlsocket(m_socket,FIONBIO,&ul);//change to nonblocking mode
	return TRUE;
}

BOOL CTCPClient::SendData(const char *buf, int len)
{
	int nBytes = 0;
	int nSendBytes = 0;

	while (nSendBytes < len)
	{
		nBytes = send(m_socket,buf+nSendBytes,len-nSendBytes,0);

		if (nBytes == SOCKET_ERROR)
		{
			int iErrorCode = WSAGetLastError();

			if (m_OnError)
			{
				m_OnError(m_pOwner,iErrorCode);
			}

			if (m_OnDisconnect)
			{
				m_OnDisconnect(m_pOwner);
			}

			Close();
			return FALSE;
		}

		nSendBytes += nBytes;

		if (nSendBytes < len)
		{
			Sleep(1000);
		}
	}
	return TRUE;
}
