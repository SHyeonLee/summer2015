#include "stdafx.h"

BOOL KDicomDS::SaveDS(CString filename, KD_TRANSFER_SYNTAX ts, BOOL header)
{
	fpos_t start, end;

	FILE * file;
	if((file = _tfopen(filename, _T("wb"))) == NULL){
      SetLastError(KE_CANNOT_OPEN_FILE);
		return FALSE;
	}
	fgetpos(file, &start);

	KDicomElement * pDE;
	KD_TAG tag;
	POSITION pos, old_pos;
	TransferSyntax = ts;

	// Delete Meta Info elements & group elements
	pos = m_listDE.GetHeadPosition();
	while(pos != NULL){
		old_pos = pos;
		pDE = m_listDE.GetNext(pos);
		tag = pDE->GetTag();
      // Command Group Length
      if(tag.group == 0x0000)
         continue;
      // Meta Info and Group
		if(tag.group == 0x0002 || tag.element == 0x0000){
			m_listDE.RemoveAt(old_pos);
			delete pDE;
		}
	}

	// Add Meta Info elements.
	if(header)
		AddHeader(ts, _T("DTK"));

	// Encode Main
	EncodeMain(file, header);

	fseek(file, 0, SEEK_END);
	fgetpos(file, &end);
	m_nFileLength = int(end - start);
	fclose(file);

	m_strFilePath = filename;

	return TRUE;
}

BOOL KDicomDS::SaveStream(CString filename, KD_TRANSFER_SYNTAX ts, BOOL header, char * stream, int length)
{
	KDicomElement * pDE;
	char preamble[128] = {0}, prefix[5] = {"DICM"};
	POSITION pos, old_pos;
   KD_TAG tag;

	FILE * file;
	if((file = _tfopen(filename, _T("wb"))) == NULL){
      SetLastError(KE_CANNOT_OPEN_FILE);
		return FALSE;
	}

	m_strFilePath = filename;

   // Add Meta Info elements.
	if(header)
		AddHeader(ts, _T("DTK"));

   // Delete All Except Header
	pos = m_listDE.GetHeadPosition();
	while(pos != NULL){
		old_pos = pos;
		pDE	= m_listDE.GetNext(pos);
		tag	= pDE->GetTag();
      if(tag.group != 0x0002)
         DeleteElement(pDE);
   }    

	WriteCS(file, preamble, 128);
	WriteCS(file, prefix, 4);

	// Encode Header
	pos = m_listDE.GetHeadPosition();
	while(pos != NULL){
		pDE = m_listDE.GetNext(pos);
		EncodeElement(file, pDE, FALSE);
	}

   // Write Stream
   size_t res_length = fwrite(stream, length, 1, file);
   if(res_length != 1){
      SetLastError(KE_CANNOT_WRITE_TO_FILE);
      return FALSE;
   }

   fclose(file);

	return TRUE;
}

BOOL KDicomDS::EncodeMain(FILE * file, BOOL bHeader)
{
	KDicomElement * pDE;
	char preamble[128] = {0}, prefix[5] = {"DICM"};
	POSITION pos;

	if(bHeader){
		WriteCS(file, preamble, 128);
		WriteCS(file, prefix, 4);
	}

	pos = m_listDE.GetHeadPosition();
	while(pos != NULL){
		pDE = m_listDE.GetNext(pos);
		EncodeElement(file, pDE, FALSE);
	}

	return TRUE;
}

