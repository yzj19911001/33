
// Series.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������
#include "config.h"

// CSeriesApp: 
// �йش����ʵ�֣������ Series.cpp
//

class CSeriesApp : public CWinApp
{
public:
	CSeriesApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CSeriesApp theApp;