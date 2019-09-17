/******************************************************************
* 文件名称：FileUtils.h
* 创建作者：zyy
* 创建时间：2011-8-17 11:25:38
* 功能描述：文件帮助类，支持ini文件，二进制文件和文本文件的读写
* 修改记录：
******************************************************************/

#if !defined(AFX_FILEUTILS_H__91C8D3E0_39CD_449F_878B_BC7DEBF8D74B__INCLUDED_)
#define AFX_FILEUTILS_H__91C8D3E0_39CD_449F_878B_BC7DEBF8D74B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CFileUtils  
{
public:
	CFileUtils();
	virtual ~CFileUtils();
	//common
	//字符串分割
	void Split(CString str, CStringList & strList, const CString & divideCode, const CString & notIncCode); 
	// 判断目录是否存在
	BOOL FolderExists(LPCSTR lpPathName);
	BOOL FileExists(LPCSTR lpFileName,BOOL bCurrentDir = TRUE);
	//创建目录
	BOOL CreateDir(LPCSTR lpPathName);
	//config
	BOOL WriteConfigfileInt(LPCSTR lpAppName, LPCSTR lpKeyName, INT lpInt, LPCSTR lpFileName);
	UINT GetConfigfileInt(LPCSTR lpAppName, LPCSTR lpKeyName, INT nDefault, LPCSTR lpFileName);
	BOOL WriteConfigfileString(LPCSTR lpAppName, LPCSTR lpKeyName, LPCSTR lpString, LPCSTR lpFileName);
	DWORD GetConfigfileString(LPCSTR lpAppName, LPCSTR lpKeyName, LPCSTR lpDefault, LPSTR lpReturnedString, DWORD nSize, LPCSTR lpFileName);
	
	//data
	BOOL WriteDatafile(LPCSTR lpFileName, BOOL bCurrentDir=TRUE, BOOL bAppend=FALSE);
	BOOL ReadDatafile(LPCSTR lpFileName, BOOL bCurrentDir = TRUE);
	DWORD GetDatafileLength();
	BOOL WriteDatafileData(LPVOID lpData, UINT dataSize);
	BOOL ReadDatafileData(LPVOID lpData,UINT dataSize);
	void CloseDatafile();

	//text
	BOOL WriteTxtfile(LPCSTR lpFileName, BOOL bCurrentDir=TRUE, BOOL bAppend=FALSE);
	BOOL WriteTxtfileTxt(LPCSTR lpTxt,UINT txtSize);
	void CloseTxtfile();
	
	LONG SeekDatafile(LONG lOff, UINT nFrom);
	
	// 获取当前目录
	CString GetCurDir();
	// 获取当前可执行文件的完整路径
	CString GetExeFilePath();
	// 通过路径名获取文件名
	static CString GetFileName(LPCSTR lpPathName);
	// 通过文件名获取文件类型
	static CString GetFileType(LPCSTR lpFileName);
	// 删除文件
	BOOL DelFile(LPCSTR lpFileName, BOOL bCurrentDir = TRUE);	
	// 拷贝文件
	BOOL CpyFile(LPCSTR lpExistingFileName, LPCTSTR lpNewFileName, BOOL bFailIfExists, BOOL bCurrentDir = TRUE);

	BOOL RenameFile(LPCTSTR lpOldFileName, LPCTSTR lpNewFileName);
	/**************************************************
	* 函数名称：CString OpenFileDlg
	* 输入参数：5个参数 
	*           参数1:BOOL bOpen=TRUE
	*           参数2:LPCTSTR lpFilter=NULL
	*           参数3:LPCTSTR lpTitle=NULL
	*           参数4:LPCTSTR lpDftPath=NULL
	*           参数5:LPCTSTR lpDftName=NULL
	* 输出参数：static 
	* 函数功能：弹出打开/保存对话框，并返回用户选择的完整路径
	* 创建作者：zyy
	* 创建时间：2011-03-24 14:11:43
	**************************************************/
	static CString OpenFileDlg(BOOL bOpen=TRUE,	LPCTSTR lpFilter=NULL, LPCTSTR lpTitle=NULL, 
							   LPCTSTR lpDftPath=NULL, LPCTSTR lpDftName=NULL);

	static void OpenMultFileDlg(CStringList *pFileNameList, BOOL bOpen=TRUE,	LPCTSTR lpFilter=NULL, LPCTSTR lpTitle=NULL, 
								LPCTSTR lpDftPath=NULL, LPCTSTR lpDftName=NULL);

	static CString OpenDirDlg(HWND hWnd, LPCTSTR lpTitle=NULL, LPCTSTR lpDftPath=NULL);


	/**************************************************
	* 函数名称：CString GetFileVersion
	* 输入参数：1个参数 
	*           参数1:char*  FileName	文件名
	* 输出参数：CString 文件版本 
	* 函数功能：获取文件版本信息
	* 创建作者：zyy
	* 创建时间：2011-8-10 14:38:27
	**************************************************/
	static CString GetFileVersion(char*  FileName);
private:
	//common
	CString m_strCurDir;
	CString m_ExeFilePath;
		
	//config
	
	//data
	CFile * m_DATA_file;
	CArchive * m_DATA_archive;
	
};

#endif // !defined(AFX_FILEUTILS_H__91C8D3E0_39CD_449F_878B_BC7DEBF8D74B__INCLUDED_)
