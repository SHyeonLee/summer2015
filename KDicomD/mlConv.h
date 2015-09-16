////////////////////////////////////////////////////////////////////////////
// MlConv.h

#pragma once

#define CINTERFACE
#define COBJMACROS

#include <MLang.h>

#undef COBJMACROS
#undef CINTERFACE


class MlConv
{
public:
   MlConv();

   static MlConv*    instance();
   void              destroy();

   BOOL              init();
   void              release();
   BOOL              mb2Unicode(UINT cpIn, char* strSrc, WCHAR** pStrDst);
   BOOL              unicode2Mb(UINT cpOut, LPCWSTR strSrc, char** pStrDst);
   CStringA          convStr(LPCSTR strSrc, UINT cpIn, UINT cpOut);
   CStringA          convStr(LPCWSTR strSrc, UINT cpOut);
   CStringA convStr(LPCWSTR strSrc);
   CStringW          convStr(LPCSTR strSrc, UINT cpIn);
   CStringW convStr(LPCSTR strSrc);
   void  setDefaultCodePage(UINT cp)      { _cpDef = cp; }

private:
   IMLangConvertCharset*   _pMlcc;
   static MlConv*    _instance;
   static CRITICAL_SECTION _cs;
   static UINT  _cpDef;  // default code page
};

MlConv*           mlConv();
