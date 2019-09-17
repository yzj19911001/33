#include "StdAfx.h"
#include "SerialUtils.h"
SerPortPar g_serPortPar;
BYTE  g_sendbuf[CMDCONTENTLENG];
BYTE  g_recebuf[CMDCONTENTLENG];
typedef struct
{
	BOOL bWrLog;
    BOOL bComOpen;
    BOOL bWrite;
    HANDLE hComm;
    HANDLE hPostMsgEvent;
    CWinThread *pRecvThread;
    OVERLAPPED osRead;
    OVERLAPPED osWrite;

    SerPortPar serPortPar;

    //结构体默认构造函数，初始化变量
    void CommParam()
    {
        void Init();
    }

    void Init()
    {
        this->bComOpen = FALSE;
        this->pRecvThread = NULL;
        this->bWrite = FALSE;
		this->bWrLog = FALSE;
        this->serPortPar.Init();
    }
} CommParam;

CommParam g_ComPar;

BOOL InitComm(int nPort)
{
    g_ComPar.Init();

    if ((g_ComPar.hPostMsgEvent == CreateEvent(NULL,TRUE,TRUE,NULL)))
        return FALSE;
    memset(&g_ComPar.osRead,0,sizeof(OVERLAPPED));
    memset(&g_ComPar.osWrite,0,sizeof(OVERLAPPED));
    if ((g_ComPar.osRead.hEvent == CreateEvent(NULL,TRUE,FALSE,NULL)))
        return FALSE;
    if ((g_ComPar.osWrite.hEvent == CreateEvent(NULL,TRUE,FALSE,NULL)))
        return FALSE;
    return TRUE;
}

DWORD ReadComm(CommParam *pCP, BYTE *buf, DWORD dwLength)
{
    DWORD length = 0;
    COMSTAT comStat;
    DWORD dwErrorFlags;
    ClearCommError(pCP->hComm,&dwErrorFlags,&comStat);
    length = min(dwLength,comStat.cbInQue);
    ReadFile(pCP->hComm,buf,length,&length,&pCP->osRead);
    return length;
}

DWORD WriteComm(CommParam *pCP, BYTE *buf, DWORD dwLength)
{
    BOOL bState;
    DWORD length = dwLength;
    COMSTAT comStat;
    DWORD dwErrorFlags;

    ClearCommError(pCP->hComm,&dwErrorFlags,&comStat);
    bState = WriteFile(pCP->hComm,buf,length,&length,&pCP->osWrite);
    if (!bState)
    {
        if (GetLastError() == ERROR_IO_PENDING)
        {
            GetOverlappedResult(pCP->hComm,&pCP->osWrite,&length,TRUE);
        }
        else
        {
            length = 0;
        }
    }
    return length;
}

int OnCommRecv(CommParam *pCP)
{
    BYTE buf[MAX_BLOCK/4];
    DWORD dwLength;
    PFCALLBACK_RECV callBack = (PFCALLBACK_RECV)pCP->serPortPar.lpRecvFun;

    if (!pCP->bComOpen)
    {
        SetEvent(pCP->hPostMsgEvent);
        return 0;
    }
    dwLength = ReadComm(pCP,buf,MAX_BLOCK/4);
    if (callBack != NULL)
        (*callBack)(pCP->serPortPar.lpParam,buf,dwLength);

	//写日志
	if (pCP->bWrLog)
	{
		ComWriteLog("[CommRecv]接收",buf,dwLength);
	}
    return 1;
}

UINT CommRecvProc(LPVOID pParam)
{
    OVERLAPPED os;
    DWORD dwMask;
    DWORD dwTrans;
    COMSTAT comStat;
    DWORD dwErrorFlags;
    CommParam *pCP = (CommParam *)pParam;

    memset(&os,0,sizeof(OVERLAPPED));
    os.hEvent = CreateEvent(NULL,TRUE,FALSE,NULL);
    if (os.hEvent == NULL)
        return (UINT)-1;

    while (pCP->bComOpen)
    {
        ClearCommError(pCP->hComm,&dwErrorFlags,&comStat);
        if (comStat.cbInQue)
        {
            WaitForSingleObject(pCP->hPostMsgEvent,INFINITE);
            ResetEvent(pCP->hPostMsgEvent);

            OnCommRecv(pCP);
            continue;
        }
        dwMask = 0;
        if (!WaitCommEvent(pCP->hComm,&dwMask,&os))
        {
            if (GetLastError() == ERROR_IO_PENDING)
            {
                GetOverlappedResult(pCP->hComm,&os,&dwTrans,TRUE);
            }
            else
            {
                CloseHandle(os.hEvent);
                return (UINT)-1;
            }
        }
    }
    CloseHandle(os.hEvent);
    return 0;
}

