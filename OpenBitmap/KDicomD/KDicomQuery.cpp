// Unified KDicomQuery class implementation

#include "stdafx.h"
#include <process.h>

#define	KD_VR_COUNT		26
char KD_VR_STRING[KD_VR_COUNT][4] =
	{"AE", "AS", "AT", "CS", "DA",
	 "DS", "DT", "FL", "FD", "IS",
	 "LO", "LT", "OB", "OW", "PN",
	 "SH", "SL", "SQ", "SS", "ST",
	 "TM", "UI", "UL", "UN", "US",
	 "UT"};

KDicomQuery::KDicomQuery()
{
	m_Atts		= NULL;

	m_hModule = ::LoadLibrary(_T("dtkr2.dll"));
	if(m_hModule == NULL)
		AfxMessageBox(_T("Cannot find dtkr2.dll"));
	else
      LoadResource();

	m_nFileCount	= 0;
	TCHAR buff[MAX_PATH];
	GetTempPath(MAX_PATH, buff);
	m_strCachePath.Format(_T("%s"), buff);
}	

KDicomQuery::~KDicomQuery()
{
	::FreeLibrary(m_hModule);

	if(m_Atts != NULL)
		delete[] m_Atts;
}

BOOL KDicomQuery::LoadResource()
{
	HRSRC	hRSrc;
	DWORD	Size;
	HGLOBAL hMem;

	hRSrc	= ::FindResource(m_hModule, _T("IDR_DICTIONARY"), _T("BINARY"));
	Size	= ::SizeofResource(m_hModule, hRSrc);
	hMem	= ::LoadResource(m_hModule, hRSrc);
	m_pView	= (unsigned char *) LockResource(hMem);
	m_pCur  = m_pView;

	// header section
	Read(&m_nNums, sizeof(int), 1);

	// alloc
	m_Atts		=	new KD_ATTRIBUTE[m_nNums];

	// attribute section
	Read(m_Atts, sizeof(KD_ATTRIBUTE), m_nNums);

	::UnlockResource(hMem);
	::FreeResource(hMem);

   /*
   
   TCHAR szCurDir[MAX_PATH];
   ::GetCurrentDirectory(MAX_PATH, szCurDir);
   CString path = szCurDir;
   path += _T("\\dic.bin");

   HANDLE hFile = CreateFile(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

   if(hFile == INVALID_HANDLE_VALUE){
      SetLastError(KE_CANNOT_CREATE_FILE);
      AfxMessageBox(_T("Cannot find dic.bin"));
      exit(0);
      return FALSE;
   }

   HANDLE hFileMap = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
   if(hFileMap == NULL){
      CloseHandle(hFile);
      SetLastError(KE_CANNOT_CREATE_FILE_MAP);
      exit(0);
      return FALSE;
   }

   m_pView = (unsigned char *) MapViewOfFile(hFileMap, FILE_MAP_READ, 0, 0, 0);

   m_pCur  = m_pView;

   // header section
   Read(&m_nNums, sizeof(int), 1);

   // alloc
   m_Atts		=	new KD_ATTRIBUTE[m_nNums];

   // attribute section
   Read(m_Atts, sizeof(KD_ATTRIBUTE), m_nNums);

   UnmapViewOfFile(m_pView);
   CloseHandle(hFileMap);
   CloseHandle(hFile);
   */

	return TRUE;
}

KD_VR_ENUM KDicomQuery::GetVRCode(char * str)
{
	for(int i=0;i<KD_VR_COUNT;i++){
		if(strncmp(KD_VR_STRING[i], str, 2) == 0)
			return (KD_VR_ENUM) i;
	}
	return NF;
}

int KDicomQuery::GetCountAttribute()
{
	return m_nNums;
}

KD_ATTRIBUTE KDicomQuery::GetAttribute(int pos)
{
	return m_Atts[pos];
}

KD_ATTRIBUTE KDicomQuery::GetAttribute(KD_TAG tag)
{
	KD_ATTRIBUTE att;
	memset(&att, 0, sizeof(att));

	unsigned short group = tag.group;
	if(group < 0x0002){
		att.tag.group	= 0xFFFF;
		att.tag.element = 0xFFFF;
		sprintf(att.name, "Private Element");
		return att;
	}

	if(tag.element == 0){
		sprintf(att.name, "Group Length");
		sprintf(att.VR, "UL");
		att.minVM	= 1;
		att.maxVM	= 1;
		att.tag.group = tag.group;
		return att;
	}

	// check private
	if((group % 2) == 1 && group != 0x0001 && group != 0x0003 && group != 0x0005 && group != 0x0007 && group != 0xFFFF){
		att.tag.group	= 0xFFFF;
		att.tag.element = 0xFFFF;
		sprintf(att.name, "Private Element");
		return att;
	}

	int comp;
	int frag = m_nNums / 2;
	int exit = 0;
	int pos = frag;

	att = m_Atts[pos];
	while(1){
		frag = frag / 2;
		if(frag <= 1){
			frag = 1;
			exit ++;
			if(exit > 20){
				memset(&att, 0, sizeof(att));
				att.tag.group	= 0xFFFF;
				att.tag.element = 0xFFFF; 
				return att;
			}
		}
		comp = Compare(tag, att.tag);
		if(comp == 0)
			break;
		else if(comp == 1)
			pos += frag;
		else
			pos -= frag;

		if(pos < 0)
			pos = 0;
		if(pos > m_nNums - 1)
			pos = m_nNums - 1;

		att = m_Atts[pos];
	}

	if(strncmp(att.VR, "RE", 2) == 0)
		sprintf(att.VR, "UN");

	return att;
}

int KDicomQuery::Compare(KD_TAG a, KD_TAG b)
{
	if(a.group > b.group)
		return 1;
	else if(a.group < b.group)
		return -1;
	else{
		if(a.element > b.element)
			return 1;
		else if(a.element < b.element)
			return -1;
		else
			return 0;
	}
}

void KDicomQuery::Read(void * value, int size, int count)
{
	memcpy(value, m_pCur, size * count);
	m_pCur += size * count;
}

void KDicomQuery::SetCachePath(CString str)
{
	m_strCachePath = str;
}

CString KDicomQuery::GetCacheFilePath()
{
	CString str;
	str.Format(_T("%s\\cwk%08d%08d.tmp"), m_strCachePath, _getpid(), m_nFileCount);
	m_nFileCount++;
	return str;
}

