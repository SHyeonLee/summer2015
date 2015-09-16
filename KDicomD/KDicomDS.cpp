#include "stdafx.h"
#include "mlConv.h"

KDicomQuery	KDicomDS::m_Query;

KDicomDS::KDicomDS()
{
	m_pView			   = NULL;
   m_bFile           = TRUE;
	m_pFile			   = NULL;
	m_bBuffer		   = FALSE;
   m_bCharacterSet   = FALSE;
   m_bMetaHeader     = FALSE;
   _csCodepage       = 1252;
   m_nTSOrg          = KD_TRANSFER_SYNTAX(1000);
}

KDicomDS::~KDicomDS()
{
	Reset();
}

void KDicomDS::Reset()
{
   KDicomElement * pDE;
	while(!m_listDE.IsEmpty()){
		pDE = m_listDE.RemoveHead();
		delete pDE;
	}

	m_pCur			   = NULL;
	m_strFilePath	   = "";
   m_bCharacterSet   = FALSE;
   m_arrCharacterSet.RemoveAll();
   TransferSyntax    = IMPLICIT_LITTLE;
}

int KDicomDS::GetFileLength()
{
	return m_nFileLength;
}

int KDicomDS::GetStreamSize()
{
	return m_nFileLength - m_nStreamPos;
}

CString KDicomDS::GetFilePath()
{
	return m_strFilePath;
}

void KDicomDS::SetFilePath(CString str)
{
	m_strFilePath = str;
}

void KDicomDS::SetFamily(KDicomElement * pChild, KDicomElement * pParent)
{
	if(pParent == NULL || pChild == NULL)
		return;

	int parent_level	= pParent->GetItemLevel();
	int parent_length	= pParent->GetLength();

	pChild->SetParent(pParent);
	pChild->SetItemLevel(parent_level + 1);
	pParent->SetLength(parent_length + pChild->GetLength() + 8);
}


KDicomElement * KDicomDS::AddElement(KD_TAG tag, KDicomElement * pParent)
{
	KDicomElement * pDE = new KDicomElement(this);
	if(pParent == NULL)
		m_listDE.AddTail(pDE);
	else 
		pParent->m_listDE.AddTail(pDE);

	pDE->SetParent(pParent);

	KD_ATTRIBUTE att = m_Query.GetAttribute(tag);
	if(att.tag.group == 0xFFFF && att.tag.element == 0xFFFF){
		KD_TAG	mask;
		mask.group		= 0xFFFF;
		mask.element	= 0xFFFF;
		pDE->SetTag(tag);
		pDE->SetMask(mask);
		pDE->SetVR(UN);
		pDE->SetchVR("UN");
		pDE->SetMaxVM(-1);
		pDE->SetMinVM(1);
	}
	else{
		pDE->SetTag(att.tag);
		pDE->SetMask(att.mask);
		pDE->SetVR(m_Query.GetVRCode(att.VR));
		pDE->SetchVR(att.VR);
		pDE->SetMaxVM(att.maxVM);
		pDE->SetMinVM(att.minVM);
	}

	return pDE;
}

KDicomElement * KDicomDS::AddElement(unsigned short group, unsigned short element, KDicomElement * pParent)
{
	KD_TAG tag;
	tag.group = group;
	tag.element = element;
	return AddElement(tag, pParent);
}

