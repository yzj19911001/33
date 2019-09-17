
// SeriesDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "Series.h"
//#include "CSeries.h"
#include "SeriesDlg.h"
#include "afxdialogex.h"
#include "SerialUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define UARTRCVBUFSIZE 2048
#define MINLEN_FRAME 8
#define MAXLEN_FRAME 1100

#define PROD_CONFIG_FPGAUPDATE              ((u16)0x0707)	   
#define PROD_RECV_SCANDATA                  ((u16)0x0102)	 

WORD	 g_UartRecv_Index = 0;   
WORD	 g_UartProc_Index = 0;      
BYTE g_u8RcvBuf[UARTRCVBUFSIZE];

CRITICAL_SECTION  g_sendcs;
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


// CSeriesDlg 对话框



CSeriesDlg::CSeriesDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CSeriesDlg::IDD, pParent)
	, m_UartOpenFlag(false)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CSeriesDlg::~CSeriesDlg()
{

}
void CSeriesDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO1, m_ComNum);
	DDX_Control(pDX, IDC_COMBO2, m_BaudRate);
	DDX_Control(pDX, IDC_RICHEDIT22, m_RecvText);
	DDX_Control(pDX, IDC_RICHEDIT21, m_SendText);
	DDX_Control(pDX, IDC_FILEPATH, m_FilePath);
	DDX_Control(pDX, IDC_TCHART1, m_chart);
}

BEGIN_MESSAGE_MAP(CSeriesDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_OPENUART, &CSeriesDlg::OnBnClickedOpenuart)
	ON_BN_CLICKED(IDC_CLOSEUART, &CSeriesDlg::OnBnClickedCloseuart)
	ON_BN_CLICKED(IDC_SELECTFILE, &CSeriesDlg::OnBnClickedSelectfile)
	ON_BN_CLICKED(IDC_SENDBUTTON, &CSeriesDlg::OnBnClickedSendbutton)
	ON_BN_CLICKED(IDC_CLEARTEXT, &CSeriesDlg::OnBnClickedCleartext)
	ON_WM_TIMER()
	ON_STN_CLICKED(IDC_ARRAY, &CSeriesDlg::OnStnClickedArray)
	ON_BN_CLICKED(IDC_ENABLESHOW, &CSeriesDlg::OnBnClickedEnableshow)
END_MESSAGE_MAP()


// CSeriesDlg 消息处理程序

BOOL CSeriesDlg::OnInitDialog()
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

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO:  在此添加额外的初始化代码
	FindCommPort();

	if (m_ComNum.GetCount()>0)
		m_ComNum.SetCurSel(0);
	m_BaudRate.SetCurSel(1);

	//默认收缩对话框
	CRect rectSeparator;
	GetWindowRect(&rectLarge);
	GetDlgItem(IDC_SERPARATOR)->GetWindowRect(&rectSeparator);

	rectSmall.left = rectLarge.left;
	rectSmall.top = rectLarge.top;
	rectSmall.right = rectSeparator.right;
	rectSmall.bottom = rectLarge.bottom;
	SetWindowPos(NULL, 0, 0, rectSmall.Width(), rectSmall.Height(), SWP_NOZORDER);



//	UpdateData(TRUE);
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CSeriesDlg::OnSysCommand(UINT nID, LPARAM lParam)
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
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CSeriesDlg::OnPaint()
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
HCURSOR CSeriesDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CSeriesDlg::FindCommPort()
{
	HKEY hKey;

	LONG nRetVal = RegOpenKeyEx(HKEY_LOCAL_MACHINE,"Hardware\\DeviceMap\\SerialComm", NULL,KEY_READ, &hKey);
	if (nRetVal == ERROR_SUCCESS)
	{
		int i = 0;
		char portName[256], commName[256];
		DWORD dwLong, dwSize;
		while (1)
		{
			dwLong = dwSize = sizeof(portName);
			nRetVal = RegEnumValue(hKey, i, portName, &dwLong, NULL, NULL, (PUCHAR)commName, &dwSize);
			if (nRetVal == ERROR_NO_MORE_ITEMS) // 枚举串口
				break;

			CString strCommName;
			strCommName.Format("%s", commName);
			m_ComNum.AddString(strCommName); // commName：串口名字
			i++;
		}
		if (m_ComNum.GetCount() == 0)
		{
			AfxMessageBox("[HKEY_LOCAL_MACHINE:Hardware\\DeviceMap\\SerialComm]里无串口!");
		}
		RegCloseKey(hKey);
	}
}


