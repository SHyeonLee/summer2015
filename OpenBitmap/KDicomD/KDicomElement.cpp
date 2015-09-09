#include "stdafx.h"
#include "mlConv.h"

KDicomElement::KDicomElement(KDicomDS * pDS)
{
	m_nTag.group	= 0xFFFF;
	m_nTag.element	= 0xFFFF;
	m_nVR			   = UN;
	m_nLength		= 0;
	m_nItemLevel	= 0;
	m_nVM			   = -1;
	m_nMaxVM		   = 1;
	m_nMinVM		   = 1;
	m_pValue		   = NULL;
	m_bString		= TRUE;
	m_bAlloc		   = FALSE;
	m_bSwap			= FALSE;
	m_pParent		= NULL;
   m_pDS          = pDS;
   m_nImagePos    = -1;
}

KDicomElement::KDicomElement(unsigned short group, unsigned short element, KDicomQuery * pQuery)
{
	m_nTag.group	= group;
	m_nTag.element	= element;

	KDicomQuery Query;
	KD_ATTRIBUTE att = pQuery->GetAttribute(m_nTag);
	if(att.tag.group == 0xFFFF && att.tag.element == 0xFFFF){
		KD_TAG	mask;
		mask.group		= 0xFFFF;
		mask.element	= 0xFFFF;
		SetMask(mask);
		SetVR(UN);
		SetchVR("UN");
		SetMaxVM(unsigned int(-1));
		SetMinVM(1);
	}
	else{
		SetMask(att.mask);
		SetVR(pQuery->GetVRCode(att.VR));
		SetchVR(att.VR);
		SetMaxVM(att.maxVM);
		SetMinVM(att.minVM);
	}

	m_nLength		= 0;
	m_nItemLevel	= 0;
	m_nVM			= -1;
	m_pValue		= NULL;
	m_bString		= TRUE;
	m_bAlloc		= FALSE;
	m_bSwap			= FALSE;
	m_pParent		= NULL;
}

KDicomElement::~KDicomElement()
{
	if(m_bAlloc)
		delete[] m_pValue;

   KDicomElement * pDE;
	while(!m_listDE.IsEmpty()){
		pDE = m_listDE.RemoveHead();
		delete pDE;
	}
}

void KDicomElement::SetTag(KD_TAG tag)
{
	m_nTag = tag;
}

KD_TAG KDicomElement::GetTag()
{
	return m_nTag;
}

void KDicomElement::SetVR(KD_VR_ENUM vr)
{
	m_nVR = vr;
	if(vr==AE || vr==AS || vr==CS || vr==DA || vr==DS || vr==DT || vr==IS || vr==LO || vr==LT ||
		vr==PN || vr==SH || vr==ST || vr==TM || vr==UI || vr==UT)
		m_bString = TRUE;
	else 
		m_bString = FALSE;
}

KD_VR_ENUM KDicomElement::GetVR()
{
	return m_nVR;
}

void KDicomElement::SetchVR(char * new_chvr)
{
	sprintf(m_szVR, "%s", new_chvr);
}

char * KDicomElement::GetchVR()
{
   return m_szVR;
}

void KDicomElement::SetLength(unsigned int length)
{
	m_nLength = length;
}

unsigned int KDicomElement::GetLength()
{
	return m_nLength;
}

void KDicomElement::SetMask(KD_TAG mask)
{
	m_nMask = mask;	
}

KD_TAG KDicomElement::GetMask()
{
	return m_nMask;
}

void KDicomElement::SetMinVM(int min)
{
	m_nMinVM = min;
}

void KDicomElement::SetMaxVM(int max)
{
	m_nMaxVM = max;
}

int KDicomElement::GetMinVM()
{
	return m_nMinVM;
}

int KDicomElement::GetMaxVM()
{
	return m_nMaxVM;
}

void KDicomElement::SetVM(int vm)
{
	m_nVM = vm;
}

