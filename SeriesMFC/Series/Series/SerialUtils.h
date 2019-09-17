#ifndef _SERIL_H
#define  _SERIL_H


#define CMDCONTENTLENG           1024 
typedef struct
{
    int nPort;
    int nBaud;
    BYTE *pDataBuf;
    int nDataBufSize;
    LPVOID lpRecvFun;
    LPVOID lpSendFun;
    LPVOID lpParam;
	bool bWrLog;

    //结构体默认构造函数，初始化变量
    void SerPortPar()
    {
        void Init();
    }

    void Init()
    {
        this->nPort = 0;
        this->nBaud = 0;
        this->lpRecvFun = 0;
        this->lpSendFun = 0;
        this->pDataBuf = NULL;
        this->lpParam = FALSE;
		this->bWrLog=FALSE;
    }
} SerPortPar;


const int MAX_BLOCK = 10000;
const int MAX_PORT_NUM = 20;

typedef void (*PFCALLBACK_RECV)(LPVOID lpParam, BYTE *pDataBuf, int nDataBufSize);
typedef void (*PFCALLBACK_SEND)(LPVOID lpParam, int nStopType);

BOOL OpenComm(SerPortPar *pSerPortPar);
void ComWriteLog(CString Module,BYTE *pbuf,int len);
/**************************************************
* 函数名称：CloseComm
* 输入参数：1个参数
*           参数1:int nPort		串口号
* 输出参数：BOOL
* 函数功能：
* 创建作者：zyy
* 创建时间：2011-8-9 9:38:25
**************************************************/

BOOL CloseComm(int nPort);
BOOL SendComm(SerPortPar *pSerPortPar);
BOOL StopSendComm(int nPort);

extern SerPortPar g_serPortPar;
extern BYTE g_sendbuf[CMDCONTENTLENG];
extern BYTE g_recebuf[CMDCONTENTLENG];

#endif