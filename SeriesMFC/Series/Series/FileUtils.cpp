// FileUtils.cpp: implementation of the CFileUtils class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FileUtils.h"
#include <io.h>
#include <WINVER.H>

#pragma comment(lib,"version.lib")

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFileUtils::CFileUtils()
{
	char strBuff[256];
	GetModuleFileName(NULL,strBuff,256);   //获取当前路径
	m_ExeFilePath.Format("%s",strBuff);
	m_strCurDir = m_ExeFilePath;
	int index = m_strCurDir.ReverseFind('\\');
	m_strCurDir.Delete(index+1,m_strCurDir.GetLength()-index-1);
	m_DATA_archive = NULL;
	m_DATA_file = NULL;
}

CFileUtils::~CFileUtils()
{
	CloseDatafile();
}

BOOL CFileUtils::WriteConfigfileInt( LPCSTR lpAppName, LPCSTR lpKeyName, INT lpInt, LPCSTR lpFileName )
{
	CreateDir(lpFileName);
	CString lpString;
	lpString.Format("%d",lpInt);
	CString sFileName = m_strCurDir + lpFileName;
	return ::WritePrivateProfileString(lpAppName,lpKeyName,lpString,sFileName);   //写入ini文件中相应字段
}

UINT CFileUtils::GetConfigfileInt( LPCSTR lpAppName, LPCSTR lpKeyName, INT nDefault, LPCSTR lpFileName )
{
	CString sFileName = m_strCurDir + lpFileName;
	return ::GetPrivateProfileInt(lpAppName,lpKeyName,nDefault,sFileName);
}

BOOL CFileUtils::WriteConfigfileString( LPCSTR lpAppName, LPCSTR lpKeyName, LPCSTR lpString, LPCSTR lpFileName )
{
	CString sFileName = m_strCurDir + lpFileName;
	return ::WritePrivateProfileString(lpAppName,lpKeyName,lpString,sFileName);
}

DWORD CFileUtils::GetConfigfileString(LPCSTR lpAppName, LPCSTR lpKeyName, LPCSTR lpDefault, LPSTR lpReturnedString, DWORD nSize, LPCSTR lpFileName)
{
	CString sFileName = m_strCurDir + lpFileName;
	return ::GetPrivateProfileString(lpAppName,lpKeyName,lpDefault,lpReturnedString,nSize,sFileName);
}

BOOL CFileUtils::WriteDatafile( LPCSTR lpFileName, BOOL bCurrentDir, BOOL bAppend)
{
	CreateDir(lpFileName);
	CString sFileName = lpFileName;
	if (bCurrentDir)
	{
		sFileName = m_strCurDir + lpFileName;	
	}	

	if (bAppend)
	{
		if (FileExists(sFileName,FALSE))
		{
			m_DATA_file = new CFile(sFileName,CFile::modeWrite);
			m_DATA_file->SeekToEnd();
			m_DATA_archive = new CArchive(m_DATA_file,CArchive::store);
			return TRUE;
		}
	}

	m_DATA_file = new CFile(sFileName,CFile::modeCreate | CFile::modeWrite);
	m_DATA_archive = new CArchive(m_DATA_file,CArchive::store);
	return TRUE;
}

BOOL CFileUtils::ReadDatafile( LPCSTR lpFileName, BOOL bCurrentDir )
{
	CString sFileName = lpFileName;
	if (bCurrentDir)
	{
		sFileName = m_strCurDir + lpFileName;	
	}	
	m_DATA_file = new CFile(sFileName, CFile::modeRead |CFile::modeCreate| CFile::modeNoTruncate/*| CFile::typeBinary*/);
	m_DATA_archive = new CArchive(m_DATA_file,CArchive::load);
	return TRUE;
}

DWORD CFileUtils::GetDatafileLength()
{
	return m_DATA_file->GetLength();
}

BOOL CFileUtils::WriteDatafileData( LPVOID lpData, UINT dataSize)
{
	m_DATA_archive->Write(lpData,dataSize);
	m_DATA_archive->Flush();
	return TRUE;
}