int KDicomElement::GetVM()
{
	return m_nVM;
}

void KDicomElement::SetItemLevel(unsigned level)
{
	m_nItemLevel = level;
}

unsigned KDicomElement::GetItemLevel()
{
	return m_nItemLevel;
}

void KDicomElement::SetParent(KDicomElement * parent)
{
	m_pParent = parent;
}

KDicomElement * KDicomElement::GetParent()
{
	return m_pParent;
}

void KDicomElement::SetValue(unsigned char * value, KD_TRANSFER_SYNTAX ts)
{
	m_pValue = value;

	int vm = 0;
	KD_VR_ENUM vr;
	unsigned int length;

	vr		   = GetVR();
	length	= GetLength();

	if(length == 0)
		return;

   AllocMem(length, 0);
   memcpy(m_pValue, value, length);

	if(vr==AE || vr==AS || vr==CS || vr==DA || vr==DS || vr==DT || vr==IS || vr==LO || vr==LT ||
		vr==PN || vr==SH || vr==ST || vr==TM || vr==UI || vr==UT){ // character string
		char * string = new char[length + 1];
		memcpy(string, value, length);
		string[length] = '\0';
		char seps[] = "\\\0";
		char * token;
		token = strtok(string, seps);
		while(token != NULL){
			vm++;
			token = strtok(NULL, seps);
		}
		delete[] string;
	}
	else if(vr==SS || vr==US)						// 2 byte
		vm = length / 2;
	else if(vr==AT || vr== FL || vr==SL || vr==UL)	// 4 byte
		vm = length / 4;
	else if(vr==FD)									// 8 byte
		vm = length / 8;
	else
		vm = 1;

	SetVM(vm);

	if(ts == EXPLICIT_BIG){
		m_bSwap = TRUE;
	}
}

void KDicomElement::SetValue(FILE * file, KD_TRANSFER_SYNTAX ts)
{
	int vm = 0;
	KD_VR_ENUM vr;
	unsigned int length;

	vr		   = GetVR();
	length	= GetLength();

	if(length == 0)
		return;

   AllocMem(length, 0);
   fread(m_pValue, length, 1, file);

   if(vr==AE || vr==AS || vr==CS || vr==DA || vr==DS || vr==DT || vr==IS || vr==LO || vr==LT ||
      vr==PN || vr==SH || vr==ST || vr==TM || vr==UI || vr==UT){ // character string
         char * string = new char[length + 1];
         memcpy(string, m_pValue, length);
         string[length] = '\0';
         char seps[] = "\\\0";
         char * token;
         token = strtok(string, seps);
         while(token != NULL){
            vm++;
            token = strtok(NULL, seps);
         }
         delete[] string;
   }
   else if(vr==SS || vr==US)						// 2 byte
      vm = length / 2;
   else if(vr==AT || vr== FL || vr==SL || vr==UL)	// 4 byte
      vm = length / 4;
   else if(vr==FD)									// 8 byte
      vm = length / 8;
   else
      vm = 1;

   SetVM(vm);

   if(ts == EXPLICIT_BIG){
      m_bSwap = TRUE;
   }
}

unsigned char * KDicomElement::GetValue()
{
   return m_pValue;
}

CString KDicomElement::GetValueString(int vm)
{
	CString  str;
   LPTSTR   buf;

	if(m_nLength == 0)
		return _T("");

	char * string = new char[m_nLength + 1];
	memcpy(string, m_pValue, m_nLength);
	string[m_nLength] = '\0';

   CString  fullStr = m_pDS->GetLocaleString(string);
   LPTSTR   lcStr = fullStr.GetBuffer(fullStr.GetLength());

   TCHAR seps[] = _T("\\\0");
   buf = _tcstok(lcStr, seps);
   while (vm--)
   {
      buf = _tcstok(NULL, seps);
   }
   str = buf;
   fullStr.ReleaseBuffer();

   str.TrimLeft();
   str.TrimRight();

   delete[] string;
   return str;
}

