// KeyReport.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "KeyReport.h"
#define _WIN32_DCOM
#include <comdef.h>
#include <atlbase.h>
#include <rpcsal.h>
// headers needed to use Mobile Broadband APIs 
#pragma comment(lib, "mbnapi_uuid.lib")
#include "mbnapi.h"
#include "TCPSocket.h"
#pragma comment(lib, "IPHLPAPI.lib")
#include <iphlpapi.h>


#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x))
#define FREE(x) HeapFree(GetProcessHeap(), 0, (x))


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CComPtr<IMbnInterfaceManager>  g_InterfaceMgr = NULL;
CTCPClient* g_pClient=NULL;

// The one and only application object

CWinApp theApp;

using namespace std;

void CALLBACK OnError(void* pOwner, int nErrorCode)
{
	printf("on error\r\n");
}

void CALLBACK OnDisconnect(void* pOwner)
{
	printf("on disconnect\r\n");
}


BOOL GetDeviceAddress(void)
{
    DWORD dwSize = 0;
    DWORD dwRetVal = 0;
	CHAR szMac[32] = {0};
	BOOL bWIFI = FALSE,bBT=FALSE,bRet = TRUE;

    unsigned int i;

    /* variables used for GetIfTable and GetIfEntry */
    MIB_IFTABLE *pIfTable;
    MIB_IFROW *pIfRow;

    // Allocate memory for our pointers.
    pIfTable = (MIB_IFTABLE *) MALLOC(sizeof (MIB_IFTABLE));
    if (pIfTable == NULL) {
		//MessageBox("Error allocating memory needed to call GetIfTable","Error",MB_ICONERROR);
        return FALSE;
    }
    // Make an initial call to GetIfTable to get the
    // necessary size into dwSize
    dwSize = sizeof (MIB_IFTABLE);
    if (GetIfTable(pIfTable, &dwSize, FALSE) == ERROR_INSUFFICIENT_BUFFER) {
        FREE(pIfTable);
        pIfTable = (MIB_IFTABLE *) MALLOC(dwSize);
        if (pIfTable == NULL) {
			//MessageBox("Error allocating memory needed to call GetIfTable","Error",MB_ICONERROR); 
            return FALSE;
        }
    }

    // Make a second call to GetIfTable to get the actual
    // data we want.
    if ((dwRetVal = GetIfTable(pIfTable, &dwSize, FALSE)) == NO_ERROR) 
	{
        for (i = 0; i < pIfTable->dwNumEntries; i++) {
            pIfRow = (MIB_IFROW *) & pIfTable->table[i];
            switch (pIfRow->dwType) {
            case IF_TYPE_ETHERNET_CSMACD:
				{
					if (strstr((char*)pIfRow->bDescr,"Bluetooth") && !bBT)
					{
						memset(szMac,0,32);
						sprintf(szMac,"BLTH:%02X-%02X-%02X-%02X-%02X-%02X\r\n",pIfRow->bPhysAddr[0],
							pIfRow->bPhysAddr[1],
							pIfRow->bPhysAddr[2],
							pIfRow->bPhysAddr[3],
							pIfRow->bPhysAddr[4],
							pIfRow->bPhysAddr[5]);
						bBT = TRUE;
						printf("%s",szMac);
						if (g_pClient)
						{
							bRet &= g_pClient->SendData(szMac,strlen(szMac));
						}
					}
				}
                break;
            case IF_TYPE_IEEE80211:
				{
					if (strstr((char*)pIfRow->bDescr,"802.11") && !bWIFI)
					{
						memset(szMac,0,32);
						sprintf(szMac,"WIFI:%02X-%02X-%02X-%02X-%02X-%02X\r\n",pIfRow->bPhysAddr[0],
							pIfRow->bPhysAddr[1],
							pIfRow->bPhysAddr[2],
							pIfRow->bPhysAddr[3],
							pIfRow->bPhysAddr[4],
							pIfRow->bPhysAddr[5]);
						bWIFI = TRUE;
						printf("%s",szMac);
						if (g_pClient)
						{
							bRet &= g_pClient->SendData(szMac,strlen(szMac));
						}
					}
				}
                break;
            default:
                //printf("Unknown type %ld\n", pIfRow->dwType);
                break;
            }
        }
    }

    if (pIfTable != NULL)
	{
        FREE(pIfTable);
        pIfTable = NULL;
    }

	return bRet;
}

