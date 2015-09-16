class KDICOM_CLASS KDicomDS
{
public:
	KDicomDS();
	virtual	~KDicomDS();

public:
   // DICOM File Common
	CString				   m_strFilePath;
	void				      SetFilePath(CString str);
	CString			     	GetFilePath();
	int					   m_nFileLength;
	int				   	GetFileLength();

   // DICOM File 
   FILE *               m_pFile;
   fpos_t               m_posStart;
   BOOL                 m_bFile;

   // Memory
	unsigned char *	   m_pCur;
   unsigned char *      m_pView;

   // DICOM Buffer
	BOOL				      m_bBuffer;

   // DICOM Stream
   int                  m_nStreamPos;
   int                  GetStreamStartPos();
 	int				   	GetStreamSize();

   // Transfer Syntax
	KD_TRANSFER_SYNTAX	m_nTSOrg;
	KD_TRANSFER_SYNTAX	TransferSyntax;
	KD_TRANSFER_VR		   CheckTransferVR();
	KD_TRANSFER_SYNTAX	GetTransferSyntax();
	KD_TRANSFER_SYNTAX	GetTransferSyntax(CString str);
	CString				   GetTransferSyntaxUID(KD_TRANSFER_SYNTAX ts);
   CString              GetTransferSyntaxUID();
 
   // Dataset Encode
	BOOL				      SaveDS(CString filename, KD_TRANSFER_SYNTAX ts, BOOL header);
   BOOL                 SaveStream(CString filename, KD_TRANSFER_SYNTAX ts, BOOL header, char * stream, int length);
	void			   	   AddHeader(KD_TRANSFER_SYNTAX ts, CString app_entity);
	void		   		   WriteUS(FILE * file, unsigned short, KD_TRANSFER_SYNTAX ts);
	void			   	   WriteUL(FILE * file, unsigned int, KD_TRANSFER_SYNTAX ts);
	void			   	   WriteCS(FILE * file, char * src, int length);
	void			   	   WriteBinary(FILE * file, unsigned char * src, int length);
	int			   		WriteValue(FILE * file, KDicomElement * pDE, KD_TRANSFER_SYNTAX tsOrg, KD_TRANSFER_SYNTAX tsDst);
	unsigned int   		EncodeElement(FILE * file, KDicomElement * pDE, BOOL bLength);
	BOOL				      EncodeMain(FILE * file, BOOL bHeader);

   BOOL                 m_bMetaHeader;
   BOOL				      SaveAsIs(CString filename);
   BOOL                 DecodeCommon(KD_TRANSFER_SYNTAX ts);
   void                 DecodeImage(KDicomElement * pDE);

   // Dataset Decode
	BOOL				      LoadDS(CString filename, BOOL bReadOnly = TRUE, KD_TRANSFER_SYNTAX ts = TS_AUTO);
   BOOL				      LoadDSBuff(unsigned char * pBuff, int length, KD_TRANSFER_SYNTAX ts = TS_AUTO);
	BOOL	      			MetaInfoProcess();
	BOOL                 ReadUS(unsigned short * value);
	BOOL                 ReadUL(unsigned int * value);
	BOOL	      			ReadCS(char * dst, int count, BOOL nc = FALSE);
	BOOL	      			Forward(unsigned int count);
	BOOL		       		Backward(unsigned int count);
	int			   		Compare(KD_TAG a, KD_TAG b);
	BOOL                 DecodeDataset(KDicomElement * pParent, unsigned int * length);
	BOOL                 DecodeElement(KDicomElement * pDE, KD_TRANSFER_VR ts, unsigned int * length);

   // Copy
	unsigned int   		CopyElement(KDicomDS * pSrcDS, KDicomElement * pSrc, KDicomElement * pDst, BOOL bValue);
	BOOL				      CopyDS(KDicomDS * pDS, BOOL bValue);

   // Reset 
	void				      CloseDS();
	void				      Reset();

   // Element
	CTypedPtrList<CPtrList, KDicomElement *> m_listDE;
	KDicomElement *		AddElement(KD_TAG tag, KDicomElement * pParent = NULL);
	KDicomElement *		AddElement(unsigned short group, unsigned short element, KDicomElement * pParent = NULL);
	KDicomElement *      InsertElement(KD_TAG tag);
	KDicomElement *      InsertElement(unsigned short group, unsigned short element);
	void				      DeleteElement(KDicomElement * pDE, KDicomElement * pParent = NULL);
   BOOL                 DeleteElement(unsigned short group, unsigned short element, KDicomElement * pParent = NULL);
	KDicomElement *		GetElement(KD_TAG tag, KDicomElement * pParent = NULL);
	KDicomElement *		GetElement(unsigned short group, unsigned short element, KDicomElement * pParent = NULL);
	void				      SetFamily(KDicomElement * pChild, KDicomElement * pParent);

	// Multi Frame
	int					   m_nFrameCount;
	int				   	CheckMultiFrame();
	int					   GetFrameCount();

   // DICOM Dictionary
	static KDicomQuery	m_Query;
	KDicomQuery *		   GetQuery();
	void				      SetCachePath(CString str);

   // Log
	CString			      m_strLogPath;
	void		      		SetLogPath(CString path, BOOL Reset);
	void     				WriteLog(CString strMessage, BOOL bShowTime = TRUE, BOOL bNewLine = TRUE);

   // Character Set
   BOOL                 m_bCharacterSet;
   CStringArray         m_arrCharacterSet;
   CString              GetLocaleString(char * string);
   DWORD                _csCodepage;
   void                 setCharset();
   void                 setCodepage(DWORD csCodepage)    { _csCodepage = csCodepage; }
   DWORD                getCodepage() const     { return _csCodepage; }
   void                 setDefaultCharset();

   int                  m_nSamplePerPixel, m_nWidth, m_nHeight, m_nBitsAllocated, m_nBitsStored, m_nRepresentation;
   double               m_dWindowCenter, m_dWindowWidth;
   KD_PHOTOMETRIC       m_nPhotometric;

   void                 ReadParameters();
   BOOL                 GetImageData(unsigned char * pBuff, int frame = 0);
   BOOL                 DecodeJpegMem8(unsigned char * pSrc, unsigned int srcLength, unsigned char * pDst);
   BOOL                 DecodeJpegMem12(unsigned char * pSrc, unsigned int srcLength, unsigned char * pDst);
   BOOL                 DecodeJpegMem16(unsigned char * pSrc, unsigned int srcLength, unsigned char * pDst);
};