unsigned char * KDicomElement::GetValueBinary(int vm, int size)
{
	if(m_pValue != NULL)
		return m_pValue + size * vm;
	else
		return NULL;
}

CString KDicomElement::GetValueAE(int vm)
{
	return GetValueString(vm);
}

KD_AS KDicomElement::GetValueAS(int vm)
{
	KD_AS age = {0, YEAR};
   CString str = GetValueString(vm);
   if(str == _T(""))
      return age;

   int length = str.GetLength();

   CString key = str.Right(1);
   BOOL bFind = TRUE;

   if(key == _T("D")){
      age.type = DAY;
   }
   else if(key == _T("W")){
      age.type = WEEK;
   }
   else if(key == _T("M")){
      age.type = MONTH;
   }
   else if(key == _T("Y")){
      age.type = YEAR;
   }
   else{
      bFind = FALSE;
   }

   if(bFind){
      if(length > 1)
         age.num = _ttoi(str.Left(length - 1));
   }
   else{
      age.num = _ttoi(str);
   }  
   
	return age;
}

KD_TAG KDicomElement::GetValueAT(int vm)
{
	KD_TAG tag;

	unsigned short * buff = (unsigned short *) GetValueBinary(vm, 4);
	tag.group	= buff[vm*2];
	tag.element	= buff[vm*2+1];

	return tag;
}

CString KDicomElement::GetValueCS(int vm)
{
	return GetValueString(vm);
}

KD_DA KDicomElement::GetValueDA(int vm)
{
	KD_DA date = {0,0,0};
	CString str = GetValueString(vm);

	if(str.GetLength() < 6)
		return date;

	if(str.GetAt(4) == '.' && str.GetAt(7) == '.'){
		date.year	= _ttoi(str.Left(4));
		date.month	= _ttoi(str.Mid(5,2));
		date.day	= _ttoi(str.Mid(8,2));
	}
	else{
		date.year	= _ttoi(str.Left(4));
		date.month	= _ttoi(str.Mid(4,2));
		date.day	= _ttoi(str.Mid(6,2));
	}

	return date;
}

CString KDicomElement::GetValueDS(int vm)
{
	return GetValueString(vm);
}

KD_DT KDicomElement::GetValueDT(int vm)
{
	KD_DT dt;
	memset(&dt, 0, sizeof(KD_DT));
	CString str = GetValueString(vm);
	int length = str.GetLength();

	if(length >= 4)
		dt.date.year	= _ttoi(str.Mid(0,4));
	if(length >= 6)
		dt.date.month	= _ttoi(str.Mid(4,2));
	if(length >= 8)
		dt.date.day		= _ttoi(str.Mid(6,2));
	if(length >= 10)
		dt.time.hour	= _ttoi(str.Mid(8,2));
	if(length >= 12)
		dt.time.min		= _ttoi(str.Mid(10,2));
	if(length >= 14)
		dt.time.sec		= _ttoi(str.Mid(12,2));
	if(length >= 21)
		dt.time.fraction	= _ttoi(str.Mid(15,6));
	if(length >= 26){
		if(str.GetAt(21) == '+')
			dt.sign = +1;
		else
			dt.sign = -1;
		dt.offset_hour	= _ttoi(str.Mid(22,2));
		dt.offset_min	= _ttoi(str.Mid(24,2));
	}
	return dt;
}

float KDicomElement::GetValueFL(int vm)
{
	float * buff = (float *) GetValueBinary(vm, sizeof(float));
	if(m_bSwap){
		float res;
		unsigned char * dst, * src;
		dst = (unsigned char *) & res;
		src = (unsigned char *) buff;
		* (dst+0) = * (src+3);
		* (dst+1) = * (src+2);
		* (dst+2) = * (src+1);
		* (dst+3) = * (src+0);
		return res;
	}
	else
		return * buff;
}

