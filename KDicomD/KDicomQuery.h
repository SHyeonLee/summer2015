class KDICOM_CLASS KDicomQuery
{
public:
	KDicomQuery();
	virtual ~KDicomQuery();

public:
	unsigned char * m_pView, * m_pCur;
	int				m_nNums;
	KD_ATTRIBUTE *	m_Atts;
 	HMODULE			m_hModule;

	BOOL			   LoadResource();

public:
	// attribute
	int				GetCountAttribute();
	KD_ATTRIBUTE	GetAttribute(KD_TAG tag);
	KD_ATTRIBUTE	GetAttribute(int pos);

	// VR
	KD_VR_ENUM		GetVRCode(char * str);

	int				Compare(KD_TAG a, KD_TAG b);
	void			   Read(void * value, int size, int count);

	// Cache
protected:
	CString			m_strCachePath;
	int				m_nFileCount;

public:
	void			   SetCachePath(CString str);
	CString			GetCacheFilePath();
};