unsigned int KDicomDS::EncodeElement(FILE * file, KDicomElement * pDE, BOOL bLength)
{
	unsigned int element_length = 0, length;
	KD_TAG				tag;
	KD_VR_ENUM			vr;
	KD_TRANSFER_SYNTAX ts, tsOrg;

	tag		= pDE->GetTag();
	length	= pDE->GetLength();
	vr		   = pDE->GetVR();

	// Transfer Syntax
   if(tag.group == 0x0002){
		ts = EXPLICIT_LITTLE;
      tsOrg = EXPLICIT_LITTLE;
   }
   else{
		ts = TransferSyntax;
      tsOrg = m_nTSOrg;
   }

	// Tag
	if(!bLength){
		WriteUS(file, tag.group, ts);
		WriteUS(file, tag.element, ts);
	}
	element_length += 4;

	// VR
	if(ts != IMPLICIT_LITTLE && tag.group != 0xFFFE){
		if(!bLength)
			WriteCS(file, pDE->GetchVR(), 2);
		element_length += 2;
	}

	// Calculate SQ Element Length
	if(pDE->m_listDE.GetCount() > 0 && length != 0xFFFFFFFF){
		length = 0;
		KDicomElement * pChild;
		POSITION pos = pDE->m_listDE.GetHeadPosition();
		while(pos != NULL){
			pChild = pDE->m_listDE.GetNext(pos);
			length += EncodeElement(file, pChild, TRUE);
		}
	}

	// Length
	if(ts == IMPLICIT_LITTLE || tag.group == 0xFFFE){
		if(!bLength)
			WriteUL(file, length, ts);
		element_length += 4;
	}
	else{
		if(vr == OB || vr == OW || vr == SQ || vr == UN || vr == UT){
			if(!bLength){
				WriteUS(file, 0, ts);
				WriteUL(file, length, ts);
			}
			element_length += 6;
		}
		else{
			if(!bLength)
				WriteUS(file, unsigned short(length), ts);
			element_length += 2;
		}
	}

	// Value
	if(pDE->m_listDE.GetCount() > 0){
		KDicomElement * pChild;
		POSITION pos = pDE->m_listDE.GetHeadPosition();
		while(pos != NULL){
			pChild = pDE->m_listDE.GetNext(pos);
			element_length += EncodeElement(file, pChild, bLength);
		}
	}
	else{
		if(!bLength)
			WriteValue(file, pDE, tsOrg, ts);
		element_length += length;
	}

	return element_length;
}

void KDicomDS::AddHeader(KD_TRANSFER_SYNTAX ts, CString app_entity)
{
	KDicomElement * pDE, * pMDE;
	CString strTrans;
	char version[2] = {0, 1};
   unsigned int group_length = 0;

	pMDE = InsertElement(0x0002, 0x0001);
	pMDE->SetchVR("OB");
	pMDE->SetVR(OB);
	pMDE->AddValueOB((unsigned char *) version, 2);
   group_length += EncodeElement(NULL, pMDE, TRUE);

	pMDE = InsertElement(0x0002, 0x0002);
	pMDE->SetchVR("UI");
	pMDE->SetVR(UI);
	pDE = GetElement(0x0008,0x0016);
	pMDE->AddValueUI(pDE->GetValueUI(0));
   group_length += EncodeElement(NULL, pMDE, TRUE);

	pMDE = InsertElement(0x0002, 0x0003);
	pMDE->SetchVR("UI");
	pMDE->SetVR(UI);
	pDE = GetElement(0x0008,0x0018);
	pMDE->AddValueUI(pDE->GetValueUI(0));
   group_length += EncodeElement(NULL, pMDE, TRUE);

	pMDE = InsertElement(0x0002, 0x0010);
	pMDE->SetchVR("UI");
	pMDE->SetVR(UI);
	strTrans = GetTransferSyntaxUID(ts);
	pMDE->AddValueUI(strTrans);
   group_length += EncodeElement(NULL, pMDE, TRUE);

	pMDE = InsertElement(0x0002, 0x0012);
	pMDE->SetchVR("UI");
	pMDE->SetVR(UI);
	pMDE->AddValueUI(IMPLEMENTATION_CLASS_UID);
   group_length += EncodeElement(NULL, pMDE, TRUE);

	pMDE = InsertElement(0x0002, 0x0016);
	pMDE->SetchVR("AE");
	pMDE->SetVR(AE);
	pMDE->AddValueAE(app_entity);
   group_length += EncodeElement(NULL, pMDE, TRUE);

   pMDE = InsertElement(0x0002, 0x0000);
   pMDE->SetchVR("UL");
   pMDE->SetVR(UL);
   pMDE->AddValueUL(group_length);
}