double KDicomElement::GetValueFD(int vm)
{
	double * buff = (double *) GetValueBinary(vm, sizeof(double));
	if(m_bSwap){
		double res;
		unsigned char * dst, * src;
		dst = (unsigned char *) & res;
		src = (unsigned char *) buff;
		* (dst+0) = * (src+7);
		* (dst+1) = * (src+6);
		* (dst+2) = * (src+5);
		* (dst+3) = * (src+4);
		* (dst+4) = * (src+3);
		* (dst+5) = * (src+2);
		* (dst+6) = * (src+1);
		* (dst+7) = * (src+0);
		return res;
	}
	else
		return * buff;
}

CString KDicomElement::GetValueIS(int vm)
{
	return GetValueString(vm);
}

CString KDicomElement::GetValueLO(int vm)
{
	return GetValueString(vm);
}

CString KDicomElement::GetValueLT(int vm)
{
	return GetValueString(vm);
}

unsigned char * KDicomElement::GetValueOB()
{
	return GetValueBinary(0, 0);
}

unsigned short * KDicomElement::GetValueOW()
{
	return (unsigned short *) GetValueBinary(0, 0);	
}

KD_PN KDicomElement::GetValuePN(int vm)
{
   KD_PN pn;
   CString str = GetValueString(vm);

   int pos, old_pos = 0;
   if((pos = str.Find('=')) != -1){
      CString str1, str2, str3;

      if(pos == 0){
         str1 = str.Mid(1);
      }
      else
         str1 = str.Mid(old_pos, pos - old_pos);
      old_pos = pos+1;
      if((pos = str.Find('=', old_pos)) != -1){
         str2 = str.Mid(old_pos, pos - old_pos);
         old_pos = pos+1;
         str3 = str.Mid(old_pos);
      }
      KD_PN pn1, pn2, pn3;
      pn1 = ParsingPN(str1);
      pn2 = ParsingPN(str2);
      pn3 = ParsingPN(str3);
      pn.family   = pn1.family;
      pn.given    = pn1.given;
      pn.middle   = pn1.middle;
      pn.prefix   = pn1.prefix;
      pn.suffix   = pn1.suffix;

      pn.family2  = pn2.family;
      pn.given2   = pn2.given;
      pn.middle2  = pn2.middle;
      pn.prefix2  = pn2.prefix;
      pn.suffix2  = pn2.suffix;

      pn.family3  = pn3.family;
      pn.given3   = pn3.given;
      pn.middle3  = pn3.middle;
      pn.prefix3  = pn3.prefix;
      pn.suffix3  = pn3.suffix;
      pn.multi = TRUE;
   }
   else{
      pn = ParsingPN(str);
      pn.multi = FALSE;
   }
  
	return pn;
}

KD_PN KDicomElement::ParsingPN(CString str)
{
   KD_PN pn;
   if(str == "")
      return pn;
   int pos, old_pos = 0;
   if((pos = str.Find('^')) != -1){
      pn.family = str.Mid(old_pos, pos - old_pos);
      old_pos = pos+1;
      if((pos = str.Find('^', old_pos)) != -1){
         pn.given = str.Mid(old_pos, pos - old_pos);
         old_pos = pos+1;
         if((pos = str.Find('^', old_pos)) != -1){
            pn.middle = str.Mid(old_pos, pos - old_pos);
            old_pos = pos+1;
            if((pos = str.Find('^', old_pos)) != -1){
               pn.prefix = str.Mid(old_pos, pos - old_pos);
               old_pos = pos+1;
               if((pos = str.Find('^', old_pos)) != -1){
                  pn.suffix = str.Mid(old_pos, pos - old_pos);
                  old_pos = pos+1;
               }
               else{
                  pn.suffix = str.Mid(old_pos);
               }
            }
            else{
               pn.prefix = str.Mid(old_pos);
            }
         }
         else{
            pn.middle = str.Mid(old_pos);
         }
      }
      else{
         pn.given = str.Mid(old_pos);
      }
   }
   else{
      pn.family = str;
   }   
   return pn;
}

