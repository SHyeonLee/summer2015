
// ChildView.h : interface of the CChildView class
//


#pragma once


// CChildView window

class CChildView : public CWnd
{
	// Construction
public:
	CChildView();

	// Attributes
public:
	//variables
	BITMAPINFO*			bitmapInfo;

	unsigned char *		dibData;
	unsigned char *		dibImage;

	unsigned char *		srcData;
	unsigned char *		dstData;

	int					samplePerPixel, width, height, srcStep, dibStep, bitsAllocated, bitsStored;
	int					pixelRepresentation;
	double				windowWidth, windowCenter, windowWidthTemp, windowCenterTemp;
	KD_PHOTOMETRIC		photometric;

	CPoint				rClick;
	BOOL				rBtnDown;
	BOOL				rMove;

	//functions
	void				OpenBITMAPFile(CString path);
	void				OpenDICOMFile(CString path);

	BOOL				CreateDIB();
	void				Trans16to8();

	int					GetRealWidth(int width);
	int					Clip(int value, int low, int high);

	// Operations
public:

	// Overrides
protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

	// Implementation
public:
	virtual ~CChildView();

	// Generated message map functions
protected:
	afx_msg void OnPaint();
	afx_msg void OnFileOpen();
	afx_msg void OnArithmeticAddC();
	afx_msg void OnArithmeticSubC();
	afx_msg void OnArithmeticMulC();
	afx_msg void OnArithmeticDivC();
	afx_msg void OnGeometryFlipH();
	afx_msg void OnGeometryFlipV();
	afx_msg void OnGeometryRotateR();
	afx_msg void OnGeometryRotateL();
	afx_msg void OnFilterAvr();
	afx_msg void OnFilterSharpen();
	DECLARE_MESSAGE_MAP()
public:
//	afx_msg void OnRButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
};

