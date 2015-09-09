////////////////////////////////////////////////////////////////////////////
// mlConv.cpp

#include "stdafx.h"
#include "mlConv.h"

#define CP_UTF16  1200

MlConv*  MlConv::_instance = NULL;
CRITICAL_SECTION MlConv::_cs;
UINT  MlConv::_cpDef = CP_ACP;


MlConv::MlConv() :
_pMlcc(NULL)
{
   CoInitialize(NULL);
}


MlConv* mlConv()
{ 
   return MlConv::instance(); 
}

MlConv* MlConv::instance()
{
   DWORD    codePage;

   if (_instance == NULL)
   {
      _instance = new MlConv;
      if (_instance->init())
      {
         int   ret = GetLocaleInfo(LOCALE_SYSTEM_DEFAULT,
            LOCALE_IDEFAULTANSICODEPAGE | LOCALE_RETURN_NUMBER, (LPTSTR)&codePage,
            sizeof(codePage) / sizeof(TCHAR));
         ASSERT(0 < ret);
         _cpDef = codePage;
      }
   }
   return _instance;
}

void MlConv::destroy()
{
   DeleteCriticalSection(&_cs);

   release();
   CoUninitialize();

   delete _instance;
   _instance = NULL;
}


BOOL MlConv::init()
{
   InitializeCriticalSection(&_cs);

   ASSERT(_pMlcc == NULL);
   HRESULT  hr = CoCreateInstance(CLSID_CMLangConvertCharset, NULL, CLSCTX_ALL,
      IID_IMLangConvertCharset, (VOID **)&_pMlcc);
   if (hr != S_OK || !_pMlcc)
      return FALSE;

   return TRUE;
}

void MlConv::release()
{
   if (_pMlcc)
   {
      IMLangConvertCharset_Release(_pMlcc);
      _pMlcc = NULL;
   }
}

BOOL MlConv::mb2Unicode(UINT cpIn, char* strSrc, WCHAR** pStrDst)
{
   UINT  lenSrc;
   UINT  lenDst;

   if (strSrc)
      lenSrc = (UINT)strlen(strSrc);
   else
      lenSrc = 0;
   lenDst = lenSrc;
   ASSERT(pStrDst);
   *pStrDst = new WCHAR[lenDst +1];

   EnterCriticalSection(&_cs);

   ASSERT(_pMlcc);
   if (IMLangConvertCharset_Initialize(_pMlcc, cpIn, CP_UTF16, 0) == S_OK)
   {
      if (IMLangConvertCharset_DoConversionToUnicode(_pMlcc, strSrc, &lenSrc, *pStrDst, &lenDst) == S_OK)
      {
         (*pStrDst)[lenDst] = L'\0';

         LeaveCriticalSection(&_cs);
         return TRUE;
      }
   }

   LeaveCriticalSection(&_cs);
   return FALSE;
}

BOOL MlConv::unicode2Mb(UINT cpOut, LPCWSTR strSrc, char** pStrDst)
{
   UINT  lenSrc;
   UINT  lenDst;

   ASSERT(pStrDst);
   if (strSrc)
   {
      lenSrc = (UINT)wcslen(strSrc);
      lenDst = WideCharToMultiByte(cpOut, 0, strSrc, lenSrc, 
         *pStrDst, 0, NULL, NULL);
   }
   else
      lenDst = lenSrc = 0;
   *pStrDst = new char[lenDst +1];

   if (lenDst == 0)
      return FALSE;

   if (0 < WideCharToMultiByte(cpOut, 0, (LPCWSTR)strSrc, lenSrc, 
      *pStrDst, lenDst, NULL, NULL))
      (*pStrDst)[lenDst] = '\0';

   return TRUE;
}

CStringA MlConv::convStr(LPCSTR strSrc, UINT cpIn, UINT cpOut)
{
   WCHAR*      unicode = NULL;
   char*       buf;
   char*       strDst = NULL;
   CStringA    str;

   if (!strSrc)
      return "";

   int   size = strlen(strSrc) + 1;
   buf = new char[size];
   strcpy_s(buf, size, strSrc);

   if (mb2Unicode(cpIn, buf, &unicode))
   {
      if (unicode2Mb(cpOut, unicode, &strDst))
      {
         str = strDst;

         delete[] strDst;
         delete[] unicode;
         delete[] buf;

         return str;
      }
      delete[] strDst;
   }
   delete[] unicode;
   delete[] buf;

   return "";
}

CStringA MlConv::convStr(LPCWSTR strSrc, UINT cpOut)
{
   char*       strDst = NULL;
   CStringA    str;

   if (unicode2Mb(cpOut, strSrc, &strDst))
   {
      str = strDst;

      delete[] strDst;
      return str;
   }
   delete[] strDst;

   return "";
}

CStringA MlConv::convStr(LPCWSTR strSrc)
{
   return convStr(strSrc, _cpDef);
}

CStringW MlConv::convStr(LPCSTR strSrc, UINT cpIn)
{
   WCHAR*      unicode = NULL;
   CStringW    str;

   if (strSrc)
   {
      int   size = strlen(strSrc) + 1;
      char* buf = new char[size];
      strcpy_s(buf, size, strSrc);

      if (mb2Unicode(cpIn, buf, &unicode))
      {
         str = unicode;

         delete[] unicode;
         delete[] buf;
         return str;
      }
      delete[] unicode;
      delete[] buf;
   }

   return L"";
}

CStringW MlConv::convStr(LPCSTR strSrc)
{
   return convStr(strSrc, _cpDef);
}