CString KDicomElement::GetValueSH(int vm)
{
	return GetValueString(vm);
}

int KDicomElement::GetValueSL(int vm)
{
	int * buff = (int *) GetValueBinary(vm, sizeof(int));
	if(m_bSwap)
		return ntohs(unsigned short(* buff));
	else
		return * buff;
}

short KDicomElement::GetValueSS(int vm)
{
	short * buff = (short *) GetValueBinary(vm, sizeof(short));
	if(m_bSwap)
		return ntohs(* buff);
	else
		return * buff;
}

CString KDicomElement::GetValueST(int vm)
{
	return GetValueString(vm);
}

KD_TM KDicomElement::GetValueTM(int vm)
{
	KD_TM time = {0,0,0};
	CString str = GetValueString(vm);

	if(str.GetLength() < 6)
		return time;

	time.hour	= _ttoi(str.Mid(0, 2));
	time.min		= _ttoi(str.Mid(2, 2));
	time.sec		= _ttoi(str.Mid(4, 2));

	if(str.GetLength() <= 6){
		time.fraction = 0;
		return time;
	}

	if(str.GetAt(6) == '.')
		time.fraction	= _ttoi(str.Mid(7, 4));
	else
		time.fraction	= 0;

	return time;
}

CString KDicomElement::GetValueUI(int vm)
{
	return GetValueString(vm);
}

unsigned KDicomElement::GetValueUL(int vm)
{
	unsigned * buff = (unsigned *) GetValueBinary(vm, sizeof(unsigned));
	if(m_bSwap)
		return ntohl(* buff);
	else
		return * buff;
}

unsigned short KDicomElement::GetValueUS(int vm)
{
	unsigned short * buff = (unsigned short *) GetValueBinary(vm, sizeof(unsigned short));
	if(m_bSwap)
		return ntohs(* buff);
	else
		return * buff;
}

CString KDicomElement::GetValueUT(int vm)
{
	return GetValueString(vm);
}

BOOL KDicomElement::AddValueAE(CString str)
{
	AddValueString(str, 16);
	return TRUE;
}

BOOL KDicomElement::AddValueAS(KD_AS age)
{
	char temp[5];
	char c = 'D';
	switch(age.type){
	case DAY:
		c = 'D';
		break;
	case WEEK:
		c = 'W';
		break;	
	case MONTH:
		c = 'M';
		break;
	case YEAR:
		c = 'Y';
		break;
	}

	sprintf(temp, "%03d%c", age.num, c);
	return AddValueBinary((unsigned char *) temp, 4);
}

BOOL KDicomElement::AddValueAT(KD_TAG tag)
{
	unsigned short temp[2];
	temp[0] = tag.group;
	temp[1] = tag.element;
	return AddValueBinary((unsigned char *) temp, 4);	
}

BOOL KDicomElement::AddValueCS(CString str)
{
	return AddValueString(str,16,0x20);
}

BOOL KDicomElement::AddValueDA(KD_DA date)
{
	// generate
	char temp[9];
	sprintf(temp,"%04d%02d%02d",date.year, date.month, date.day);

	return AddValueBinary((unsigned char *) temp, 8);
}

BOOL KDicomElement::AddValueDS(CString str)
{
	return AddValueString(str,16,0x20);
}