KDicomElement * KDicomDS::InsertElement(KD_TAG tag)
{
	KDicomElement * pDE = new KDicomElement(this);
	KDicomElement * pIS;
	KD_TAG istag;
	POSITION pos, old_pos;
	pDE->SetTag(tag);

	KD_ATTRIBUTE att = m_Query.GetAttribute(tag);
	if(att.tag.group == 0xFFFF && att.tag.element == 0xFFFF){
		KD_TAG	mask;
		mask.group		= 0xFFFF;
		mask.element	= 0xFFFF;
		pDE->SetTag(tag);
		pDE->SetMask(mask);
		pDE->SetVR(UN);
		pDE->SetchVR("UN");
		pDE->SetMaxVM(-1);
		pDE->SetMinVM(1);
	}
	else{
		pDE->SetTag(att.tag);
		pDE->SetMask(att.mask);
		pDE->SetVR(m_Query.GetVRCode(att.VR));
		pDE->SetchVR(att.VR);
		pDE->SetMaxVM(att.maxVM);
		pDE->SetMinVM(att.minVM);
	}

	// Empty list.
	pos = m_listDE.GetHeadPosition();
	if(pos == NULL){
		m_listDE.AddTail(pDE);
		return pDE;
	}
	// Insert
	int res;
	pos = m_listDE.GetHeadPosition();
	while(pos != NULL){
		old_pos = pos;
		pIS = m_listDE.GetNext(pos);
		istag = pIS->GetTag();		
		if(pIS->GetItemLevel() != 0)
			continue;
		res = Compare(tag, istag);
		if(res > 0)
			continue;
		if(res == 0){
			delete pDE;
			return NULL;		
		}
		m_listDE.InsertBefore(old_pos, pDE);
		return pDE;
	}

	m_listDE.AddTail(pDE);
	return pDE;
}

KDicomElement * KDicomDS::InsertElement(unsigned short group, unsigned short element)
{
	KD_TAG tag;
	tag.group = group;
	tag.element = element;
	return InsertElement(tag);
}

void KDicomDS::DeleteElement(KDicomElement * pDE, KDicomElement * pParent)
{
   CPtrList * pList;
   if(pParent == NULL)
      pList = & m_listDE;
   else
      pList = &(pParent->m_listDE);

	KDicomElement * pDET;
	POSITION p_pos;
	POSITION pos = pList->GetHeadPosition();	
	while(pos != NULL){
		p_pos = pos;
		pDET = (KDicomElement *) pList->GetNext(pos);
		if(pDET == pDE){
			pList->RemoveAt(p_pos);
			delete pDE;
         return;
		}
	}
}

BOOL KDicomDS::DeleteElement(unsigned short group, unsigned short element, KDicomElement * pParent)
{
   KDicomElement * pDE = GetElement(group, element, pParent);
   if(pDE == NULL)
      return FALSE;
   DeleteElement(pDE, pParent);
   return TRUE;
}

KDicomElement * KDicomDS::GetElement(KD_TAG tag, KDicomElement * pParent)
{
	KDicomElement * pDE;
	POSITION pos;
	KD_TAG mytag;

	if(pParent == NULL){
		pos = m_listDE.GetHeadPosition();
		while(pos != NULL){
			pDE = m_listDE.GetNext(pos);
			mytag = pDE->GetTag();
         if(mytag.group == tag.group && mytag.element == tag.element){
				return pDE;
         }
		}
	}
	else{
		pos = pParent->m_listDE.GetHeadPosition();
		while(pos != NULL){
			pDE = pParent->m_listDE.GetNext(pos);
			mytag = pDE->GetTag();
         if(mytag.group == tag.group && mytag.element == tag.element){
				return pDE;
         }
		}
	}

	return NULL;
}

KDicomElement * KDicomDS::GetElement(unsigned short group, unsigned short element, KDicomElement * pParent)
{
	KD_TAG tag;
	tag.group = group;
	tag.element = element;
	return GetElement(tag, pParent);
}

