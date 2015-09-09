#include "stdafx.h"

KDicomFileMap::KDicomFileMap()
{
   m_hFileMap  = NULL;
   m_pView		= NULL;
}

KDicomFileMap::~KDicomFileMap()
{
   if(m_pView){
      UnmapViewOfFile(m_pView);
      m_pView = NULL;
   }
   if(m_hFileMap){
      CloseHandle(m_hFileMap);
      m_hFileMap	= NULL;
   }
   if(m_bCreateFile)
      DeleteFile(m_strFilePath);
}

BOOL KDicomFileMap::Create(CString filename, BOOL bReadOnly, int length)
{
   HANDLE hFile;

   m_strFilePath = filename;

   if(length > 0){
      m_bCreateFile = TRUE;
      hFile = CreateFile(m_strFilePath, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
      if(hFile == INVALID_HANDLE_VALUE){
         SetLastError(KE_CANNOT_CREATE_FILE);
         return FALSE;
      }

      m_hFileMap = CreateFileMapping(hFile, NULL, PAGE_READWRITE, 0, length, NULL);
      if(m_hFileMap == NULL){
         CloseHandle(hFile);
         SetLastError(KE_CANNOT_CREATE_FILE_MAP);
         return FALSE;
      }

   }
   else{
      m_bCreateFile = FALSE;
      if(bReadOnly)
         hFile = CreateFile(m_strFilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
      else
         hFile = CreateFile(m_strFilePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
      if(hFile == INVALID_HANDLE_VALUE){
         SetLastError(KE_CANNOT_CREATE_FILE);
         return FALSE;
      }

      if(bReadOnly)
         m_hFileMap = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
      else
         m_hFileMap = CreateFileMapping(hFile, NULL, PAGE_READWRITE, 0, 0, NULL);
      if(m_hFileMap == NULL){
         CloseHandle(hFile);
         SetLastError(KE_CANNOT_CREATE_FILE_MAP);
         return FALSE;
      }
   }

   CloseHandle(hFile);
   return TRUE;
}

unsigned char * KDicomFileMap::Map()
{
   if(m_hFileMap == NULL)
      return NULL;

   if(m_pView != NULL)
      return (unsigned char *) m_pView;

   if(m_bCreateFile){
      m_pView = MapViewOfFile(m_hFileMap, FILE_MAP_WRITE, 0, 0, 0);
//      Beep(500,100);
   }
   else{
      m_pView = MapViewOfFile(m_hFileMap, FILE_MAP_READ, 0, 0, 0);
//    Beep(500,100);
   }
   if(m_pView == NULL){
      CloseHandle(m_hFileMap);
      SetLastError(KE_CANNOT_MAP_VIEW_OF_FILE);
      return NULL;
   }

   return (unsigned char *) m_pView;
}

void KDicomFileMap::Unmap()
{
   if(m_pView == NULL)
      return;
   UnmapViewOfFile(m_pView);
   m_pView = NULL;

   if(m_bCreateFile){
//      Beep(900,100);
   }
   else{
//    Beep(900,100);
   }
}