void CSeriesDlg::InitVar()
{
	 m_RecvPkg=0;
	 m_SendPkg=0;
	 m_TotalPkg=0;
	 m_FileLength = 0;
	 m_SendDataFlag = FALSE;
	 g_UartRecv_Index = 0;
	 g_UartProc_Index = 0;
	 memset(g_u8RcvBuf, 0, sizeof(g_u8RcvBuf));
	 pBinData.clear();
}
void CSeriesDlg::OnBnClickedOpenuart()
{
	// TODO:  在此添加控件通知处理程序代码
	int nBaud; //波特率
	CString strtmp="";
	CString  strChoosed = "";
	CString   str1="";
	CString str = "";

	if (!m_UartOpenFlag)
	{
		InitializeCriticalSection(&g_sendcs);//初始化临界变量
		InitVar();
		m_BaudRate.GetLBText(m_BaudRate.GetCurSel(), strtmp);
		nBaud = atoi(strtmp.GetString());

		g_serPortPar.nBaud = nBaud;
		g_serPortPar.lpRecvFun = (LPVOID)OnCommRecv;
		g_serPortPar.lpParam = (LPVOID)this;
		g_serPortPar.bWrLog = true;

		m_ComNum.GetLBText(m_ComNum.GetCurSel(), strChoosed);
		str1 = strChoosed.Mid(3, 3);
		g_serPortPar.nPort = _ttoi(str1);

		if (OpenComm(&g_serPortPar))
		{
			m_UartOpenFlag = TRUE;
			str.Format("COM%d口成功打开！", g_serPortPar.nPort);
			WriteLog(&m_RecvText,str);
		}
		else
		{
			AfxMessageBox("串口打开失败！！！");
		}
	}

}


void CSeriesDlg::OnBnClickedCloseuart()
{
	// TODO:  在此添加控件通知处理程序代码
	int nPort = 0;
	CString str = "";
	if (m_UartOpenFlag)
	{
		nPort = g_serPortPar.nPort;
		if (CloseComm(nPort))
		{
			m_UartOpenFlag = FALSE;
			str.Format("COM%d口成功关闭", nPort);
			WriteLog(&m_RecvText, str);
		}
		else
		{
			AfxMessageBox("串口关闭失败！！！");
		}
	}
}


void CSeriesDlg::OnBnClickedSelectfile()
{
	// TODO:  在此添加控件通知处理程序代码

	CString filepath, l_filename = "";
	CString l_strTmp = "";
	char l_strFilter[] = "Bin File(*.rbf)|*.rbf; *.rbf|All Files(*.*)|*.*|| ";

	CFileDialog fileDlg(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, l_strFilter);
	if (IDOK == fileDlg.DoModal())
	{
//		GetDlgItem(IDC_FILEPATH)->SetWindowText(fileDlg.GetPathName());
		m_FilePath.SetWindowTextA(fileDlg.GetPathName());
	}

}

void OnCommRecv(LPVOID lpParam, u8 *pDataBuf, int nDataBufSize)
{
	BOOL l_bLaserFrame = FALSE;
	int  JGLenCheck = 0;
	int i = 0;
	CString str = "";
	CString strtmp = "";
	char str1[10] = { 0 };
	CSeriesDlg *pMainWnd = (CSeriesDlg*)AfxGetApp()->m_pMainWnd;

	for (i = 0; i<nDataBufSize; i++)
	{
		g_u8RcvBuf[(g_UartRecv_Index + i)&(UARTRCVBUFSIZE - 1)] = pDataBuf[i];
  //      _ltoa(pDataBuf[i],str1,16);
		//str.Format("%s ", str1);
		//strtmp += str;
	}

//	pMainWnd->WriteLog(&pMainWnd->m_RecvText, strtmp);
	g_UartRecv_Index = (g_UartRecv_Index + nDataBufSize) % UARTRCVBUFSIZE;

	pMainWnd->RecComData();

}