KD_TRANSFER_SYNTAX KDicomDS::GetTransferSyntax()
{
	KDicomElement * pDE;
   if((pDE = GetElement(0x0002,0x0010)) == NULL){
		return IMPLICIT_LITTLE;
   }
	CString str = pDE->GetValueUI(0);
	str.TrimRight(' ');

	if(str.Compare(_T("1.2.840.10008.1.2"))== 0)
		return IMPLICIT_LITTLE;
	else if(str.Compare(_T("1.2.840.10008.1.2.1")) == 0)
		return EXPLICIT_LITTLE;
	else if(str.Compare(_T("1.2.840.10008.1.2.2")) == 0)
		return EXPLICIT_BIG;
	else if(_tcsncmp(str, _T("1.2.840.10008.1.2.4.50"), 22) == 0)
		return JPEG_BASELINE;
	else if(_tcsncmp(str, _T("1.2.840.10008.1.2.4.51"), 22) == 0)
		return JPEG_EXTENDED_2_4;
	else if(_tcsncmp(str, _T("1.2.840.10008.1.2.4.52"), 22) == 0)
		return JPEG_EXTENDED_3_5;
	else if(_tcsncmp(str, _T("1.2.840.10008.1.2.4.53"), 22) == 0)
		return JPEG_SPECTRAL_6_8;
	else if(_tcsncmp(str, _T("1.2.840.10008.1.2.4.54"), 22) == 0)
		return JPEG_SPECTRAL_7_9;
	else if(_tcsncmp(str, _T("1.2.840.10008.1.2.4.55"), 22) == 0)
		return JPEG_FULL_10_12;
	else if(_tcsncmp(str, _T("1.2.840.10008.1.2.4.56"), 22) == 0)
		return JPEG_FULL_11_13;
	else if(_tcsncmp(str, _T("1.2.840.10008.1.2.4.57"), 22) == 0)
		return JPEG_LOSSLESS_14;
	else if(_tcsncmp(str, _T("1.2.840.10008.1.2.4.58"), 22) == 0)
		return JPEG_LOSSLESS_15;
	else if(_tcsncmp(str, _T("1.2.840.10008.1.2.4.59"), 22) == 0)
		return JPEG_EXTENDED_16_18;
	else if(_tcsncmp(str, _T("1.2.840.10008.1.2.4.60"), 22) == 0)
		return JPEG_EXTENDED_17_19;
	else if(_tcsncmp(str, _T("1.2.840.10008.1.2.4.61"), 22) == 0)
		return JPEG_SPECTRAL_20_22;
	else if(_tcsncmp(str, _T("1.2.840.10008.1.2.4.62"), 22) == 0)
		return JPEG_SPECTRAL_21_23;
	else if(_tcsncmp(str, _T("1.2.840.10008.1.2.4.63"), 22) == 0)
		return JPEG_FULL_24_26;
	else if(_tcsncmp(str, _T("1.2.840.10008.1.2.4.64"), 22) == 0)
		return JPEG_FULL_25_27;
	else if(_tcsncmp(str, _T("1.2.840.10008.1.2.4.65"), 22) == 0)
		return JPEG_LOSSLESS_28;
	else if(_tcsncmp(str, _T("1.2.840.10008.1.2.4.66"), 22) == 0)
		return JPEG_LOSSLESS_29;
	else if(_tcsncmp(str, _T("1.2.840.10008.1.2.4.70"), 22) == 0)
		return JPEG_LOSSLESS_FIRST_14;
	else if(_tcsncmp(str, _T("1.2.840.10008.1.2.4.90"), 22) == 0)
		return JPEG_2000_IMAGE_COMPRESSON_LOSSLESS_ONLY;
	else if(_tcsncmp(str, _T("1.2.840.10008.1.2.4.91"), 22) == 0)
		return JPEG_2000_IMAGE_COMPRESSON;
	else if(_tcsncmp(str, _T("1.2.840.10008.1.2.5"), 19) == 0)
		return RLE;
	else
		return IMPLICIT_LITTLE;
}

CString KDicomDS::GetTransferSyntaxUID()
{
   KDicomElement * pDE;
   if((pDE = GetElement(0x0002,0x0010)) == NULL)
      return GetTransferSyntaxUID(GetTransferSyntax());
   CString str = pDE->GetValueUI(0);
   str.TrimRight(' ');
   return str;
}

