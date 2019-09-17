
// SeriesDlg.h : 头文件
//

#pragma once
#include "afxwin.h"
#include "afxcmn.h"
#include <vector>
#include "FileUtils.h"
#include "tchart1.h"
#include "CSeries.h"
#include "CTChart.h"

using namespace std;
// CSeriesDlg 对话框
class CSeriesDlg : public CDialogEx
{
// 构造
public:
	CSeriesDlg(CWnd* pParent = NULL);	// 标准构造函数
	virtual ~CSeriesDlg();
// 对话框数据
	enum { IDD = IDD_SERIES_DIALOG };

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
	CComboBox m_ComNum;
	void FindCommPort();
	void InitVar();
	CComboBox m_BaudRate;
	afx_msg void OnBnClickedOpenuart();
	afx_msg void OnBnClickedCloseuart();
	bool m_UartOpenFlag;
	afx_msg void OnBnClickedSelectfile();
	void UartEven_FPGAUpdateFunc(u8 *p_pu8Data, u32 u32Len);
	void UartEventProc(u8 *p_pu8Data, u32 p_u32Len);
	void RecComData(void);
	u8 BCC_CHECK(BYTE *p_pu8checkbuf, u16 p_u16CheckLen);
	void WriteLog(CRichEditCtrl *pREC, CString sLog);
	void FPGAUpDate(u8 *pSendBuf, u16 length, u16 pkgno);
	void UartEven_ShowData(u8 *p_pu8Data, u32 u32Len);
	void ReBootDev(void);
	u8 CHECK_BCC(BYTE *p_pu8checkbuf, u16 Len);
	void ComSend(BYTE *pbuff, int nLen);
	CRichEditCtrl m_RecvText;
	CRichEditCtrl m_SendText;
	afx_msg void OnBnClickedSendbutton();
	CEdit m_FilePath;
	CFileUtils m_File;
	vector<u8> pBinData;
	BOOL m_SendDataFlag;
	int m_FileLength;
	u16 m_RecvPkg;
	u16 m_SendPkg;
	u16 m_TotalPkg;
	CRect rectLarge;
	CRect rectSmall;
	CTchart1 m_chart;
	afx_msg void OnBnClickedCleartext();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnStnClickedArray();

	afx_msg void OnBnClickedEnableshow();
//	CSeries m_lineSeries;
};
void OnCommRecv(LPVOID lpParam, u8 *pDataBuf, int nDataBufSize);
UINT ThreadProc(LPVOID pParam);
