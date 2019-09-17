
// NetTestDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "NetTest.h"
#include "NetTestDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define SERVER_PORT 8080
// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CNetTestDlg �Ի���




CNetTestDlg::CNetTestDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CNetTestDlg::IDD, pParent)
	, m_bConning(false)
	, m_bAccept(FALSE)
	, m_bConnect(false)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	RecThread=NULL;
	sConnecting=false;
	pRecvThread=NULL;
}

void CNetTestDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_IPADDRESS1, m_IpAddr);
	DDX_Control(pDX, IDC_EDIT1, m_Port);
	DDX_Control(pDX, IDC_RECVDATA, m_RecvEdit);
}

BEGIN_MESSAGE_MAP(CNetTestDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_INITNET, &CNetTestDlg::OnBnClickedInitnet)
	ON_BN_CLICKED(IDC_SENDDATA, &CNetTestDlg::OnBnClickedSenddata)
	ON_BN_CLICKED(IDC_INITCLIENT, &CNetTestDlg::OnBnClickedInitclient)
	ON_BN_CLICKED(IDC_SENDDATA2, &CNetTestDlg::OnBnClickedSendToServe)
END_MESSAGE_MAP()


// CNetTestDlg ��Ϣ�������

BOOL CNetTestDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������
	m_IpAddr.SetAddress(192,168,2,112);
	m_Port.SetWindowText("8080");

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CNetTestDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CNetTestDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CNetTestDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CNetTestDlg::OnBnClickedInitnet()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	InitNetWork();
}


bool CNetTestDlg::InitNetWork(void)
{
	CString Str="";
	GetDlgItemText(IDC_INITNET,Str);
	if(Str =="��������" )
	{	
		m_bConning=false;
		m_bAccept=FALSE;
		WaitForSingleObject(AccEvent,INFINITE);//�ȴ��߳̽���
		WaitForSingleObject(RecEvevt,INFINITE);
		closesocket(m_Server);
		closesocket(m_Client);
		WSACleanup();
		SetDlgItemTextA(IDC_INITNET,"����������");
		return TRUE;
	}
	else if(Str =="���ڼ���")
	{
		m_bAccept=FALSE;
		WaitForSingleObject(AccEvent,INFINITE);
		closesocket(m_Client);
		WSACleanup();
		SetDlgItemTextA(IDC_INITNET,"����������");
		return TRUE;
	}
	int reVal;
	//��ʼ��Windows Sockets DLL
	WSADATA  wsData;
	reVal = WSAStartup(MAKEWORD(2,2),&wsData);

	//�����׽���
	m_Server = socket(AF_INET, SOCK_STREAM, 0);
	if(INVALID_SOCKET== m_Server)
		return FALSE;

	//�����׽��ַ�����ģʽ
	unsigned long ul = 1;
	reVal = ioctlsocket(m_Server, FIONBIO, (unsigned long*)&ul);
	if (SOCKET_ERROR == reVal)
		return FALSE;

	//���׽���
	sockaddr_in serAddr;
	serAddr.sin_family = AF_INET;
	serAddr.sin_port = htons(SERVER_PORT);
	serAddr.sin_addr.S_un.S_addr = INADDR_ANY;
	reVal = bind(m_Server, (struct sockaddr*)&serAddr, sizeof(serAddr));
	if(SOCKET_ERROR == reVal )
		return FALSE;

	//����
	reVal = listen(m_Server, SOMAXCONN);
	if(SOCKET_ERROR == reVal)
		return FALSE;
	SetDlgItemTextA(IDC_INITNET,"���ڼ���");
	m_bAccept=TRUE;
	NetThread=AfxBeginThread(WaitConnectThread,this,THREAD_PRIORITY_NORMAL,0,0,NULL);
	if(NetThread==NULL)
	{
		return FALSE;
	}

	return TRUE;

}

UINT WaitConnectThread(LPVOID LPARAM)
{
    CNetTestDlg *Dlg=(CNetTestDlg *)LPARAM;
	sockaddr ClientAddr;
	int AddrLen=sizeof(ClientAddr);
	UINT sClient=0;
	Dlg->m_bConning=FALSE;
	while(Dlg->m_bAccept)
	{
		sClient=accept(Dlg->m_Server,/*(struct sockaddr*)*/&ClientAddr,&AddrLen);
		if(INVALID_SOCKET == sClient)
		{		
			Sleep(1000);
			int nErrCode = WSAGetLastError();
			if(nErrCode == WSAEWOULDBLOCK)	//�޷��������һ�����赲���׽��ֲ���
			{
//				Sleep(TIMEFOR_THREAD_SLEEP);
				continue;//�����ȴ�
			}
			else
			{
				return 0;//�߳��˳�
			}

	    }else
		{
		  Dlg->m_Client=sClient;
		  if(Dlg->RecThread)
		  {
			 //Dlg->m_bConning=false;
			 //WaitForSingleObject(Dlg->RecEvevt,INFINITE);//�ȴ��߳̽���
			 Dlg->RecThread=NULL;
		  }
          Dlg->m_bConning=TRUE;
          Dlg->RecThread =AfxBeginThread(RecvThread,(LPVOID)&(Dlg->m_Client),THREAD_PRIORITY_NORMAL,0,0,NULL);
		  if( Dlg->RecThread==NULL)
		  {
			  return FALSE;
		  }
		  SetDlgItemTextA( Dlg->m_hWnd,IDC_INITNET,"��������");
		
		}
	}
	Dlg->AccEvent.SetEvent();
	Dlg->OnWriteLog("�˳������̣߳�");
	return TRUE;
}