CString KDicomDS::GetTransferSyntaxUID(KD_TRANSFER_SYNTAX ts)
{
	CString str;
	switch(ts){
	case IMPLICIT_LITTLE:
		str = "1.2.840.10008.1.2";
		break;
	case EXPLICIT_LITTLE:
		str = "1.2.840.10008.1.2.1";
		break;
	case EXPLICIT_BIG:
		str = "1.2.840.10008.1.2.2";
		break;
	case JPEG_BASELINE:
		str = "1.2.840.10008.1.2.4.50";
		break;
	case JPEG_EXTENDED_2_4:
		str = "1.2.840.10008.1.2.4.51";
		break;
	case JPEG_EXTENDED_3_5:
		str = "1.2.840.10008.1.2.4.52";
		break;
	case JPEG_SPECTRAL_6_8:
		str = "1.2.840.10008.1.2.4.53";
		break;
	case JPEG_SPECTRAL_7_9:
		str = "1.2.840.10008.1.2.4.54";
		break;
	case JPEG_FULL_10_12:
		str = "1.2.840.10008.1.2.4.55";
		break;
	case JPEG_FULL_11_13:
		str = "1.2.840.10008.1.2.4.56";
		break;
	case JPEG_LOSSLESS_14:
		str = "1.2.840.10008.1.2.4.57";
		break;
	case JPEG_LOSSLESS_15:
		str = "1.2.840.10008.1.2.4.58";
		break;
	case JPEG_EXTENDED_16_18:
		str = "1.2.840.10008.1.2.4.59";
		break;
	case JPEG_EXTENDED_17_19:
		str = "1.2.840.10008.1.2.4.60";
		break;
	case JPEG_SPECTRAL_20_22:
		str = "1.2.840.10008.1.2.4.61";
		break;
	case JPEG_SPECTRAL_21_23:
		str = "1.2.840.10008.1.2.4.62";
		break;
	case JPEG_FULL_24_26:
		str = "1.2.840.10008.1.2.4.63";
		break;
	case JPEG_FULL_25_27:
		str = "1.2.840.10008.1.2.4.64";
		break;
	case JPEG_LOSSLESS_28:
		str = "1.2.840.10008.1.2.4.65";
		break;
	case JPEG_LOSSLESS_29:
		str = "1.2.840.10008.1.2.4.66";
		break;
	case JPEG_LOSSLESS_FIRST_14:
		str = "1.2.840.10008.1.2.4.70";
		break;
	case JPEG_2000_IMAGE_COMPRESSON_LOSSLESS_ONLY:
		str = "1.2.840.10008.1.2.4.90";
		break;
	case JPEG_2000_IMAGE_COMPRESSON:
		str = "1.2.840.10008.1.2.4.91";
		break;
	case RLE:
		str = "1.2.840.10008.1.2.5";
		break;
	}	
	return str;
}

KD_TRANSFER_SYNTAX KDicomDS::GetTransferSyntax(CString str)
{
	if(str.Compare(_T("1.2.840.10008.1.2")) == 0)
		return IMPLICIT_LITTLE;
	else if(str.Compare(_T("1.2.840.10008.1.2.1")) == 0)
		return EXPLICIT_LITTLE;
	else if(str.Compare(_T("1.2.840.10008.1.2.2")) == 0)
		return EXPLICIT_BIG;
	else if(str.Compare(_T("1.2.840.10008.1.2.4.50")) == 0)
		return JPEG_BASELINE;
	else if(str.Compare(_T("1.2.840.10008.1.2.4.51")) == 0)
		return JPEG_EXTENDED_2_4;
	else if(str.Compare(_T("1.2.840.10008.1.2.4.52")) == 0)
		return JPEG_EXTENDED_3_5;
	else if(str.Compare(_T("1.2.840.10008.1.2.4.53")) == 0)
		return JPEG_SPECTRAL_6_8;
	else if(str.Compare(_T("1.2.840.10008.1.2.4.54")) == 0)
		return JPEG_SPECTRAL_7_9;
	else if(str.Compare(_T("1.2.840.10008.1.2.4.55")) == 0)
		return JPEG_FULL_10_12;
	else if(str.Compare(_T("1.2.840.10008.1.2.4.56")) == 0)
		return JPEG_FULL_11_13;
	else if(str.Compare(_T("1.2.840.10008.1.2.4.57")) == 0)
		return JPEG_LOSSLESS_14;
	else if(str.Compare(_T("1.2.840.10008.1.2.4.58")) == 0)
		return JPEG_LOSSLESS_15;
	else if(str.Compare(_T("1.2.840.10008.1.2.4.59")) == 0)
		return JPEG_EXTENDED_16_18;
	else if(str.Compare(_T("1.2.840.10008.1.2.4.60")) == 0)
		return JPEG_EXTENDED_17_19;
	else if(str.Compare(_T("1.2.840.10008.1.2.4.61")) == 0)
		return JPEG_SPECTRAL_20_22;
	else if(str.Compare(_T("1.2.840.10008.1.2.4.62")) == 0)
		return JPEG_SPECTRAL_21_23;
	else if(str.Compare(_T("1.2.840.10008.1.2.4.63")) == 0)
		return JPEG_FULL_24_26;
	else if(str.Compare(_T("1.2.840.10008.1.2.4.64")) == 0)
		return JPEG_FULL_25_27;
	else if(str.Compare(_T("1.2.840.10008.1.2.4.65")) == 0)
		return JPEG_LOSSLESS_28;
	else if(str.Compare(_T("1.2.840.10008.1.2.4.66")) == 0)
		return JPEG_LOSSLESS_29;
	else if(str.Compare(_T("1.2.840.10008.1.2.4.70")) == 0)
		return JPEG_LOSSLESS_FIRST_14;
   else if(str.Compare(_T("1.2.840.10008.1.2.4.90")) == 0)
      return JPEG_2000_IMAGE_COMPRESSON_LOSSLESS_ONLY;
   else if(str.Compare(_T("1.2.840.10008.1.2.4.91")) == 0)
      return JPEG_2000_IMAGE_COMPRESSON;
	else if(str.Compare(_T("1.2.840.10008.1.2.5")) == 0)
		return RLE;
	return UNKNOWN_TRANSFER_SYNTAX;
}

