
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
	unsigned char *pDib;
	unsigned char *tDib;
	BITMAPINFO *bitmapInfo;
	unsigned char samplePerPixel;
	unsigned int imageWidth;
	unsigned int imageHeight;
	unsigned int imageStep;
	unsigned char *srcData;
	unsigned char *dstData;

	int GetRealWidth(int);
	unsigned char Clip(int, int , int);
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
};