BOOL KDicomElement::AddValueDT(KD_DT dt)
{
	KD_DA	date = dt.date;
	KD_TM	time = dt.time;

	char temp[30];
	char c;
	if(dt.sign < 0)
		c = '-';
	else
		c = '+';
	if(dt.sign == 0){
		sprintf(temp,"%04d%02d%02d%02d%02d%02d.%06d"
		,date.year, date.month, date.day, time.hour, time.min, time.sec, time.fraction);
		return AddValueBinary((unsigned char *)temp, 21);
	}
	else{
		sprintf(temp,"%04d%02d%02d%02d%02d%02d.%06d%c%02d%02d"
		,date.year, date.month, date.day, time.hour, time.min, time.sec, time.fraction, c, dt.offset_hour, dt.offset_min);
		return AddValueBinary((unsigned char *)temp, 26);
	}
}

BOOL KDicomElement::AddValueFL(float data)
{
	return AddValueBinary((unsigned char *) &data, 4);
}

BOOL KDicomElement::AddValueFD(double data)
{
	return AddValueBinary((unsigned char *) &data, 8);
}

BOOL KDicomElement::AddValueIS(CString str)
{
	return AddValueString(str,12);
}

BOOL KDicomElement::AddValueLO(CString str)
{
	return AddValueString(str,64);
}

BOOL KDicomElement::AddValueLT(CString str)
{
	return AddValueString(str,10240);
}

BOOL KDicomElement::AddValueOB(unsigned char * data, int num)
{
	return AddValueBinary(data, num);
}

BOOL KDicomElement::AddValueOW(unsigned short * data, int num)
{
	return AddValueBinary((unsigned char *) data, num * 2);
}

BOOL KDicomElement::AddValueOW(CString filename, int offset)
{
	FILE * file;
	int length;
	fpos_t spos, epos;
	if((file = _tfopen(filename, _T("rb"))) == NULL){
		AfxMessageBox(_T("Cannot open data file"));
		return FALSE;
	}
	fseek(file, offset, SEEK_SET);
	fgetpos(file, &spos);
	fseek(file, 0, SEEK_END);
	fgetpos(file, &epos);
	fseek(file, offset, SEEK_SET);
	length = int(epos - spos);
	unsigned short * data = new unsigned short[length / 2];
	fread(data, sizeof(unsigned short), length / 2, file);
	fclose(file);
	if(!AddValueOW(data, length / 2)){
		delete[] data;
		return FALSE;
	}
	delete[] data;
	return TRUE;
}

BOOL KDicomElement::AddValuePN(KD_PN pn)
{
	CString str = pn.family + _T("^") + pn.given + _T("^") + pn.middle + _T("^") + pn.prefix + _T("^") + pn.suffix;
	BOOL bStop = 0;
	while(!bStop){
		if(str.Right(1) == "^")
			str = str.Left(str.GetLength() - 1);
		else
			bStop = 1;
	}
	return AddValueString(str,64);
}

BOOL KDicomElement::AddValueSH(CString str)
{
	return AddValueString(str,16);
}

BOOL KDicomElement::AddValueSL(int data)
{
	return AddValueBinary((unsigned char *) &data,4);
}

BOOL KDicomElement::AddValueSQ()
{
	return TRUE;
}

BOOL KDicomElement::AddValueSS(short data)
{
	return AddValueBinary((unsigned char *) &data,2);
}

BOOL KDicomElement::AddValueST(CString str)
{
	return AddValueString(str,1024);
}

BOOL KDicomElement::AddValueTM(KD_TM time)
{
	BOOL res;
	if(time.hour > 23)
		time.hour = 0;
	if(time.min > 59)
		time.min = 0;
	if(time.sec > 59)
		time.sec = 0;
	if(time.fraction > 9999)
		time.fraction = 0;
	char temp[16];
	if(time.fraction == 0){
		sprintf(temp, "%02d%02d%02d", time.hour, time.min, time.sec);
		res = AddValueBinary((unsigned char *) temp, 6);
	}
	else{
		sprintf(temp, "%02d%02d%02d.%04d", time.hour, time.min, time.sec, time.fraction);
		temp[11] = 0x20;
		res = AddValueBinary((unsigned char *) temp, 12);
	}

	return res;
}