void CSeriesDlg::RecComData(void)
{
	int i = 0;
	int tmp_len = 0;
	unsigned short tmp_in = 0;
	unsigned short tmp_out = 0;
	unsigned short l_u32RealRecLen = 0;
	BYTE UartRecvBuf[UARTRCVBUFSIZE] = { 0 };
	tmp_in = g_UartRecv_Index;
	tmp_out = g_UartProc_Index;

	while (tmp_out != tmp_in)
	{
		if (tmp_in>tmp_out)
		{
			tmp_len = tmp_in - tmp_out;
		}
		else
		{
			tmp_len = tmp_in + UARTRCVBUFSIZE - tmp_out;
		}

		if (tmp_len<MINLEN_FRAME)
		{
			break;
		}
		if ((0xFF == g_u8RcvBuf[tmp_out&(UARTRCVBUFSIZE - 1)]) && (0xAA == g_u8RcvBuf[(tmp_out + 1)&(UARTRCVBUFSIZE - 1)]))
		{

			l_u32RealRecLen = (g_u8RcvBuf[(tmp_out + 2)&(UARTRCVBUFSIZE - 1)] << 8) + g_u8RcvBuf[(tmp_out + 3)&(UARTRCVBUFSIZE - 1)] + 4; //
			if (tmp_len >= l_u32RealRecLen)
			{
				memset(UartRecvBuf, 0, sizeof(UartRecvBuf));
				for (i = 0; i < l_u32RealRecLen; i++)
				{
					UartRecvBuf[i] = g_u8RcvBuf[(tmp_out + i)&(UARTRCVBUFSIZE - 1)];
				}
				if (CHECK_BCC(UartRecvBuf, l_u32RealRecLen-6))
				{
					UartEventProc(UartRecvBuf, l_u32RealRecLen);
					tmp_out = (tmp_out + l_u32RealRecLen)&(UARTRCVBUFSIZE - 1);
				}

			}
			else
			{
				if (l_u32RealRecLen<MAXLEN_FRAME && l_u32RealRecLen>MINLEN_FRAME)
				{
					break;
				}
				else
				{
					tmp_out = (tmp_out + 1)&(UARTRCVBUFSIZE - 1);
				}
			}
		}
		else if ((0xFF == g_u8RcvBuf[tmp_out&(UARTRCVBUFSIZE - 1)]) && (0xFF == g_u8RcvBuf[(tmp_out + 1)&(UARTRCVBUFSIZE - 1)]))
		{
			l_u32RealRecLen = (g_u8RcvBuf[(tmp_out + 2)&(UARTRCVBUFSIZE - 1)] << 8) + g_u8RcvBuf[(tmp_out + 3)&(UARTRCVBUFSIZE - 1)] + 4;
			if (tmp_len >= l_u32RealRecLen)
			{
				//memset(UartRecvBuf, 0, sizeof(UartRecvBuf));
				//for (i = 0; i < l_u32RealRecLen; i++)
				//{
				//	UartRecvBuf[i] = g_u8RcvBuf[(tmp_out + i)&(UARTRCVBUFSIZE - 1)];
				//}
				if (CHECK_BCC(UartRecvBuf, l_u32RealRecLen - 6))
				{
//					UartEventProc(UartRecvBuf, l_u32RealRecLen);
					tmp_out = (tmp_out + l_u32RealRecLen)&(UARTRCVBUFSIZE - 1);
				}

			}
			else
			{
				if (l_u32RealRecLen<MAXLEN_FRAME && l_u32RealRecLen>MINLEN_FRAME)
				{
					break;
				}
				else
				{
					tmp_out = (tmp_out + 1)&(UARTRCVBUFSIZE - 1);
				}
			}

		}
		else
		{
			tmp_out = (tmp_out + 1)&(UARTRCVBUFSIZE - 1);
		}
	}
	g_UartProc_Index = tmp_out;

}

u8 CSeriesDlg::BCC_CHECK(BYTE *p_pu8checkbuf, u16 p_u16CheckLen)
{
	u16 i;
	u8 l_u8BCC = 0;               
	for (i = 0; i<p_u16CheckLen; i++)
	{
		l_u8BCC ^= p_pu8checkbuf[i];
	}
	return l_u8BCC;
}

u8 CSeriesDlg::CHECK_BCC(BYTE *p_pu8checkbuf, u16 Len)
{
	int	i;
	u8 l_u16BCC = 0;

	for (i = 0; i<Len; i++)
	{
		l_u16BCC ^= p_pu8checkbuf[2+i];
	}
	if (l_u16BCC != p_pu8checkbuf[2+Len+1])
	{
		return 0;
	}

	return 1;

}