BOOL GetIMEI(void)
{
    SAFEARRAY *psa = NULL;
	LONG lBound=0;
	BOOL bRet = FALSE;
	MBN_INTERFACE_CAPS InterfaceCaps;
	CComPtr<IMbnInterface> pMbnInterface = NULL;
	HRESULT hr=CoInitializeEx(NULL, COINIT_MULTITHREADED);
	if (FAILED(hr)) goto END;
    hr = CoCreateInstance(CLSID_MbnInterfaceManager,NULL,CLSCTX_ALL,IID_IMbnInterfaceManager,(void**)&g_InterfaceMgr);
	if (FAILED(hr)) goto END;
	hr = g_InterfaceMgr->GetInterfaces(&psa);
	if (FAILED(hr)) goto END;
	SafeArrayGetElement(psa, &lBound, &pMbnInterface);
	if (FAILED(hr)) goto END;
	hr = pMbnInterface->GetInterfaceCapability(&InterfaceCaps);
	if (FAILED(hr)) goto END;
	wchar_t* pBuf = InterfaceCaps.deviceID;
	char szID[32]={'I','M','E','I',':'};
	wcstombs(szID+5,pBuf,wcslen(pBuf));
	strcat(szID,"\r\n");
	printf("%s",szID);
	if (g_pClient)
	{
		bRet = g_pClient->SendData(szID,strlen(szID));
	}

    SysFreeString(InterfaceCaps.customDataClass);
    SysFreeString(InterfaceCaps.customBandClass);
    SysFreeString(InterfaceCaps.deviceID);
    SysFreeString(InterfaceCaps.manufacturer);
    SysFreeString(InterfaceCaps.model);
    SysFreeString(InterfaceCaps.firmwareInfo);

	return bRet;
END:
	g_InterfaceMgr = NULL;
	pMbnInterface = NULL;
	CoUninitialize();
	printf("IMEI:None\r\n");
	if (g_pClient)
	{
		bRet=g_pClient->SendData("IMEI:None\r\n",strlen("IMEI:None\r\n"));
	}
	return bRet;
}

int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	int nRetCode = 0;
	CFile fp;
	char *szBuf=NULL;
	char *p=NULL;
	DWORD n;
	char szKeyID[64]={0};
	char szKey[64]={0};
	char szSN[64]={0};

	g_pClient = new CTCPClient(NULL);
	g_pClient->m_OnError = OnError;
	g_pClient->m_OnDisconnect = OnDisconnect;
	g_pClient->Open();

	// initialize MFC and print and error on failure
	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
	{
		// TODO: change error code to suit your needs
		_tprintf(_T("Fatal Error: MFC initialization failed\n"));
		nRetCode = 4;
		goto END;
	}
	if (argc == 4)
	{
		if (strcmp(argv[1],"tt") && strncmp(argv[1],"ok",2))
		{
			printf("invalid parameter!\r\n");
			nRetCode = 1;
			goto END;
		}
		int ret = inet_addr(argv[2]);
		if (INADDR_NONE == ret)
		{
			printf("please input a valid ip!\r\n");
			nRetCode = 2;
			goto END;
		}
		int port = atoi(argv[3]);
		if (port < 400 || port > 9999)
		{
			printf("please input a valid port[400-9999]!\r\n");
			nRetCode = 3;
			goto END;
		}
		if (!g_pClient->Connect(argv[2],port))
		{
			printf("connect server failed!\r\n");
			nRetCode = 4;
			goto END;
		}
		if (strncmp(argv[1],"ok",2)==0)
		{
			strcpy(szSN,"SN:");
			strcat(szSN,argv[1]+2);
			strcat(szSN,"\r\n");
			if (fp.Open("oa3.xml",CFile::modeRead))
			{
				n = fp.GetLength();
				szBuf = new char[n];
				fp.Read(szBuf,n);
				fp.Close();
				p = strstr(szBuf,"<ProductKeyID>");
				if (p)
				{
					strcpy(szKeyID,"ID:");
					strncat(szKeyID,p+14,13);
					strcat(szKeyID,"\r\n");
				}
				else
				{
					printf("parse oa3.xml failed!\r\n");
					nRetCode = 5;
					goto END;
				}
				p = strstr(szBuf,"<ProductKey>");
				if (p)
				{
					strcpy(szKey,"KEY:");
					strncat(szKey,p+12,29);
					strcat(szKey,"\r\n");
				}
				else
				{
					printf("parse oa3.xml failed!\r\n");
					nRetCode = 6;
					goto END;
				}
				printf("%s",szSN);
				printf("%s",szKeyID);
				printf("%s",szKey);
				if (g_pClient)
				{
					if (!g_pClient->SendData(szSN,strlen(szSN)))
					{
						nRetCode = 11;
						goto END;
					}
					if (!g_pClient->SendData(szKeyID,strlen(szKeyID)))
					{
						nRetCode = 11;
						goto END;
					}
					if (!g_pClient->SendData(szKey,strlen(szKey)))
					{
						nRetCode = 11;
						goto END;
					}
				}
			}
			else
			{
				printf("oa3.xml does not exist!\r\n");
				nRetCode = 7;
				goto END;
			}
			if (!GetDeviceAddress())
			{
				printf("get device address failed!\r\n");
				nRetCode = 8;
				goto END;
			}
			if (!GetIMEI())
			{
				printf("get device IMEI failed!\r\n");
				nRetCode = 9;
				goto END;
			}
		}
		else
		{
			printf("test connectivity successfully\r\n");
		}
	}
	else
	{
		printf("invalid parameter!\r\n");
		nRetCode = 10;
	}
END:
	if (szBuf)
	{
		delete szBuf;
	}
	if (g_pClient)
	{
		delete g_pClient;
	}
	return nRetCode;
}
