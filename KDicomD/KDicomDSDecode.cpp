#include "stdafx.h"

BOOL KDicomDS::LoadDS(CString filename, BOOL bReadOnly, KD_TRANSFER_SYNTAX ts)
{
	Reset();

   m_bFile = TRUE;

   // Open File
   if((m_pFile = _tfopen(filename, _T("rb"))) == NULL){
      SetLastError(KE_CANNOT_OPEN_FILE);         
      return FALSE;
   }

   // Get File length;
   fpos_t end;
   fgetpos(m_pFile, &m_posStart);
   fseek(m_pFile, 0, SEEK_END);
   fgetpos(m_pFile, &end);
   fseek(m_pFile, 0, SEEK_SET);
   m_nFileLength = int(end - m_posStart);

   // Get File Name
	m_strFilePath = filename;

   if(!DecodeCommon(ts))
      return FALSE;

   // Close File
   fclose(m_pFile);

   ReadParameters();

   return TRUE;
}

BOOL KDicomDS::LoadDSBuff(unsigned char * pBuff, int length, KD_TRANSFER_SYNTAX ts)
{
	Reset();

	m_bFile  		= FALSE;
	m_pCur			= pBuff;
	m_pView			= pBuff;
	m_nFileLength	= length;
	m_bBuffer      = TRUE;

   if(!DecodeCommon(ts))
      return FALSE;

   return TRUE;
}

BOOL KDicomDS::DecodeCommon(KD_TRANSFER_SYNTAX ts)
{
   // Read Meta Info Header.
   MetaInfoProcess();

   // Check Transfer Syntax.
   if(ts == TS_AUTO)
      TransferSyntax = GetTransferSyntax();
   else
      TransferSyntax = ts;

   m_nTSOrg = TransferSyntax;

   // Decode Dataset (due to SPIHT)
   unsigned int length;
   if(!DecodeDataset(NULL, &length))
      return FALSE;

   if(ts == TS_AUTO)
      TransferSyntax = GetTransferSyntax();
   else
      TransferSyntax = ts;

   // Check Frame
   m_nFrameCount	= CheckMultiFrame();

   KDicomElement * pDE;
   // Character Set
   if((pDE = GetElement(0x0008, 0x0005)) != NULL){
      for(int i=0;i<pDE->GetVM();i++)
         m_arrCharacterSet.Add(pDE->GetValueCS(i));
   }
   if(m_arrCharacterSet.GetSize() > 0)
      setCharset();
   else
      setDefaultCharset();

   // OB to OW
   if((pDE = GetElement(0x0028, 0x0100)) != NULL){
      int BitsAllocated = pDE->GetValueUS(0);
      if(BitsAllocated == 16){
         if((pDE = GetElement(0x7FE0, 0x0010)) != NULL){
            if(pDE->m_listDE.GetCount() == 0){
               pDE->SetVR(OW);
               pDE->SetchVR("OW");
            }
         }
      }
   }

   return TRUE;  
}

BOOL KDicomDS::MetaInfoProcess()
{
	// Check Command Set
   if(m_nFileLength < 132)
   	return FALSE;

	// Check DICOM prefix.
	char mark[4];
	if(!Forward(128))
      return FALSE;
	if(!ReadCS(mark, 4))
      return FALSE;
	if(memcmp(mark, "DICM", 4) != 0){
		Backward(128 + 4);
		return FALSE;
	}

   // There is Meta Header
   m_bMetaHeader = TRUE;

	// Read data set.
	KDicomElement * pDE;
	KD_TAG tag;
	KD_VR_ENUM vr;
	char VR[3];
	unsigned int length;
	TransferSyntax	= EXPLICIT_LITTLE;
	while(1){
		// Read tag
      if(!ReadUS(&tag.group))
         return FALSE;
      if(!ReadUS(&tag.element))
         return FALSE;

		// Exit condition.
		if(tag.group != 0x0002){
			Backward(4);
			break;
		}

		// Add Element;
		pDE = AddElement(tag);

		// Read VR.
		if(!ReadCS(VR, 2, TRUE))
         return FALSE;
		pDE->SetchVR(VR);
		vr = m_Query.GetVRCode(VR);
		pDE->SetVR(vr);

		// Read length.
		if(vr == OB || vr == OW || vr == SQ || vr == UN){
			if(!Forward(2))
            return FALSE;
			if(!ReadUL(&length))
            return FALSE;
		}
      else{
         unsigned short slength;
         if(!ReadUS(&slength))
            return FALSE;
         length = slength;
      }
		pDE->SetLength(length);

		// Set Value;
		if(m_bFile){
         pDE->SetValue(m_pFile, TransferSyntax);
		}
		else{
         pDE->SetValue(m_pCur, TransferSyntax);
         if(!Forward(length))
            return FALSE;
		}
	}
	return TRUE;
}