void KDicomDS::WriteUS(FILE * file, unsigned short value, KD_TRANSFER_SYNTAX ts)
{
	unsigned short dst = value;
	if(ts == EXPLICIT_BIG)
		dst = htons(value);
   
   fwrite(&dst, 2, 1, file);
}

void KDicomDS::WriteUL(FILE * file, unsigned int value, KD_TRANSFER_SYNTAX ts)
{
	unsigned dst = value;
	if(ts == EXPLICIT_BIG)
		dst  = htonl(value);
	fwrite(&dst, 4, 1, file);
}

void KDicomDS::WriteCS(FILE * file, char * src, int length)
{
	if(length <= 0)
		return;
	fwrite(src, length, 1, file);
}

void KDicomDS::WriteBinary(FILE * file, unsigned char * src, int length)
{
	if(length <= 0)
		return;
	fwrite(src, length, 1, file);
}

int KDicomDS::WriteValue(FILE * file, KDicomElement * pDE, KD_TRANSFER_SYNTAX tsOrg, KD_TRANSFER_SYNTAX tsDst)
{
	int length = pDE->GetLength();
	if(length <= 0)
		return 0;

	int i;
	unsigned char * pValue = pDE->GetValue();

	if((tsOrg != EXPLICIT_BIG && tsDst != EXPLICIT_BIG) ||    // Little -> Little
      (tsOrg == EXPLICIT_BIG && tsDst == EXPLICIT_BIG)){    // Big -> Big
		fwrite(pValue, length, 1, file);
		return length;
	}
	else{ // Little -> Big, Big -> Little
		KD_VR_ENUM vr = pDE->GetVR();
		int vm	= pDE->GetVM();

		if(vr != OW && vr != US && vr != SS && vr != AT && vr != UL && vr != SL && vr != FL && vr != FD){
			fwrite(pValue, length, 1, file);
			return length;
		}

		if(vr == OW){
			for(i=0;i<length/2;i++){
				fwrite(pValue+1, 1, 1, file);
				fwrite(pValue  , 1, 1, file);
				pValue	+= 2;
			}
			return length;
		}

		int res = 0;
		for(i=0;i<vm;i++){
			if(vr==US || vr==SS){					// 2byte
				fwrite(pValue+1, 1, 1, file);
				fwrite(pValue  , 1, 1, file);
				pValue	+= 2;
				res += 2;
			}
			else if(vr==AT){						// 2byte * 2
				fwrite(pValue+2, 2, 1, file);
				fwrite(pValue  , 2, 1, file);
				pValue	+= 4;
				res += 4;
			}
			else if(vr==UL || vr==SL || vr==FL){	// 4byte
				fwrite(pValue+3, 1, 1, file);
				fwrite(pValue+2, 1, 1, file);
				fwrite(pValue+1, 1, 1, file);
				fwrite(pValue  , 1, 1, file);
				pValue += 4;
				res += 4;
			}
			else if(vr==FD){						// 8byte
				fwrite(pValue+7, 1, 1, file);
				fwrite(pValue+6, 1, 1, file);
				fwrite(pValue+5, 1, 1, file);
				fwrite(pValue+4, 1, 1, file);
				fwrite(pValue+3, 1, 1, file);
				fwrite(pValue+2, 1, 1, file);
				fwrite(pValue+1, 1, 1, file);
				fwrite(pValue  , 1, 1, file);
				pValue += 8;
				res += 8;
			}
		}
		return res;
	}
}

BOOL KDicomDS::SaveAsIs(CString filename)
{
   FILE * file;
   if((file = _tfopen(filename, _T("wb"))) == NULL){
      SetLastError(KE_CANNOT_OPEN_FILE);
      return FALSE;
   }

   KDicomElement * pDE;
   POSITION old_pos;
   POSITION pos = m_listDE.GetHeadPosition();
   while(pos != NULL){
      old_pos = pos;
      pDE = m_listDE.GetNext(pos);
      if(pDE->GetTag().group == 0x0002)
         continue;
      if(pDE->GetTag().element == 0x0000){
         m_listDE.RemoveAt(old_pos);
         delete pDE;
      }
   }

   EncodeMain(file, m_bMetaHeader);

   fclose(file);

   return TRUE;
}