int KDicomDS::CheckMultiFrame()
{
	KDicomElement * pDE;
	if((pDE = GetElement(0x0028, 0x0008)) != NULL){
		return _ttoi(pDE->GetValueIS(0));	
	}
	return 1;
}

int KDicomDS::Compare(KD_TAG a, KD_TAG b)
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

int KDicomDS::GetStreamStartPos()
{
	return m_nStreamPos;
}

KDicomQuery * KDicomDS::GetQuery()
{
	return & m_Query;
}

void KDicomDS::SetCachePath(CString str)
{
	m_Query.SetCachePath(str);
}

void KDicomDS::CloseDS()
{
	Reset();
}

int KDicomDS::GetFrameCount()
{
	return m_nFrameCount;
}

BOOL KDicomDS::CopyDS(KDicomDS * pDS, BOOL bValue)
{
   KDicomElement * pSrc, * pDst;

   Reset();

   TransferSyntax = pDS->TransferSyntax;
   m_nTSOrg = pDS->m_nTSOrg;

   POSITION pos = pDS->m_listDE.GetHeadPosition();
   while(pos != NULL){
      pSrc = pDS->m_listDE.GetNext(pos);
      pDst = AddElement(pSrc->GetTag());
      CopyElement(pDS, pSrc, pDst, bValue);
   }

   return TRUE;
}

unsigned int KDicomDS::CopyElement(KDicomDS * pSrcDS, KDicomElement * pSrc, KDicomElement * pDst, BOOL bValue)
{
   KD_TAG tag = pSrc->GetTag();

   if(pSrc->m_listDE.GetCount() > 0){
		KDicomElement * pSrcChild, * pDstChild;
		POSITION pos = pSrc->m_listDE.GetHeadPosition();
		while(pos != NULL){
			pSrcChild = pSrc->m_listDE.GetNext(pos);
			pDstChild = new KDicomElement(this);
			pDst->m_listDE.AddTail(pDstChild);
			CopyElement(pSrcDS, pSrcChild, pDstChild, bValue);
		}
	}

	// Tag
	pDst->SetTag(tag);

	// VR
	pDst->SetVR(pSrc->GetVR());
	pDst->SetchVR(pSrc->GetchVR());

   // Length
   if(pSrc->GetLength() == 0xFFFFFFFF){
      pDst->SetLength(0xFFFFFFFF);
      return 0;
   }

   // Value
	if(bValue){
      int length = pSrc->GetLength();
      if((length > 0) && (pSrc->GetTag().group != 0xFFFE) && (pSrc->GetVR() != SQ)){
         unsigned char * temp;
         if((temp = pDst->ValueAlloc(length, 0)) != NULL){
            memcpy(temp, pSrc->m_pValue, length);
         }
      }
      pDst->m_nVM = pSrc->m_nVM;
	}
   else{
      pDst->SetLength(0);
   }
	return 0;
}