BOOL KDicomDS::DecodeDataset(KDicomElement * pParent, unsigned int * length)
{
	KDicomElement * pDE;
	KD_TAG			tag;
	KD_TRANSFER_VR	ts;
   * length = 0;
	
	ts = CheckTransferVR();
	while(1){
		// Exit condition #1: EOF
		if(!m_bFile){
         if(m_pCur == (unsigned char *) m_pView + m_nFileLength){
            return TRUE;
         }
		}
		// Group
      if(!ReadUS(&tag.group)){
         // Exit condition #2: EOF
         if(m_bFile){
            if(feof(m_pFile) != 0)
               return TRUE;
         }
         return FALSE;
      }
		// Element
      if(!ReadUS(&tag.element))
         return FALSE;
		// Exit condition #3: tag is 0, still under consideration

      if(m_nFileLength > 100000 && tag.group == 0)
			continue;
		// Add Element
		pDE = AddElement(tag, pParent);
		// Decode
      unsigned int element_length;
		DecodeElement(pDE, ts, &element_length);
      * length += element_length;
		// Exit condition #4:
		if(pParent != NULL){
			if(tag.group == 0xFFFE && tag.element == 0xE00D)
				break;
			if(tag.group == 0xFFFE && tag.element == 0xE0DD)
				break;
			if(pParent->GetLength() <= * length)
				break;
		}
	}

   return TRUE;
}

BOOL KDicomDS::DecodeElement(KDicomElement * pDE, KD_TRANSFER_VR ts, unsigned int * element_length)
{
	KD_TAG				tag, big_tag;
	char				   VR[3];
	KD_VR_ENUM			vr;
	unsigned int		length;
	KD_TRANSFER_VR		element_ts;

   * element_length = 0;

	// Tag
	tag = pDE->GetTag();
	* element_length += 4;

	// Syntax
	if(tag.group == 0xFFFE)
		element_ts = IMPLICIT;
	else
		element_ts = ts;

	// VR
   if(element_ts == EXPLICIT){
      if(!ReadCS(VR, 2, TRUE))
         return FALSE;
      vr = m_Query.GetVRCode(VR);
	   pDE->SetVR(vr);
	   pDE->SetchVR(VR);
      * element_length += 2;
	}
	else{
	   vr = pDE->GetVR();	
	}

	// Length
	if(element_ts == EXPLICIT){
		if((vr == OB || vr == OW || vr == SQ || vr == UN || vr == UT) && tag.element != 0){
			if(!Forward(2))
            return FALSE;
			if(!ReadUL(&length))
            return FALSE;
			* element_length += 6;
		}
		else{
         unsigned short slength;
			if(!ReadUS(&slength))
            return FALSE;
         length = slength;
			* element_length += 2;
		}
	}
	else{
      if(!ReadUL(&length))
         return FALSE;
		* element_length += 4;
	}
	pDE->SetLength(length);
   if(length == 0){
		return TRUE;
   }

   KDicomElement * big_parent = NULL;
	big_tag.group = 0;
	big_tag.element = 0;
	if(pDE->GetParent() != NULL){
		big_tag = pDE->GetParent()->GetTag();
      big_parent = pDE->GetParent()->GetParent();
	}
   
	// Value
   if(big_tag.group == 0x7FE0 && big_tag.element == 0x0010){
      if(big_parent == NULL){
         // Compressed Image
		   if(length == 0xFFFFFFFF){
            // Undefined Length
            unsigned int dataset_length;
            if(!DecodeDataset(pDE, &dataset_length))
               return FALSE;
            * element_length += dataset_length;
		   }
		   else{
			   if(m_bFile){
               // Compressed Image: File to Memory
               DecodeImage(pDE);
			   }
			   else{
               // Compressed Image: Memory to Memory
               pDE->SetValue(m_pCur,TransferSyntax);
               if (!Forward(length))
                  return FALSE;
 			   }
			   * element_length += length;
		   }
      }
      else{
         // Compressed Image
         if(length == 0xFFFFFFFF){
            // Undefined Length
            unsigned int dataset_length;
            if(!DecodeDataset(pDE, &dataset_length))
               return FALSE;
            * element_length += dataset_length;
         }
         else{
            // Defined Length
            if(m_bFile){
               // Compressed Image: File to Memory
               DecodeImage(pDE);
            }
            else{
               // Compressed Image: Memory to Memory
               pDE->SetValue(m_pCur,TransferSyntax);
               if (!Forward(length))
                  return FALSE;
             }
            * element_length += length;
         }
      }
	}
	else if(length == 0xFFFFFFFF || vr == SQ || (tag.group == 0xFFFE && tag.element == 0xE000)){
      unsigned int dataset_length;
		if(!DecodeDataset(pDE, &dataset_length))
         return FALSE;
      * element_length += dataset_length;
	}
	else{
		if(m_bFile){
         if(tag.group == 0x7FE0 && tag.element == 0x0010 && pDE->m_pParent == NULL){
            // Uncompressed Image: File to Memory
            DecodeImage(pDE);
         }
         else{
            // Normal Value: File to Memory 
            pDE->SetValue(m_pFile, TransferSyntax);
         }
		}
		else{
         pDE->SetValue(m_pCur,TransferSyntax);
         if (!Forward(length))
            return FALSE;
		}
		* element_length += length;
	}

	return TRUE;
}