BOOL KDicomElement::AddValueUI(CString str)
{
	str.Remove(' ');
	return AddValueString(str,128);
}

BOOL KDicomElement::AddValueUL(unsigned data)
{
	return AddValueBinary((unsigned char *) &data,4);
}

BOOL KDicomElement::AddValueUS(unsigned short data)
{
	return AddValueBinary((unsigned char *) &data,2);
}

BOOL KDicomElement::AddValueUT(CString str)
{
	return AddValueString(str, -1);
}

BOOL KDicomElement::AddValueString(CString str, int max_num, int padding)
{
	int length = str.GetLength();
	if(length == 0)
		return FALSE;

   char * temp = new char[length * 3];

#ifdef _UNICODE
   //USES_CONVERSION;
   //sprintf(temp, "%s", W2A(str));
   sprintf(temp, "%s", mlConv()->convStr(str));
#else
   sprintf(temp, "%s", str);
#endif

   // length
   length = strlen(temp);

   // generate
   if(max_num != -1){
      if(length > max_num)
         length = max_num;
   }

   // alloc
   char * buff;
   if((buff = (char* ) ValueAlloc(length, padding)) == NULL)
      return FALSE;
   
   memcpy(buff, temp, length);
   delete[] temp;

   return TRUE;
}

BOOL KDicomElement::AddValueBinary(unsigned char * buff, int length)
{
	// alloc
	unsigned char * temp;
	if((temp = ValueAlloc(length, 0)) == NULL)
		return FALSE;

   // Header
   if(GetTag().group < 0x0008){
      memcpy(temp, buff, length);
      return TRUE;
   }

	// little endian write
   if(m_pDS->m_nTSOrg != EXPLICIT_BIG){
      memcpy(temp, buff, length);
      return TRUE;
   }
  
   // big endian
   KD_VR_ENUM vr = GetVR();

   if(vr != OW && vr != US && vr != SS && vr != AT && vr != UL && vr != SL && vr != FL && vr != FD){
      memcpy(temp, buff, length);
      return TRUE;
   }

   int i;
   if(vr == OW){
      for(i=0;i<length/2;i++){
         temp[i*2]   = buff[i*2+1];
         temp[i*2+1] = buff[i*2];
      }
      return TRUE;
   }

   if(vr==US || vr==SS){					// 2byte
      temp[0] = buff[1];
      temp[1] = buff[0];
   }
   else if(vr==AT){						// 2byte * 2
      memcpy(temp+2, buff, 2);
      memcpy(temp, buff+2, 2);
   }
   else if(vr==UL || vr==SL || vr==FL){	// 4byte
      temp[0] = buff[3];
      temp[1] = buff[2];
      temp[2] = buff[1];
      temp[3] = buff[0];
   }
   else if(vr==FD){						// 8byte
      temp[0] = buff[7];
      temp[1] = buff[6];
      temp[2] = buff[5];
      temp[3] = buff[4];
      temp[4] = buff[3];
      temp[5] = buff[2];
      temp[6] = buff[1];
      temp[7] = buff[0];
   }
	return TRUE;	
}

unsigned char * KDicomElement::ValueAlloc(int length, int padding)
{
	// check VM
//	if(CheckVM() == FALSE)
//		return NULL;

	// Increase VM
	if(m_nVM == - 1)
		m_nVM = 1;
	else
		m_nVM++;
	// check length
	if(length == 0)
		return NULL;
	// first value alloc
	if(m_pValue == NULL){
		m_nLength = length;
		// make even
		if((m_nLength % 2) == 1)
			m_nLength++;
		AllocMem(m_nLength, padding);
		return m_pValue;
	}
	// after first value alloc
	else{
		// find null
		if(IsString() == TRUE && (m_pValue[m_nLength-1] == '\0' || m_pValue[m_nLength-1] == 0x20))
			m_nLength--;
		// backup
		unsigned char * buff = new unsigned char[m_nLength];
		memcpy(buff, m_pValue, m_nLength);
		int buff_length = m_nLength;
		// free memory
		FreeMem();
		// new memory
		m_nLength += length;
		// Prepare deliminator space
		if(IsString())
			m_nLength ++;
		// make even
		if((m_nLength % 2) == 1)
			m_nLength++;
		// alloc
		AllocMem(m_nLength, padding);
		// restore backup
		memcpy(m_pValue, buff, buff_length);
		// write deliminator
		if(IsString())
			m_pValue[buff_length] = '\\';
		// free backup
		delete[] buff;
		// return new buff point
		return m_pValue + buff_length + 1;
	}
}