BOOL CFileUtils::ReadDatafileData( LPVOID lpData, UINT dataSize )
{
	m_DATA_archive->Read(lpData,dataSize);	
	return TRUE;
}

void CFileUtils::CloseDatafile()
{
	if(m_DATA_archive)
	{
		m_DATA_archive->Flush();
		m_DATA_archive->Close();
		m_DATA_file->Close();
		delete m_DATA_archive;
		delete m_DATA_file;
		m_DATA_archive = NULL;
		m_DATA_file = NULL;
	} 
}

BOOL CFileUtils::WriteTxtfile( LPCSTR lpFileName, BOOL bCurrentDir, BOOL bAppend)
{
	CreateDir(lpFileName);
	CString sFileName = lpFileName;
	if (bCurrentDir)
	{
		sFileName = m_strCurDir + lpFileName;	
	}	

	if (bAppend)
	{
		if (FileExists(sFileName,FALSE))
		{
			m_DATA_file = new CFile(sFileName,CFile::modeWrite);
			m_DATA_file->SeekToEnd();
			return TRUE;
		}
	}

	m_DATA_file = new CFile(sFileName,CFile::modeCreate | CFile::modeWrite);	
	return TRUE;
}

BOOL CFileUtils::WriteTxtfileTxt(LPCSTR lpTxt,UINT txtSize)
{
	if (m_DATA_file)
	{
		m_DATA_file->Write(lpTxt,txtSize);		
		return TRUE;
	}
	return FALSE;
}

void CFileUtils::CloseTxtfile()
{
	if (m_DATA_file)
	{
		m_DATA_file->Close();
		m_DATA_file = NULL;
	}
}



BOOL CFileUtils::CreateDir( LPCSTR lpPathName )
{
	CStringList sDirNameList;
	CString sSeparator = "\\";
	CString notIncCode = ".";
	CString sDirName;
	CString sPathName;
	BOOL bNeedJudge = TRUE;
	
	Split(lpPathName,sDirNameList,sSeparator,notIncCode);
	
	if (!sDirNameList.IsEmpty())
	{
		sPathName = m_strCurDir;
		POSITION pos = sDirNameList.GetHeadPosition();
		while(pos != NULL)
		{
			sDirName = sDirNameList.GetNext(pos);
			sPathName += sSeparator + sDirName;
			if (bNeedJudge)
			{
				if (FolderExists(sPathName))
				{
					continue;
				}
				bNeedJudge = FALSE;
			}
			CreateDirectory(sPathName,NULL);			
		}
		return TRUE;
	}	

	return FALSE;
	
}

BOOL CFileUtils::FolderExists( LPCSTR lpPathName )
{
	DWORD attr; 
    attr = GetFileAttributes(lpPathName); 
    return (attr != (DWORD)(-1) ) && (attr & FILE_ATTRIBUTE_DIRECTORY); 
}

void CFileUtils::Split(CString str, CStringList & strList, const CString & divideCode, const CString & notIncCode)
{
	CString strValue; 
	strList.RemoveAll();
	while (true) 
	{ 
		strValue = str.SpanExcluding(divideCode); 
		if ((strValue != "") && (strValue.Find(notIncCode) == -1))
		{
			strList.AddTail(strValue); 		
		}
		str = str.Right(str.GetLength() - strValue.GetLength() - 1); 
		if (str.IsEmpty()) 
			break; 
	} 
}

CString CFileUtils::GetCurDir()
{
	return m_strCurDir;
}

CString CFileUtils::GetExeFilePath()
{
	return m_ExeFilePath;
}

LONG CFileUtils::SeekDatafile(LONG lOff, UINT nFrom)
{
	if (m_DATA_file)
	{
		m_DATA_file->Seek(lOff,nFrom);	
		m_DATA_archive->Flush();
	}
	return 0;
}

BOOL CFileUtils::DelFile(LPCSTR lpFileName, BOOL bCurrentDir)
{
	CString sFileName = lpFileName;
	if (bCurrentDir)
		sFileName = m_strCurDir + lpFileName;

	if (access(sFileName,0) == 0)
		return DeleteFile(sFileName);	
	else
		return FALSE;
}