void KDicomDS::DecodeImage(KDicomElement * pDE)
{
   pDE->SetValue(m_pFile, TransferSyntax);
}

BOOL KDicomDS::ReadUS(unsigned short * value)
{
	if(m_bFile){
      if(fread(value, 2, 1, m_pFile) < 1){
         SetLastError(KE_TRY_TO_READ_INVALID_FILE_AREA);
         return FALSE;
      }
	}
	else{
      if(m_pCur + 2 > (unsigned char *) m_pView + m_nFileLength){
         SetLastError(KE_TRY_TO_READ_INVALID_FILE_AREA);
         return FALSE;         
      }
      * value = * ((unsigned short *) m_pCur);
      m_pCur += 2;
	}
	if(TransferSyntax == EXPLICIT_BIG)
		* value = ntohs(* value);
   return TRUE;
}

BOOL KDicomDS::ReadUL(unsigned int * value)
{
	if(m_bFile){
      if(fread(value, 4, 1, m_pFile) < 1){
         SetLastError(KE_TRY_TO_READ_INVALID_FILE_AREA);
         return FALSE;
      }
	}
	else{
      if(m_pCur + 4 > (unsigned char *) m_pView + m_nFileLength){
         SetLastError(KE_TRY_TO_READ_INVALID_FILE_AREA);
         return FALSE;
      }
      * value = *((unsigned int *) m_pCur);
      m_pCur += 4;
	}
	if(TransferSyntax == EXPLICIT_BIG)
		* value = ntohl(* value);
	return TRUE;
}

BOOL KDicomDS::ReadCS(char * dst, int count, BOOL nc)
{
	if(m_bFile){
      if(fread(dst, count, 1, m_pFile) < 1){
         SetLastError(KE_TRY_TO_READ_INVALID_FILE_AREA);
         return FALSE;
      }
      if(nc)
         dst[count] = '\0';
	}
	else{
      if(m_pCur + count > (unsigned char *) m_pView + m_nFileLength){
         SetLastError(KE_TRY_TO_READ_INVALID_FILE_AREA);
         return FALSE;
      }
      memcpy(dst, m_pCur, count);
      if(nc)
         dst[count] = '\0';
      m_pCur += count;
	}
   return TRUE;
}

BOOL KDicomDS::Forward(unsigned int count)
{
   if(m_bFile){
      if(fseek(m_pFile, count, SEEK_CUR) != 0){
         SetLastError(KE_TRY_TO_READ_INVALID_FILE_AREA);
         return FALSE;
      }
   }
   else{
      if(m_pCur + count > (unsigned char *) m_pView + m_nFileLength){
         SetLastError(KE_TRY_TO_READ_INVALID_FILE_AREA);
         return FALSE;
      }
      m_pCur += count;
   }
   return TRUE;
}

