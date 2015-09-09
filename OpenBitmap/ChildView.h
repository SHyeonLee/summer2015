
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
	unsigned char	*dibData;
	unsigned char	*tDib;
	BITMAPINFO		*bitmapInfo;
	unsigned char	samplePerPixel;
	unsigned int	Width;
	unsigned int	Height;
	unsigned int	Step;
	unsigned char	*srcData;
	unsigned char	*dstData;
	short			*src;
	CString			Photometric;
	int				BitsAllocated;
	int				BitsStored;
	int				PixelRepresentation;
	double			WindowWidth;
	double			WindowCenter;

	BOOL			rightButtonDown;
	CPoint			rightbuttonPoint;


	void			OpenBMPFile(CString path);
	void			OpenDCMFile(CString path);
	BOOL			CreateDIB();
	int				GetRealWidth(int);
	unsigned char	Clip(int, int , int);
	void			GammaCorrection(double gamma);
	void			SpatialFilter(double *mask);
	unsigned char	bit16to8(short ivalue, short WindowW, short WindowC, double ratio);

	CScrollBar		scrollBar;

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
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnTestHelloworld();
	afx_msg void OnFileOpen();
	afx_msg void OnArithmeticAdd();
	afx_msg void OnArithmeticSub();
	afx_msg void OnArithmeticMultiply();
	afx_msg void OnArithmeticDivide();
	afx_msg void OnArithmeticNegative();
	afx_msg void OnArithmeticPic();
	afx_msg void OnGeometryFlipH();
	afx_msg void OnGeometryFlipV();
	afx_msg void OnGeometryRotateL();
	afx_msg void OnGeometryRotateR();
	afx_msg void OnUpdateLutAdd(CCmdUI *pCmdUI);
	afx_msg void OnLutAdd();
	afx_msg void OnLutNegative();
	afx_msg void OnLutGamma();
	afx_msg void OnFilterBlur();
	afx_msg void OnFilterSharpen();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
};

