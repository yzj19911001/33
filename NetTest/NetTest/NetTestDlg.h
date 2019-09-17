
// NetTestDlg.h : 头文件
//

#pragma once
#include "afxcmn.h"
#include "afxwin.h"


// CNetTestDlg 对话框
class CNetTestDlg : public CDialogEx
{
// 构造
public:
	CNetTestDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_NETTEST_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedInitnet();
	bool InitNetWork(void);
	SOCKET m_Server;
	SOCKET m_Client;
	SOCKET s_Server;
	SOCKET s_Client;
	CEvent AccEvent;
	CEvent RecEvevt;
	bool SendData(LPCTSTR buf,int buflens);
	bool m_bConning;
	bool sConnecting;
	afx_msg void OnBnClickedSenddata();
	bool InitClient(void);
	CWinThread *NetThread;
	CWinThread *RecThread;
	CWinThread *pRecvThread;
	CIPAddressCtrl m_IpAddr;
	CEdit m_Port;
	afx_msg void OnBnClickedInitclient();
	bool ConnectServer(const char *Ip,UINT16 Port);
	int SendDataToServe(char *pBuf,int Lens);
	afx_msg void OnBnClickedSendToServe();
	int OnWriteLog(char *RecvData);
	CRichEditCtrl m_RecvEdit;
	BOOL m_bAccept;
	bool m_bConnect;
	CEvent EndEvent;
};
UINT WaitConnectThread(LPVOID LPARAM);
UINT RecvThread(LPVOID LPARAM);
UINT sRecvThread(LPVOID LPARAM);