void CSeriesDlg::UartEventProc(u8 *p_pu8Data, u32 p_u32Len)
{
	u8 l_u8BccValue; 
	u16 *l_pu16netrecbuf;

	if ((0xFF == (p_pu8Data[0] & 0xff)) && (0xAA == (p_pu8Data[1] & 0xff)))   // FFAA
	{
		l_u8BccValue = BCC_CHECK(p_pu8Data + 2, p_u32Len - 6);	   
		if (l_u8BccValue != p_pu8Data[p_u32Len - 3])						 
		{
			return;
		}

		l_pu16netrecbuf = (unsigned short *)p_pu8Data;

		switch (l_pu16netrecbuf[11] & 0xffff)
		{
			case PROD_RECV_SCANDATA:
				UartEven_ShowData(p_pu8Data, p_u32Len);
				break;
			case PROD_CONFIG_FPGAUPDATE:       
				UartEven_FPGAUpdateFunc(p_pu8Data, p_u32Len);
				break;
			default:
				break;
		}
	}
}

void CSeriesDlg::UartEven_FPGAUpdateFunc(u8 *p_pu8Data, u32 u32Len)
{
	u16 pkgnum = 0;
	CString str = "";
	pkgnum = (p_pu8Data[26]<<8 )+p_pu8Data[27];
	str.Format("FPGA程序第%d包下载成功！", pkgnum);
	WriteLog(&m_RecvText, str);
	m_RecvPkg = pkgnum+1;
	if (m_RecvPkg == m_TotalPkg)
	{
		KillTimer(1);
		WriteLog(&m_RecvText, "下载完成！");
		ReBootDev();
		WriteLog(&m_RecvText, "正在重启设备！");
	}

}

void CSeriesDlg::UartEven_ShowData(u8 *p_pu8Data, u32 u32Len)
{
	long i = 0;
	u32 l_DistData[272] = { 0 };
	u32 l_TmpArray[272] = { 0 };
	u8  l_TmpData[600] = { 0 };
	u16 Index = 0;

	if (u32Len < 542)
	{
		return;
	}

	memcpy(l_TmpData, p_pu8Data + 26, 271 * 2);

	((CSeries)m_chart.Series(0)).Clear();

	for (i = 0; i < 271; i++)
	{
		l_DistData[i] = (l_TmpData[Index] << 8) + l_TmpData[Index + 1];
		Index += 2;
		l_TmpArray[i] = i;
	}

//	CSeries LineSeries = (CSeries)m_chart.Series(0);
	//m_chart.get_Header().SetCaption("");

	COleSafeArray XValues;
	COleSafeArray YValues;

	DWORD wLength = 271;
	XValues.Create(VT_I4, 1, &wLength);
	YValues.Create(VT_I4, 1, &wLength);

	for (i = 0; i<271; i++)
	{
		XValues.PutElement(&i, l_TmpArray + i);
		YValues.PutElement(&i, l_DistData + i);
	}

//	VT_I4
	//m_chart.get_Axis().GetLeft().SetAutomatic(TRUE);
	//m_chart.get_Axis().GetBottom().SetAutomatic(TRUE);

//	LineSeries.Clear();



	//for ( i = 0; i < 271; i++)

	//{
	//	((CSeries)m_chart.Series(0)).AddXY((double)i, l_DistData[i], NULL, NULL);
	//}
	((CSeries)m_chart.Series(0)).AddArray(271, YValues, XValues);

//	Sleep(10);


}
void CSeriesDlg::WriteLog(CRichEditCtrl *pREC, CString sLog)
{
	sLog = CTime::GetCurrentTime().Format("%H:%M:%S ") + sLog + "\r\n";
	pREC->SetSel(-1, -1);
	pREC->ReplaceSel(sLog);
	pREC->PostMessage(WM_VSCROLL, SB_BOTTOM, 0);
}