BOOL CFileUtils::CpyFile(LPCSTR lpExistingFileName, LPCTSTR lpNewFileName, BOOL bFailIfExists, BOOL bCurrentDir)
{
	CString sExtFileName = lpExistingFileName;
	if (bCurrentDir)
		sExtFileName = m_strCurDir + sExtFileName;	
	CString sNewFileName = lpNewFileName;
	if (bCurrentDir)
		sNewFileName = m_strCurDir + sNewFileName;
		
	if (access(sExtFileName,0) == 0)
		return CopyFile(sExtFileName,sNewFileName,bFailIfExists);
	else
		return FALSE;
}

CString CFileUtils::GetFileName(LPCSTR lpPathName)
{
	CString fileName = lpPathName;
	int iPos = fileName.ReverseFind('\\');
	if (iPos > 0)
	{
		fileName.Delete(0,iPos+1);
		return fileName;
	}
	else
		return _T("");
}

CString CFileUtils::GetFileType(LPCSTR lpFileName)
{
	CString fileType = lpFileName;
	int iPos = fileType.ReverseFind('.');
	if (iPos > 0)
	{
		fileType.Delete(0,iPos);
		return fileType;
	}
	else
		return _T("");
}

BOOL CFileUtils::FileExists(LPCSTR lpFileName,BOOL bCurrentDir)
{
	CString sFileName = lpFileName;
	if (bCurrentDir)
	{
		sFileName = m_strCurDir + lpFileName;	
	}	
	return (GetFileAttributes(sFileName)!=-1);
}

CString CFileUtils::OpenFileDlg(BOOL bOpen/* =TRUE */, LPCTSTR lpFilter/* =NULL */, LPCTSTR lpTitle/* =NULL */, 
								LPCTSTR lpDftPath/* =NULL */, LPCTSTR lpDftName/* =NULL */)
{
	CString sPath = _T("");
	char BASED_CODE szFilterProgram[] = "所有文件 (*.*)|*.*||";
	if (lpFilter == NULL)
		lpFilter = szFilterProgram;		
	
	CFileDialog fileDlg(bOpen,NULL,lpDftName,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,lpFilter);
	
	if (lpTitle == NULL)
		fileDlg.m_ofn.lpstrTitle = lpTitle;
	if (lpDftPath == NULL)
		fileDlg.m_ofn.lpstrInitialDir = lpDftPath;
	
	if(fileDlg.DoModal() == IDOK)
		sPath = fileDlg.GetPathName();
	
	return sPath;
}

void CFileUtils::OpenMultFileDlg(CStringList *pFileNameList, BOOL bOpen/* =TRUE */, LPCTSTR lpFilter/* =NULL */, LPCTSTR lpTitle/* =NULL */, 
								 LPCTSTR lpDftPath/* =NULL */, LPCTSTR lpDftName/* =NULL */)
{
	char sFileNameBuf[100000];
	CString sPath;	
	char BASED_CODE szFilterProgram[] = "所有文件 (*.*)|*.*||";
	if (lpFilter == NULL)
		lpFilter = szFilterProgram;		
	
	CFileDialog fileDlg(TRUE,NULL,NULL,OFN_ALLOWMULTISELECT|OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,lpFilter);
	//fileOpenBox.m_ofn.lpstrInitialDir = "E:\\zyy\\workspace\\data\\波形\\弯板\\防作弊\\违规行驶波形\\内蒙波形(48015A)\\"; // For Test

	if (lpTitle == NULL)
		fileDlg.m_ofn.lpstrTitle = lpTitle;
	if (lpDftPath == NULL)
		fileDlg.m_ofn.lpstrInitialDir = lpDftPath;
	
	sFileNameBuf[0] = 0;
	fileDlg.m_ofn.lpstrFile = sFileNameBuf; 
	fileDlg.m_ofn.nMaxFile = 100000; 	
	
	pFileNameList->RemoveAll();
	if(fileDlg.DoModal() == IDOK)
	{
		//选取的文件路径列表索引
		POSITION pathPOS;
		//取得第一个路径的位置
		pathPOS = fileDlg.GetStartPosition();		
		while (pathPOS != NULL)
		{
			//获取各个文件路径,如果获取完，则pathPOS==NULL
			sPath = fileDlg.GetNextPathName(pathPOS);
			if (sPath != "")
				pFileNameList->AddTail(sPath);
		}
	}	
}