UINT RecvThread(LPVOID LPARAM)
{
   CNetTestDlg *pMainDlg=(CNetTestDlg *)AfxGetApp()->GetMainWnd();
   SOCKET s= *(SOCKET *)(LPARAM);
   char RecvBuff[100]={0};
   int RecvLen=0;
   bool retVal = TRUE;

   while(pMainDlg->m_bConning)
   {
	   RecvLen=recv(s,RecvBuff,256,NULL);

	   if(SOCKET_ERROR == RecvLen)
	   {
		   int nErrCode = WSAGetLastError();
		   if ( WSAEWOULDBLOCK == nErrCode )	//�������ݻ�����������
		   {
			   Sleep(20);
			   continue;						//����ѭ��
		   }
		   else if(WSAENETDOWN == nErrCode || WSAETIMEDOUT == nErrCode || WSAECONNRESET == nErrCode) //�ͻ��˹ر�������
		   {
			   retVal = FALSE;	//������ʧ��
			   break;							//�߳��˳�
		   }
	   }
	   else if(0 == RecvLen)
	   {
		   retVal = FALSE;
		   break;
	   }
	   else if(RecvLen>0)
	   {
		 pMainDlg->OnWriteLog(RecvBuff);;
	   }
	   else
	   {
			Sleep(10);
	   }
	  
   }
   pMainDlg->RecEvevt.SetEvent();
   pMainDlg->OnWriteLog("�˳������̣߳�");
   return 0;

}

bool CNetTestDlg::SendData(LPCTSTR buf,int buflens)
{
	int retVal=0;
	if(strlen(buf)==0)
		return false;
	if(m_bConning)
	{
		retVal=send(m_Client,buf,buflens,NULL);

		if (SOCKET_ERROR == retVal)
		{
			int nErrCode = WSAGetLastError();//�������
			if (WSAEWOULDBLOCK == nErrCode)
			{
			
			}
			else if(WSAENETDOWN == nErrCode || WSAETIMEDOUT == nErrCode || WSAECONNRESET == nErrCode)
			{
				return FALSE;
			}
		}

	}
	return TRUE;
}



void CNetTestDlg::OnBnClickedSenddata()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	char DataTemp[100]={0};
	int datalen=0;
	//::GetDlgItemText(m_hWnd,IDC_DATANET,DataTemp,100);
	GetDlgItem(IDC_CLIENTDATA)->GetWindowText(DataTemp,100);
	datalen=strlen(DataTemp);
	SendData(DataTemp,datalen);
}





bool CNetTestDlg::InitClient(void)
{
	int reVal=0;
	WSADATA wsData;
	reVal=WSAStartup(MAKEWORD(2,2),&wsData);

	s_Client = socket(AF_INET, SOCK_STREAM, 0);
	if(INVALID_SOCKET==s_Client)
	{
		WSACleanup();
		return false;
	}

	unsigned long ul = 1;
	reVal = ioctlsocket(s_Client, FIONBIO, (unsigned long*)&ul);
	if (reVal == SOCKET_ERROR)
		return FALSE;

	m_bConnect=TRUE;

	return false;
}


void CNetTestDlg::OnBnClickedInitclient()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CString Str="";
	GetDlgItemText(IDC_INITCLIENT,Str);
	if(Str=="��������")//�Ͽ�
	{
		sConnecting=FALSE;
		WaitForSingleObject(EndEvent,INFINITE);
//		CloseHandle(pRecvThread->m_hThread);
		pRecvThread=NULL;
		SetDlgItemText(IDC_INITCLIENT,"��ʼ������");
		closesocket(s_Client);
		WSACleanup();
		return ;
	}
	CString Port="";
	UINT16 PORT=0;
	byte strip[10]={0};
	char IP[20]={0};
	BYTE nf1,nf2,nf3,nf4;
	m_IpAddr.GetAddress(nf1,nf2,nf3,nf4);
	CString str;
	str.Format("%d.%d.%d.%d",nf1,nf2,nf3,nf4);//�����nf�õ���ֵ��IPֵ

	m_Port.GetWindowText(Port);
	PORT=atoi(Port);

	if(str==""||PORT==0)
	{
		MessageBox("������IP��ַ��˿ںţ�");
		return ;
	}
	memcpy(IP,str.GetBuffer(),str.GetLength());
	str.ReleaseBuffer();
	InitClient();
	ConnectServer(IP,PORT);
}