void CSeriesDlg::OnBnClickedSendbutton()
{
	// TODO:  在此添加控件通知处理程序代码
	CString l_FilePath = "";
	char l_FilePathTmp[100] = { 0 };
	CWinThread *pThread ;
	int i = 0;
	if (m_UartOpenFlag)
	{
		pBinData.clear();
		m_FilePath.GetWindowText(l_FilePath);
		if (l_FilePath == "")
		{
			WriteLog(&m_RecvText, "所选文件目录为空！");
			return;
		}
		memcpy(l_FilePathTmp, l_FilePath.GetBuffer(), l_FilePath.GetLength());
		l_FilePath.ReleaseBuffer();

		CFile File(l_FilePathTmp, CFile::modeRead | CFile::typeBinary);
		m_FileLength = File.GetLength();
		char *pData = new char[m_FileLength + 1];
		if (File.Read(pData, m_FileLength))
		{
			for (i = 0; i < m_FileLength; i++)
			{
				pBinData.push_back( pData[i]);
			}
		   delete pData;
			m_SendDataFlag = TRUE;
			pThread =::AfxBeginThread(ThreadProc,(LPVOID) this, THREAD_PRIORITY_NORMAL,0,0, NULL);
			if (pThread == NULL)
			{
				WriteLog(&m_RecvText, "线程创建失败！");
				return;
			}
			File.Close();
		}

		
	}
	else
	{
		AfxMessageBox("串口未打开！");
		return;
	}	
}

UINT ThreadProc(LPVOID pParam)
{
	CSeriesDlg *pDlg = (CSeriesDlg *)pParam;
	u16 l_remain = 0;
	u16 l_length = 0;
	u8 TmpBuf[300] = { 0 };
	CString str = "";

	pDlg->m_TotalPkg = (pDlg->m_FileLength + 255) / 256;
	l_remain = pDlg->m_FileLength % 256;

	str.Format("FPGA程序总共%d包！", pDlg->m_TotalPkg);
	pDlg->WriteLog(&pDlg->m_RecvText, str);

	while (pDlg->m_SendDataFlag &&pDlg->m_UartOpenFlag)
	{
		if ((pDlg->m_SendPkg < pDlg->m_TotalPkg) && (pDlg->m_SendPkg == pDlg->m_RecvPkg))
		{
			if (pDlg->m_SendPkg <= pDlg->m_TotalPkg)
			{
				memcpy(TmpBuf, &pDlg->pBinData[pDlg->m_SendPkg * 256], 256);
				l_length = 256;
			}
			pDlg->FPGAUpDate(TmpBuf, l_length, pDlg->m_SendPkg);

			pDlg->m_SendPkg++;

			KillTimer(pDlg->m_hWnd,1);
			SetTimer(pDlg->m_hWnd, 1, 2000, NULL);
		}
		else
		{
			Sleep(50);
//			pDlg->m_RecvPkg++;
		}

	}
	return 1;
}


void CSeriesDlg::FPGAUpDate(u8 *pSendBuf,u16 length,u16 pkgno)
{
	u8 SendBuf[300] = { 0 };
	u16 Index = 0;

	SendBuf[Index++] = 0xFF;
	SendBuf[Index++] = 0xAA;

	Index = 4;
	SendBuf[Index++] = 0x00;
	SendBuf[Index++] = 0x00;
	SendBuf[Index++] = 0x00;
	SendBuf[Index++] = 0x00;
	SendBuf[Index++] = 0x00;
	SendBuf[Index++] = 0x00;

	SendBuf[Index++] = 0x01;

	SendBuf[Index++] = 0x01;

	SendBuf[Index++] = 0x00;	
	SendBuf[Index++] = 0x05;

	Index += 8;
	SendBuf[Index++] = 0x07;
	SendBuf[Index++] = 0x07;

	SendBuf[Index++] = 0x00;
	SendBuf[Index++] = 0x00;

	SendBuf[Index++] = (((m_FileLength+255)/256)>>8 &0xFF); //总包数
	SendBuf[Index++] = ((m_FileLength + 255) / 256)&0xFF; //总包数

	SendBuf[Index++] =( pkgno>>8)&0xFF; //当前包数
	SendBuf[Index++] = pkgno &0xFF; //当前包数

	memcpy(SendBuf + Index, pSendBuf, length);
	Index += length;

	SendBuf[2] = (Index>>8)&0xFF; //总包数
	SendBuf[3] = Index &0xFF; //当前包数

	u8 l_Bcc = BCC_CHECK(SendBuf + 2, Index - 2);
	SendBuf[Index++] = 0x00; 
	SendBuf[Index++] = l_Bcc; //当前包数

	SendBuf[Index++] = 0xEE;
	SendBuf[Index++] = 0xEE;

	ComSend(SendBuf, Index);
}