int CALLBACK BrowseCallbackProc(HWND hwnd,UINT uMsg,LPARAM lParam,LPARAM lpData)  
{
	if(uMsg == BFFM_INITIALIZED)
	{  
		SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
	}
	return 0;  
}

CString CFileUtils::OpenDirDlg(HWND hWnd, LPCTSTR lpTitle/* =NULL */, LPCTSTR lpDftPath/* =NULL */)
{
	CString sPath = _T("");
	BROWSEINFO bi;
	char szPathName[MAX_PATH];
	
	ZeroMemory(&bi, sizeof(BROWSEINFO));
	bi.hwndOwner = hWnd;
	bi.pszDisplayName = szPathName;
	bi.lpszTitle = lpTitle;
	bi.ulFlags = BIF_STATUSTEXT  |   BIF_RETURNONLYFSDIRS;   //0x0040  0x0040会导致在win8下提取异常  
	//设定指定打开窗口 20140508 likang
	CFileFind   finder;
	CString strTemp = lpDftPath;
	if(finder.FindFile(strTemp,0)==FALSE)
	{
		bi.lParam =0;
		bi.lpfn = NULL;
	}
	else
	{
		//bi.lParam = (long)(lpDftPath.GetBuffer(lpDftPath.GetLength()));//初始化路径，形如(_T("c:\\Symbian"));
		bi.lParam = (long)lpDftPath;
		bi.lpfn = BrowseCallbackProc;
    }
	finder.Close();
	
	LPITEMIDLIST idl = SHBrowseForFolder(&bi);
	if(idl == NULL)
		return sPath;
	
	SHGetPathFromIDList(idl, sPath.GetBuffer(MAX_PATH * 2));
	sPath.ReleaseBuffer();
	if(sPath != "" && sPath.GetAt(sPath.GetLength() - 1) != '\\')
		sPath += "\\";
	
	return sPath;
}

CString CFileUtils::GetFileVersion(char* FileName)
{
	int iVerInfoSize;  
    char *pBuf;  
    CString asVer="";  
    VS_FIXEDFILEINFO *pVsInfo;  
    unsigned int iFileInfoSize = sizeof(VS_FIXEDFILEINFO);  
	
	
    iVerInfoSize = GetFileVersionInfoSize(FileName,NULL);   
	
    if(iVerInfoSize!= 0)  
    {     
        pBuf = new char[iVerInfoSize];  
        if(GetFileVersionInfo(FileName,0,iVerInfoSize,pBuf))     
        {     
            if(VerQueryValue(pBuf, "//",(void **)&pVsInfo,&iFileInfoSize))     
            {     
                asVer.Format("%d.%d.%d.%d",HIWORD(pVsInfo->dwFileVersionMS),LOWORD(pVsInfo->dwFileVersionMS),HIWORD(pVsInfo->dwFileVersionLS),LOWORD(pVsInfo->dwFileVersionLS));  
            }     
        }     
        delete   pBuf;     
    }     
    return   asVer;     
}

BOOL CFileUtils::RenameFile(LPCTSTR lpOldFileName, LPCTSTR lpNewFileName)
{
	TCHAR pTo[MAX_PATH]={0};     
    _tcscpy(pTo,lpNewFileName);  
    TCHAR pFrom[MAX_PATH]={0};     
    _tcscpy(pFrom,lpOldFileName);  
	
    SHFILEOPSTRUCT FileOp={0};   
    FileOp.fFlags = FOF_NOCONFIRMATION;  
    FileOp.pFrom = pFrom;   
    FileOp.pTo = pTo;   
    FileOp.wFunc = FO_RENAME;   
    return SHFileOperation(&FileOp) == 0;    
}