BOOL OpenComm(SerPortPar *pSerPortPar)
{
    CString sPort;
    CString sBaud;
    COMMTIMEOUTS timeOuts;
    int nPort = pSerPortPar->nPort;
    int nBaud = pSerPortPar->nBaud;

    if (nBaud<0)
        return FALSE;


    InitComm(nPort);

    g_ComPar.serPortPar = *pSerPortPar;
	g_ComPar.bWrLog=pSerPortPar->bWrLog;

    sPort.Format("COM%d",nPort);
    if (nPort >= 9)
        sPort = "\\\\.\\"+sPort;

    g_ComPar.hComm = CreateFile(sPort,GENERIC_READ|GENERIC_WRITE,0,NULL,OPEN_EXISTING,
                                FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OVERLAPPED,NULL);//创建串口设备文件，异步操作
    if (g_ComPar.hComm == INVALID_HANDLE_VALUE)
        return FALSE;

    SetupComm(g_ComPar.hComm,MAX_BLOCK,MAX_BLOCK);//设置windows I/O输入/输出缓存
    SetCommMask(g_ComPar.hComm,EV_RXCHAR);

    timeOuts.ReadIntervalTimeout = MAXDWORD;
    timeOuts.ReadTotalTimeoutMultiplier = 0;
    timeOuts.ReadTotalTimeoutConstant = 0;
    timeOuts.WriteTotalTimeoutMultiplier = 50;
    timeOuts.WriteTotalTimeoutConstant = 2000;
    SetCommTimeouts(g_ComPar.hComm,&timeOuts);//设置超时

    DCB dcb;
    if (!GetCommState(g_ComPar.hComm,&dcb))
        return FALSE;

    dcb.fBinary = 1;
    dcb.BaudRate = nBaud;

    dcb.ByteSize = 8;
    dcb.Parity = NOPARITY;
    dcb.StopBits = ONESTOPBIT;

    g_ComPar.serPortPar.nPort = nPort ;
    if (SetCommState(g_ComPar.hComm,&dcb))//配置串口参数
    {
        g_ComPar.pRecvThread = ::AfxBeginThread(CommRecvProc,&g_ComPar,THREAD_PRIORITY_NORMAL,
                                                0,CREATE_SUSPENDED,NULL);
        if (g_ComPar.pRecvThread == NULL)
        {
            CloseHandle(g_ComPar.hComm);
            return FALSE;
        }
        else
        {
            g_ComPar.bComOpen = TRUE;
            g_ComPar.pRecvThread->ResumeThread();
        }
    }
    else
    {
        CloseHandle(g_ComPar.hComm);
        return FALSE;
    }
    return TRUE;
}

BOOL CloseComm(int nPort)
{
    if (nPort<1 || nPort>MAX_PORT_NUM)
        return FALSE;

    nPort--;
    if (g_ComPar.bComOpen)
    {
        SetEvent(g_ComPar.hPostMsgEvent);
        SetCommMask(g_ComPar.hComm,0);
        WaitForSingleObject(g_ComPar.pRecvThread->m_hThread,INFINITE);
        CloseHandle(g_ComPar.hComm);
        InitComm(nPort);
        return TRUE;
    }
    return FALSE;
}

UINT OnCommSend(LPVOID pParam)
{
    CommParam *pCP = (CommParam *)pParam;
    DWORD dwLen;
    int nSendSize;
    PFCALLBACK_SEND callBack = (PFCALLBACK_SEND)pCP->serPortPar.lpSendFun;
    nSendSize = 0;

    while(pCP->bWrite && nSendSize<pCP->serPortPar.nDataBufSize)
    {
        dwLen = MAX_BLOCK/2;
        if (pCP->serPortPar.nDataBufSize-nSendSize < dwLen)
        {
            dwLen = pCP->serPortPar.nDataBufSize - nSendSize;
        }
        nSendSize += WriteComm(pCP,pCP->serPortPar.pDataBuf+nSendSize,dwLen);
    }

    pCP->bWrite = FALSE;
    if (callBack != NULL)
    {
        if (nSendSize < pCP->serPortPar.nDataBufSize)
            (*callBack)(pCP->serPortPar.lpParam,1);
        else
            (*callBack)(pCP->serPortPar.lpParam,0);
    }

	//写日志
	if (pCP->bWrLog)
	{
		ComWriteLog("[CommSend]发送",pCP->serPortPar.pDataBuf,pCP->serPortPar.nDataBufSize);
	}
    return 0;
}

BOOL SendComm(SerPortPar *pSerPortPar)
{
    int nPort;

    nPort = pSerPortPar->nPort;
    if (nPort<1 || nPort>MAX_PORT_NUM)
        return FALSE;

    nPort--;
    g_ComPar.serPortPar.pDataBuf = pSerPortPar->pDataBuf;
    g_ComPar.serPortPar.nDataBufSize = pSerPortPar->nDataBufSize;
    g_ComPar.serPortPar.lpSendFun = pSerPortPar->lpSendFun;
    g_ComPar.bWrite = TRUE;
    if(::AfxBeginThread(OnCommSend,&g_ComPar,THREAD_PRIORITY_NORMAL,0,0,NULL) == NULL)
        return FALSE;
    return TRUE;
}

BOOL StopSendComm(int nPort)
{
    if (nPort<1 || nPort>MAX_PORT_NUM)
        return FALSE;

    nPort--;
    if (g_ComPar.bWrite)
    {
        g_ComPar.bWrite = FALSE;
        return TRUE;
    }
    return FALSE;
}

void ComWriteLog(CString Module,BYTE *pbuf,int len)
{
	int i=0;
	char pRoot[MAX_PATH]={0};//存放路径的变量
	GetCurrentDirectory(MAX_PATH,pRoot);//获取程序的当前目录
	CString FullPath="";
	CString strTmp="";
	CString l_strInfoTxt="";
	FullPath.Format("%s",pRoot);
	CString LogFile = CTime::GetCurrentTime().Format("%Y-%m-%d.dat");
	FullPath = FullPath + "\\"+LogFile;
	FILE *pf= fopen(FullPath,"at+");
	if (pf!=NULL)
	{

		//准备日志内容
		strTmp=CTime::GetCurrentTime().Format("%H:%M:%S ");
		l_strInfoTxt += strTmp;

		strTmp.Format("%s%d位数据：",Module,len);
		l_strInfoTxt +=strTmp;

		for (i=0;i<len;i++)
		{
			strTmp.Format("%02X ",pbuf[i]);
			l_strInfoTxt +=strTmp;
		}

		l_strInfoTxt+="\n";

		fseek(pf,0,SEEK_END);
		fwrite(l_strInfoTxt,strlen(l_strInfoTxt),1,pf);            
		fclose(pf);
	}
}