bool CNetTestDlg::ConnectServer(const char *Ip,UINT16 Port)
{
	 int reVal=0;
	 sockaddr_in m_Addr;
     m_Addr.sin_family=AF_INET;
	 m_Addr.sin_addr.S_un.S_addr=inet_addr(Ip);//�ַ�����ʽ
	 m_Addr.sin_port=htons(Port);

	while(TRUE)
	{
		reVal=connect(s_Client,(sockaddr *)&m_Addr,sizeof(m_Addr));

		if(SOCKET_ERROR == reVal)
		{
			int nErrCode = WSAGetLastError();
			if( WSAEWOULDBLOCK == nErrCode || WSAEINVAL == nErrCode)    //���ӻ�û�����
			{
				Sleep(1000);
				continue;
			}
			else if (WSAEISCONN == nErrCode)//�����Ѿ����//������ģʽ
			{
				if(pRecvThread)
				{
					sConnecting=false;
					WaitForSingleObject(EndEvent,INFINITE);
//					CloseHandle(pRecvThread->m_hThread);
				}
				sConnecting = TRUE;
				pRecvThread=AfxBeginThread(sRecvThread,this,THREAD_PRIORITY_NORMAL,0,0,NULL);
//				pRecvThread->m_hThread=CreateEvent(NULL,FALSE,FALSE,"�����߳�");
				if(pRecvThread==NULL)
				{
					return false;
				}
				SetDlgItemTextA(IDC_INITCLIENT,"��������");
	
				break;
			}
			else//����ԭ������ʧ��
			{
				Sleep(1000);
			}
		}
		if ( reVal == 0 )//���ӳɹ� //����ģʽ
		{
			if(pRecvThread)
			{
				sConnecting=false;
				WaitForSingleObject(EndEvent,INFINITE);
			}
			sConnecting = TRUE;
			pRecvThread=AfxBeginThread(sRecvThread,this,THREAD_PRIORITY_NORMAL,0,0,NULL);
			if(pRecvThread==NULL)
			{
				return false;
			}
			break;
		}
	}
	OnWriteLog("�ͻ��������߳��˳���");
	return TRUE;
}

UINT sRecvThread(LPVOID LPARAM)
{
	int reVal=0;
	char RecvBuf[100]={0};
	CNetTestDlg *Dlg=(CNetTestDlg *)LPARAM;
	int RecvLen=0;
	while(Dlg->sConnecting)
	{
		RecvLen=recv(Dlg->s_Client,RecvBuf,sizeof(RecvBuf),NULL);

		if(SOCKET_ERROR ==RecvLen)
		{
			int nErrCode = WSAGetLastError();
			if ( WSAEWOULDBLOCK == nErrCode )	//�������ݻ�����������
			{
				Sleep(20);
				continue;						//����ѭ��
			}
			else if(WSAENETDOWN == nErrCode || WSAETIMEDOUT == nErrCode || WSAECONNRESET == nErrCode) //�ͻ��˹ر�������
			{
				break;							//�߳��˳�
			}
		}
		else if(RecvLen>0)
		{
			Dlg->OnWriteLog(RecvBuf);
		}
		else
		{
			Sleep(100);
		}
	}
	Dlg->EndEvent.SetEvent();
	Dlg->OnWriteLog("�ͻ��˽����߳��˳���");
	return TRUE;
}


int CNetTestDlg::SendDataToServe(char *pBuf,int Lens)
{
	int reVal=0;
	if(pBuf==NULL || !strlen(pBuf))
	{
		return false;
	}
    if(sConnecting)
	{
		reVal=send(s_Client,pBuf,Lens,NULL);

		if (SOCKET_ERROR == reVal)
		{
			int nErrCode = WSAGetLastError();//�������
			if (WSAEWOULDBLOCK == nErrCode)
			{
				return FALSE;
			}
			else if(WSAENETDOWN == nErrCode || WSAETIMEDOUT == nErrCode || WSAECONNRESET == nErrCode)
			{
				return FALSE;
			}
		}

	}
	
	return TRUE;
}


void CNetTestDlg::OnBnClickedSendToServe()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	char pBuf[256]={0};
	int length=0;
	GetDlgItemText(IDC_CLIENTDATA,pBuf,256);
	length=strlen(pBuf);
	SendDataToServe(pBuf,length);

}


int CNetTestDlg::OnWriteLog(char *RecvData)
{
	CString Log="";
	m_RecvEdit.SetSel(-1, -1);
	CTime tm = CTime::GetCurrentTime();
	CString strtm= tm.Format("%H:%M:%S ");
	Log+=strtm;	
	Log.Format("%s\r\n",RecvData);
	m_RecvEdit.ReplaceSel( (LPCTSTR)(Log) );
	return 0;
}
