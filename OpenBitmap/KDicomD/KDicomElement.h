class KDicomDS;
class KDicomQuery;

class KDICOM_CLASS KDicomElement
{
public:
	KDicomElement(KDicomDS * pDS);
	KDicomElement(unsigned short group, unsigned short element, KDicomQuery * pQuery);
	virtual ~KDicomElement();

public:
	KD_TAG			   m_nTag, m_nMask;
	char				   m_szVR[4];
	KD_VR_ENUM		   m_nVR;
	unsigned			   m_nLength, m_nItemLevel;
	int				   m_nMinVM, m_nMaxVM, m_nVM;
	KDicomElement *   m_pParent;
	BOOL				   m_bString;
	BOOL				   m_bAlloc;
	BOOL				   m_bSwap;
   int               m_nImagePos;

	CTypedPtrList<CPtrList, KDicomElement *> m_listDE;

	unsigned char *	m_pValue;

   KDicomDS *        m_pDS;
	void				   SetTag(KD_TAG tag);
	KD_TAG				GetTag();
	void				   SetMask(KD_TAG new_mask);
	KD_TAG				GetMask();
	KD_VR_ENUM			GetVR();
	void				   SetVR(KD_VR_ENUM new_vr);
	char *				GetchVR();
	void				   SetchVR(char * new_chvr);
	unsigned			   GetLength();
	void				   SetLength(unsigned length);
	void				   SetVM(int vm);
	int					GetVM();
	int					GetMinVM();
	void				   SetMinVM(int min);
	int					GetMaxVM();
	void				   SetMaxVM(int max);
	void				   SetParent(KDicomElement * parent);
	KDicomElement		* GetParent();
	void				   SetItemLevel(unsigned level);
	unsigned			   GetItemLevel();
	void				   SetValue(unsigned char * value, KD_TRANSFER_SYNTAX ts);
	void				   SetValue(FILE * file, KD_TRANSFER_SYNTAX ts);
	unsigned char *	GetValue();

	BOOL				   AddValueString(CString str, int max_num, int padding = 0);
	BOOL				   AddValueBinary(unsigned char * buff, int num);
	unsigned char *	ValueAlloc(int length, int padding);
	BOOL				   CheckVM();
	BOOL				   IsString();
	void				   FreeMem();
	BOOL				   AllocMem(int length, int padding);
   KD_PN             ParsingPN(CString str);

public:
	CString				GetValueAE(int vm);
	KD_AS				   GetValueAS(int vm);
	KD_TAG				GetValueAT(int vm);
	CString				GetValueCS(int vm);
	KD_DA				   GetValueDA(int vm);
	CString				GetValueDS(int vm);
	KD_DT				   GetValueDT(int vm);
	float				   GetValueFL(int vm);
	double				GetValueFD(int vm);
	CString				GetValueIS(int vm);
	CString				GetValueLO(int vm);
	CString				GetValueLT(int vm);
	unsigned char *	GetValueOB();
	unsigned short *	GetValueOW();
	KD_PN				   GetValuePN(int vm);
	CString				GetValueSH(int vm);
	int					GetValueSL(int vm);
	short				   GetValueSS(int vm);
	CString				GetValueST(int vm);
	KD_TM				   GetValueTM(int vm);
	CString				GetValueUI(int vm);
	unsigned			   GetValueUL(int vm);
	unsigned short		GetValueUS(int vm);
	CString				GetValueUT(int vm);
	CString				GetValueString(int vm);
	unsigned char * 	GetValueBinary(int vm, int size);

	BOOL	   			AddValueAE(CString str);
	BOOL	   			AddValueAS(KD_AS age);
	BOOL	   			AddValueAT(KD_TAG);
	BOOL	   			AddValueCS(CString str);
	BOOL	   			AddValueDA(KD_DA date);
	BOOL	   			AddValueDS(CString str);
	BOOL	   			AddValueDT(KD_DT dt);
	BOOL	   			AddValueFL(float data);
	BOOL	   			AddValueFD(double data);
	BOOL	   			AddValueIS(CString str);
	BOOL	   			AddValueLO(CString str);
	BOOL	   			AddValueLT(CString str);
	BOOL	   			AddValueOB(unsigned char * data, int num);
	BOOL	   			AddValueOW(unsigned short * data, int num);
	BOOL	   			AddValueOW(CString filename, int offset);
	BOOL	   			AddValuePN(KD_PN pn);
	BOOL	   			AddValueSH(CString str);
	BOOL	   			AddValueSL(int data);
	BOOL	   			AddValueSQ();
	BOOL	   			AddValueSS(short data);
	BOOL	   			AddValueST(CString str);
	BOOL		   		AddValueTM(KD_TM time);
	BOOL	   			AddValueUI(CString str);
	BOOL	   			AddValueUL(unsigned int data);
	BOOL	   			AddValueUS(unsigned short data);
	BOOL	   			AddValueUT(CString str);

	BOOL		   		DeleteValue(unsigned int vm);
};