void CSeriesDlg::ReBootDev(void)
{
	u8 SendBuf[300] = { 0 };
	u16 Index = 0;

	SendBuf[Index++] = 0xFF;
	SendBuf[Index++] = 0xAA;

	Index = 4;
	SendBuf[Index++] = 0x00;
	SendBuf[Index++] = 0x00;
	SendBuf[Index++] = 0x00;
	SendBuf[Index++] = 0x00;
	SendBuf[Index++] = 0x00;
	SendBuf[Index++] = 0x00;

	SendBuf[Index++] = 0x01;

	SendBuf[Index++] = 0x01;

	SendBuf[Index++] = 0x00;
	SendBuf[Index++] = 0x05;

	Index += 8;
	SendBuf[Index++] = 0x06;
	SendBuf[Index++] = 0x03;

	SendBuf[Index++] = 0x00;
	SendBuf[Index++] = 0x00;

	SendBuf[Index++] = 0x00;
	SendBuf[Index++] = 0x00;
	SendBuf[Index++] = 0x00;
	SendBuf[Index++] = 0x00;


	SendBuf[2] = (Index >> 8) & 0xFF; //总包数
	SendBuf[3] = Index & 0xFF; //当前包数

	u8 l_Bcc = BCC_CHECK(SendBuf + 2, Index - 2);
	SendBuf[Index++] = 0x00;
	SendBuf[Index++] = l_Bcc; //当前包数

	SendBuf[Index++] = 0xEE;
	SendBuf[Index++] = 0xEE;

	ComSend(SendBuf, Index);
}

void CSeriesDlg::ComSend(BYTE *pbuff, int nLen)
{
	if (m_UartOpenFlag)
	{
		//开启定时器
		//进临界区 
		EnterCriticalSection(&g_sendcs);
		memset(g_sendbuf, 0, sizeof(g_sendbuf));
		memcpy(g_sendbuf, pbuff, nLen);
		g_serPortPar.pDataBuf = g_sendbuf;
		g_serPortPar.nDataBufSize = nLen;
		SendComm(&g_serPortPar);
		LeaveCriticalSection(&g_sendcs);
		//已出临界区	
	}

}

void CSeriesDlg::OnBnClickedCleartext()
{
	// TODO:  在此添加控件通知处理程序代码
	m_RecvText.SetSel(0, -1);
	m_RecvText.Clear();
	m_SendText.SetSel(0, -1);
	m_SendText.Clear();
}


void CSeriesDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO:  在此添加消息处理程序代码和/或调用默认值
	switch (nIDEvent)
	{
		case 1:
			m_SendPkg=m_RecvPkg;
//			KillTimer(1);
			break;
		default:
			break;
	}
	CDialogEx::OnTimer(nIDEvent);
}


void CSeriesDlg::OnStnClickedArray()
{
	// TODO:  在此添加控件通知处理程序代码
	CString str;


	if (GetDlgItemText(IDC_ARRAY, str), str == ">>")
	{

		SetDlgItemText(IDC_ARRAY, "<<");
	}
	else
	{
		SetDlgItemText(IDC_ARRAY, ">>");
	}
	if (rectLarge.IsRectNull())
	{
		CRect rectSeparator;
		GetWindowRect(&rectLarge);
		GetDlgItem(IDC_SERPARATOR)->GetWindowRect(&rectSeparator);

		rectSmall.left = rectLarge.left;
		rectSmall.top = rectLarge.top;
		rectSmall.right = rectSeparator.right;
		rectSmall.bottom = rectLarge.bottom;
	}
	if (str == "<<")
	{
		SetWindowPos(NULL, 0, 0, rectSmall.Width(), rectSmall.Height(), SWP_NOMOVE | SWP_NOZORDER);

	}
	else
	{
		SetWindowPos(NULL, 0, 0, rectLarge.Width(), rectLarge.Height(), SWP_NOMOVE | SWP_NOZORDER);
	}

}


void CSeriesDlg::OnBnClickedEnableshow()
{
	// TODO:  在此添加控件通知处理程序代码


}

