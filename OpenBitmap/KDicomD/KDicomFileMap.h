class KDICOM_CLASS KDicomFileMap
{
public:
   KDicomFileMap();
   virtual	~KDicomFileMap();

public:
   HANDLE				   m_hFileMap;
   void *				   m_pView;
   CString              m_strFilePath;
   BOOL                 m_bCreateFile;

   BOOL                 Create(CString filename, BOOL bReadOnly, int length = 0);
   unsigned char *      Map();
   void                 Unmap();
};