BOOL KDicomElement::CheckVM()
{
	if(m_nMaxVM == -1)
		return TRUE;
	else if(m_nVM >= m_nMaxVM)
		return FALSE;
	return TRUE;
}

BOOL KDicomElement::IsString()
{
	return m_bString;
}

void KDicomElement::FreeMem()
{
	if(m_pValue != NULL)
		delete[] m_pValue;
	m_pValue = NULL;
}

BOOL KDicomElement::AllocMem(int length, int padding)
{
	if(length <= 0)
		return FALSE;
	m_pValue = new unsigned char[length];
	
   memset(m_pValue, padding, length);
	m_bAlloc = TRUE;
	return TRUE;
}

BOOL KDicomElement::DeleteValue(unsigned int vm)
{
	if(m_nVM <= 0)
		return TRUE;
	else if(m_nVM == 1){
		FreeMem();
		m_nLength = 0;
		m_nVM = 0;
	}
	else{
		unsigned int i;
		unsigned count, new_length, unit_size;
		// backup
		unsigned char * buff = new unsigned char[m_nLength];
		memcpy(buff, m_pValue, m_nLength);
		// find deliminator and position
		int * pos = new int[m_nVM+1];
		for(i=0, count=0, pos[0]=0;i<m_nLength;i++){
			if(m_pValue[i] == '\\'){
				count++;
				pos[count] = i+1;
			}
		}
		if(count == 0){
			unit_size = m_nLength / m_nVM;
			new_length = m_nLength - unit_size;
			// make even
			if((new_length % 2) == 1)
				new_length++;
			// free memory
			FreeMem();
			// alloc
			AllocMem(new_length, 0);
			memset(m_pValue, 0, new_length);
			// copy front buffer
			memcpy(m_pValue, buff, unit_size * vm);
			// copy back buffer
			memcpy(m_pValue + unit_size * vm, buff + unit_size * (vm + 1), m_nLength - unit_size * (vm + 1));
		}
		else if(vm == count){
			pos[count+1] = m_nLength;
			// calc new value length
			new_length = m_nLength - (pos[vm + 1] - pos[vm] + 1);
			// make even
			if((new_length % 2) == 1)
				new_length++;
			// free memory
			FreeMem();
			// alloc
			AllocMem(new_length, 0);
			memset(m_pValue, 0, new_length);
			// copy front buffer only
			memcpy(m_pValue, buff, pos[vm] - 1);
		}
		else{
			pos[count+1] = m_nLength;
			// calc new value length
			new_length = m_nLength - (pos[vm + 1] - pos[vm]);
			// make even
			if((new_length % 2) == 1)
				new_length++;
			// free memory
			FreeMem();
			// alloc
			AllocMem(new_length, 0);
			memset(m_pValue, 0, new_length);
			// copy front buffer
			memcpy(m_pValue, buff, pos[vm]);
			// copy back buffer
			memcpy(m_pValue + pos[vm], buff + pos[vm + 1], m_nLength - pos[vm + 1]);
		}
		// parameters
		m_nVM--;
		m_nLength = new_length;
		// free backup
		delete[] pos;
		delete[] buff;
	}
	return TRUE;
}