BOOL KDicomDS::Backward(unsigned int count)
{
   if(m_bFile){
      int temp = count;
      if(fseek(m_pFile, -temp, SEEK_CUR) != 0){
         SetLastError(KE_TRY_TO_READ_INVALID_FILE_AREA);
         return FALSE;
      }
   }
	else{
      if(m_pCur - count < (unsigned char *) m_pView){
         SetLastError(KE_TRY_TO_READ_INVALID_FILE_AREA);
         return FALSE;
      }
      m_pCur -= count;
	}
   return TRUE;
}

KD_TRANSFER_VR KDicomDS::CheckTransferVR()
{
	KD_TRANSFER_VR	transfer;
	char			vr[3];

	if(m_bFile){
      fseek(m_pFile, 4, SEEK_CUR);
      fread(vr, 2, 1, m_pFile);
      fseek(m_pFile, -6, SEEK_CUR);
	}
	else{
      m_pCur += 4;
      memcpy(vr, m_pCur, 2);
      m_pCur -= 4;
	}

	if(m_Query.GetVRCode(vr) == NF)
		transfer = IMPLICIT;
	else
		transfer = EXPLICIT;

	return transfer;
}

void KDicomDS::setCharset()
{
   CString  cs;

   m_bCharacterSet = TRUE;

   for (int i = 0; i < m_arrCharacterSet.GetSize(); i++) 
   {
      cs = m_arrCharacterSet.GetAt(i);
      // JIS to S-JIS
      if (-1 < cs.Find(_T("ISO 2022 IR 87")) || -1 < cs.Find(_T("ISO 2022 IR 13")) ||
         -1 < cs.Find(_T("ISO_IR 87")) || -1 < cs.Find(_T("ISO_IR 13")))
      {
         _csCodepage = 50220;
      }
      // UTF-8 to the default code page of the default locale
      else if (-1 < cs.Find(_T("ISO_IR 192")) || -1 < cs.Find(_T("ISO 2022 IR 192")))
      {
         _csCodepage = CP_UTF8;
      }
   }
}

void KDicomDS::setDefaultCharset()
{
   switch(GetSystemDefaultLCID())
   {
   case 0x0411:   // Japanese
      _csCodepage = 50220;
      break;

   default:
      _csCodepage = 1252;
   }
}

void KDicomDS::ReadParameters()
{
   KDicomElement * pDE;

   CString str;
   CString photometric[13] = {_T("MONOCHROME1"),_T("MONOCHROME2"),_T("PALETTE COLOR"),_T("RGB"),_T("HSV"),
      _T("ARGB"),_T("CMYK"),_T("YBR_FULL"),_T("YBR_FULL_422"),_T("YBR_PARTIAL_422")
      _T("YBR_PARTIAL_420"), _T("YBR_ICT"), _T("YBR_RCT")};

   // sample per pixel
   if((pDE = GetElement(0x0028, 0x0002)) == NULL)
      m_nSamplePerPixel = 1;
   else
      m_nSamplePerPixel	= pDE->GetValueUS(0);
   
   // photometric interpretation
   if((pDE = GetElement(0x0028, 0x0004)) != NULL){
      str = pDE->GetValueCS(0);
      str.TrimRight();
      for(int i=0;i<13;i++){
         if(str == photometric[i]){
            m_nPhotometric = (KD_PHOTOMETRIC) i;
            break;
         }
      }
   }

   // height
   if((pDE = GetElement(0x0028, 0x0010)) != NULL){
      m_nHeight		= pDE->GetValueUS(0);
   }

   // width
   if((pDE = GetElement(0x0028, 0x0011)) != NULL){
      m_nWidth		   = pDE->GetValueUS(0);
   }

   // bits allocated
   if((pDE = GetElement(0x0028, 0x0100)) != NULL){
      m_nBitsAllocated = pDE->GetValueUS(0);
   }

   // bits stored
   if((pDE = GetElement(0x0028, 0x0101)) != NULL){
      m_nBitsStored	= pDE->GetValueUS(0);
   }

   // pixel representation
   if((pDE = GetElement(0x0028, 0x0103)) == NULL)
      m_nRepresentation = 0;
   else
      m_nRepresentation	= pDE->GetValueUS(0);

   // Window Width
   if((pDE = GetElement(0x0028, 0x1051)) == NULL)
      m_dWindowWidth = 0;
   else
      m_dWindowWidth = _tstof(pDE->GetValueDS(0));

   // Window Center
   if((pDE = GetElement(0x0028, 0x1050)) == NULL)
      m_dWindowCenter = 0;
   else
      m_dWindowCenter = _tstof(pDE->GetValueDS(0));
}
