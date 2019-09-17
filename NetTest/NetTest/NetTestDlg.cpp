
// NetTestDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "NetTest.h"
#include "NetTestDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define SERVER_PORT 8080
// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
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


// CNetTestDlg 对话框




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


// CNetTestDlg 消息处理程序

BOOL CNetTestDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
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

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	m_IpAddr.SetAddress(192,168,2,112);
	m_Port.SetWindowText("8080");

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
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

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CNetTestDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CNetTestDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CNetTestDlg::OnBnClickedInitnet()
{
	// TODO: 在此添加控件通知处理程序代码
	InitNetWork();
}


bool CNetTestDlg::InitNetWork(void)
{
	CString Str="";
	GetDlgItemText(IDC_INITNET,Str);
	if(Str =="连接正常" )
	{	
		m_bConning=false;
		m_bAccept=FALSE;
		WaitForSingleObject(AccEvent,INFINITE);//等待线程结束
		WaitForSingleObject(RecEvevt,INFINITE);
		closesocket(m_Server);
		closesocket(m_Client);
		WSACleanup();
		SetDlgItemTextA(IDC_INITNET,"建立服务器");
		return TRUE;
	}
	else if(Str =="正在监听")
	{
		m_bAccept=FALSE;
		WaitForSingleObject(AccEvent,INFINITE);
		closesocket(m_Client);
		WSACleanup();
		SetDlgItemTextA(IDC_INITNET,"建立服务器");
		return TRUE;
	}
	int reVal;
	//初始化Windows Sockets DLL
	WSADATA  wsData;
	reVal = WSAStartup(MAKEWORD(2,2),&wsData);

	//创建套接字
	m_Server = socket(AF_INET, SOCK_STREAM, 0);
	if(INVALID_SOCKET== m_Server)
		return FALSE;

	//设置套接字非阻塞模式
	unsigned long ul = 1;
	reVal = ioctlsocket(m_Server, FIONBIO, (unsigned long*)&ul);
	if (SOCKET_ERROR == reVal)
		return FALSE;

	//绑定套接字
	sockaddr_in serAddr;
	serAddr.sin_family = AF_INET;
	serAddr.sin_port = htons(SERVER_PORT);
	serAddr.sin_addr.S_un.S_addr = INADDR_ANY;
	reVal = bind(m_Server, (struct sockaddr*)&serAddr, sizeof(serAddr));
	if(SOCKET_ERROR == reVal )
		return FALSE;

	//监听
	reVal = listen(m_Server, SOMAXCONN);
	if(SOCKET_ERROR == reVal)
		return FALSE;
	SetDlgItemTextA(IDC_INITNET,"正在监听");
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
			if(nErrCode == WSAEWOULDBLOCK)	//无法立即完成一个非阻挡性套接字操作
			{
//				Sleep(TIMEFOR_THREAD_SLEEP);
				continue;//继续等待
			}
			else
			{
				return 0;//线程退出
			}

	    }else
		{
		  Dlg->m_Client=sClient;
		  if(Dlg->RecThread)
		  {
			 //Dlg->m_bConning=false;
			 //WaitForSingleObject(Dlg->RecEvevt,INFINITE);//等待线程结束
			 Dlg->RecThread=NULL;
		  }
          Dlg->m_bConning=TRUE;
          Dlg->RecThread =AfxBeginThread(RecvThread,(LPVOID)&(Dlg->m_Client),THREAD_PRIORITY_NORMAL,0,0,NULL);
		  if( Dlg->RecThread==NULL)
		  {
			  return FALSE;
		  }
		  SetDlgItemTextA( Dlg->m_hWnd,IDC_INITNET,"连接正常");
		
		}
	}
	Dlg->AccEvent.SetEvent();
	Dlg->OnWriteLog("退出连接线程！");
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
		   if ( WSAEWOULDBLOCK == nErrCode )	//接受数据缓冲区不可用
		   {
			   Sleep(20);
			   continue;						//继续循环
		   }
		   else if(WSAENETDOWN == nErrCode || WSAETIMEDOUT == nErrCode || WSAECONNRESET == nErrCode) //客户端关闭了连接
		   {
			   retVal = FALSE;	//读数据失败
			   break;							//线程退出
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
   pMainDlg->OnWriteLog("退出接收线程！");
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
			int nErrCode = WSAGetLastError();//错误代码
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
	// TODO: 在此添加控件通知处理程序代码
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
	// TODO: 在此添加控件通知处理程序代码
	CString Str="";
	GetDlgItemText(IDC_INITCLIENT,Str);
	if(Str=="连接正常")//断开
	{
		sConnecting=FALSE;
		WaitForSingleObject(EndEvent,INFINITE);
//		CloseHandle(pRecvThread->m_hThread);
		pRecvThread=NULL;
		SetDlgItemText(IDC_INITCLIENT,"初始化网络");
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
	str.Format("%d.%d.%d.%d",nf1,nf2,nf3,nf4);//这里的nf得到的值是IP值

	m_Port.GetWindowText(Port);
	PORT=atoi(Port);

	if(str==""||PORT==0)
	{
		MessageBox("请输入IP地址或端口号！");
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
	 m_Addr.sin_addr.S_un.S_addr=inet_addr(Ip);//字符串格式
	 m_Addr.sin_port=htons(Port);

	while(TRUE)
	{
		reVal=connect(s_Client,(sockaddr *)&m_Addr,sizeof(m_Addr));

		if(SOCKET_ERROR == reVal)
		{
			int nErrCode = WSAGetLastError();
			if( WSAEWOULDBLOCK == nErrCode || WSAEINVAL == nErrCode)    //连接还没有完成
			{
				Sleep(1000);
				continue;
			}
			else if (WSAEISCONN == nErrCode)//连接已经完成//非阻塞模式
			{
				if(pRecvThread)
				{
					sConnecting=false;
					WaitForSingleObject(EndEvent,INFINITE);
//					CloseHandle(pRecvThread->m_hThread);
				}
				sConnecting = TRUE;
				pRecvThread=AfxBeginThread(sRecvThread,this,THREAD_PRIORITY_NORMAL,0,0,NULL);
//				pRecvThread->m_hThread=CreateEvent(NULL,FALSE,FALSE,"接收线程");
				if(pRecvThread==NULL)
				{
					return false;
				}
				SetDlgItemTextA(IDC_INITCLIENT,"连接正常");
	
				break;
			}
			else//其它原因，连接失败
			{
				Sleep(1000);
			}
		}
		if ( reVal == 0 )//连接成功 //阻塞模式
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
	OnWriteLog("客户端连接线程退出！");
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
			if ( WSAEWOULDBLOCK == nErrCode )	//接受数据缓冲区不可用
			{
				Sleep(20);
				continue;						//继续循环
			}
			else if(WSAENETDOWN == nErrCode || WSAETIMEDOUT == nErrCode || WSAECONNRESET == nErrCode) //客户端关闭了连接
			{
				break;							//线程退出
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
	Dlg->OnWriteLog("客户端接收线程退出！");
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
			int nErrCode = WSAGetLastError();//错误代码
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
	// TODO: 在此添加控件通知处理程序代码
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