void KDicomDS::SetLogPath(CString path, BOOL)
{
	m_strLogPath = path;

	TCHAR temp[MAX_PATH];
	wsprintf(temp, _T("%s\\log_dataset.txt"), m_strLogPath);

	FILE * file;
	file = _tfopen(temp, _T("w"));
	fclose(file);
}

void KDicomDS::WriteLog(CString strMessage, BOOL bShowTime, BOOL bNewLine)
{
	if(m_strLogPath == "")
		return;

	TCHAR temp[MAX_PATH];
	wsprintf(temp, _T("%s\\log_dataset.txt"), m_strLogPath);

	FILE * file;
	file = _tfopen(temp, _T("a"));
	if(bShowTime){
		CTime time = CTime::GetCurrentTime();
		fprintf(file, "%02d:%02d:%02d ", time.GetHour(), time.GetMinute(), time.GetSecond());
	}
	fprintf(file, "%s", strMessage);
	if(bNewLine)
		fprintf(file, "\n");
	fclose(file);
}

CString KDicomDS::GetLocaleString(char * string)
{
   if(string == NULL)
      return _T("");
   if (!m_bCharacterSet || strlen(string) == 0){
      CString res;
      res = string;
      return res;
   }

   CString str;
   str = string;

   if (_csCodepage == 50220)
   {
      TCHAR ISO_IR13_KANA_ES[] = { 0x1B, 0x29, 0x49, 0x00 };
      TCHAR ISO_IR14_ROMAN_ES[] = { 0x1B, 0x28, 0x4A, 0x00 };

      str.Replace(ISO_IR13_KANA_ES, ISO_IR14_ROMAN_ES);

#ifdef UNICODE
      return mlConv()->convStr(string, _csCodepage);
#else
      return mlConv()->convStr(str, _csCodepage, 932);
#endif
   }
   else if (_csCodepage == CP_UTF8)
   {
      DWORD    codePage;
      int   ret = GetLocaleInfo(LOCALE_SYSTEM_DEFAULT, 
         LOCALE_IDEFAULTANSICODEPAGE | LOCALE_RETURN_NUMBER, (LPTSTR)&codePage, 
         sizeof(codePage) / sizeof(TCHAR));
      ASSERT(0 < ret);

#ifdef UNICODE
      return mlConv()->convStr(string, CP_UTF8);
#else
      return mlConv()->convStr(str, CP_UTF8, codePage);
#endif
   }

   return str;
}

BOOL KDicomDS::GetImageData(unsigned char * pBuff, int frame)
{
   KDicomElement * pDE;
   pDE = GetElement(0x7FE0, 0x0010);
   if(pDE == NULL)
      return NULL;

   int frameLength, rawLength = 0;
   unsigned char * pRaw = NULL;
   int step;
   if(m_nBitsAllocated == 8)
      step = m_nWidth * m_nSamplePerPixel;
   else
      step = m_nWidth * 2;

   if(pDE->GetLength() == 0xFFFFFFFF){
      frameLength	= step * m_nHeight;

      KDicomElement * pCL;
      POSITION pos = pDE->m_listDE.FindIndex(frame + 1);
      if(pos){
         pCL = pDE->m_listDE.GetNext(pos);
         rawLength   = pCL->GetLength();
         pRaw        = pCL->GetValueOB();
      }
      if(pRaw == NULL)
         return FALSE;

      // Decompress
      switch(GetTransferSyntax()){
      case JPEG_BASELINE:
         DecodeJpegMem8(pRaw, rawLength, pBuff);
         break;
      case JPEG_EXTENDED_2_4:
         if(m_nBitsAllocated == 8)
            DecodeJpegMem8(pRaw, rawLength, pBuff);
         else
            DecodeJpegMem12(pRaw, rawLength, pBuff);
         break;
      case JPEG_LOSSLESS_14:
      case JPEG_LOSSLESS_FIRST_14:
         if(m_nBitsAllocated == 8)
            DecodeJpegMem8(pRaw, rawLength, pBuff);
         else
            DecodeJpegMem16(pRaw, rawLength, pBuff);
         break;
      }
   }
   else{
      frameLength	= pDE->GetLength() / m_nFrameCount;
      pRaw = pDE->GetValueOB() + frameLength * frame;
      memcpy(pBuff, pRaw, m_nWidth * m_nHeight * 2);
   }
}