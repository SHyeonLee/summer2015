
// ChildView.cpp : implementation of the CChildView class
//

#include "stdafx.h"
#include "OpenBitmap.h"
#include "ChildView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CChildView

CChildView::CChildView()
{
	pDib = NULL;
	dstData = NULL;
	tDib = NULL;
}

CChildView::~CChildView()
{
	if(pDib)
		delete pDib;

	if(tDib)
		delete tDib;


	if(dstData)
		delete dstData;
}


BEGIN_MESSAGE_MAP(CChildView, CWnd)
	ON_WM_PAINT()
	ON_COMMAND(ID_TEST_HELLOWORLD, &CChildView::OnTestHelloworld)
	ON_COMMAND(ID_FILE_OPEN, &CChildView::OnFileOpen)
	ON_COMMAND(ID_ARITHMETIC_ADD, &CChildView::OnArithmeticAdd)
	ON_COMMAND(ID_ARITHMETIC_SUB, &CChildView::OnArithmeticSub)
	ON_COMMAND(ID_ARITHMETIC_MULTIPLY, &CChildView::OnArithmeticMultiply)
	ON_COMMAND(ID_ARITHMETIC_DIVIDE, &CChildView::OnArithmeticDivide)
	ON_COMMAND(ID_ARITHMETIC_NEGATIVE, &CChildView::OnArithmeticNegative)
	ON_COMMAND(ID_ARITHMETIC_PIC, &CChildView::OnArithmeticPic)
	ON_COMMAND(ID_GEOMETRY_FLIPH, &CChildView::OnGeometryFlipH)
	ON_COMMAND(ID_GEOMETRY_FLIPV, &CChildView::OnGeometryFlipV)
	ON_COMMAND(ID_GEOMETRY_ROTATEL, &CChildView::OnGeometryRotateL)
	ON_COMMAND(ID_GEOMETRY_ROTATER, &CChildView::OnGeometryRotateR)
END_MESSAGE_MAP()



// CChildView message handlers

BOOL CChildView::PreCreateWindow(CREATESTRUCT& cs) 
{
	if (!CWnd::PreCreateWindow(cs))
		return FALSE;

	cs.dwExStyle |= WS_EX_CLIENTEDGE;
	cs.style &= ~WS_BORDER;
	cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS, 
		::LoadCursor(NULL, IDC_ARROW), reinterpret_cast<HBRUSH>(COLOR_WINDOW+1), NULL);

	return TRUE;
}

void CChildView::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	if(pDib==NULL)
		return;

	SetDIBitsToDevice(dc.m_hDC,
		0,0,Step,Height,
		0,0,0,Height,
		dstData,bitmapInfo,DIB_RGB_COLORS);

	// Do not call CWnd::OnPaint() for painting messages
}



void CChildView::OnTestHelloworld()
{
}


void CChildView::OnFileOpen()
{
	CString szFilter = _T ("bitmap Files (*.bmp)|*.bmp|All Files (*.*)|*.*|");

	CFileDialog dlg (TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT |
		OFN_ALLOWMULTISELECT, szFilter, this);
	if(dlg.DoModal () == IDCANCEL)
		return;

	FILE *file;
	_wfopen_s(&file, dlg.GetPathName(), L"rb");

	BITMAPFILEHEADER bitmapFileHeader;

	fread(&bitmapFileHeader, sizeof(BITMAPFILEHEADER), 1, file);

	int dibSize = bitmapFileHeader.bfSize-sizeof(BITMAPFILEHEADER);

	if(pDib)
		delete[] pDib;

	pDib = new unsigned char[dibSize];

	fread(pDib, dibSize, 1, file);

	//Important Variables
	bitmapInfo = (BITMAPINFO *) pDib;
	samplePerPixel = bitmapInfo->bmiHeader.biBitCount/8;
	Width = bitmapInfo->bmiHeader.biWidth;
	Height = bitmapInfo->bmiHeader.biHeight;
	Step = GetRealWidth(Width);
	srcData =  pDib + sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * bitmapInfo->bmiHeader.biClrUsed;

	//Allocate destination memory
	if(dstData)
		delete[] dstData;

	dstData = new unsigned char[Step*Height];

	//compy src to dst
	memcpy(dstData, srcData, Step*Height);

	//close file
	fclose(file);

	//Repaint client area
	Invalidate(FALSE);
}

void CChildView::OnArithmeticAdd()
{
	if(pDib == NULL)
		return;

	for(int i=0; i<Width*Height; i++)
	{
		dstData[i] = Clip(srcData[i] + 100, 0, 255);
	}
	Invalidate(FALSE);
}

void CChildView::OnArithmeticSub()
{
	if(pDib==NULL)
		return;
	for(int i=0; i<Width*Height; i++)
	{
		dstData[i] = Clip(srcData[i] - 100, 0, 255);
	}
	Invalidate(FALSE);
}

void CChildView::OnArithmeticMultiply()
{
	if(pDib==NULL)
		return;
	for(int i=0; i<Width*Height; i++)
	{
		dstData[i] = Clip(srcData[i]*2, 0, 255);
	}
	Invalidate(FALSE);
}

void CChildView::OnArithmeticDivide()
{
	if(pDib==NULL)
		return;
	for(int i=0; i<Width*Height; i++)
	{
		dstData[i] = Clip(srcData[i]/2, 0, 255);
	}
	Invalidate(FALSE);
}

void CChildView::OnArithmeticNegative()
{
	if(pDib==NULL)
		return;
	for(int i=0; i<Width*Height; i++)
	{
		dstData[i] = 255-srcData[i];
	}
	Invalidate(FALSE);
}

void CChildView::OnArithmeticPic()
{
	unsigned char *tempData;
	BITMAPINFO *tempInfo;

	CString szFilter = _T ("bitmap Files (*.bmp)|*.bmp|All Files (*.*)|*.*|");

	CFileDialog dlg (TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT |
		OFN_ALLOWMULTISELECT, szFilter, this);
	if(dlg.DoModal () == IDCANCEL)
		return;

	FILE *file;
	_wfopen_s(&file, dlg.GetPathName(), L"rb");

	BITMAPFILEHEADER bitmapFileHeader;
	fread(&bitmapFileHeader, sizeof(BITMAPFILEHEADER), 1, file);

	int dibSize = bitmapFileHeader.bfSize-sizeof(BITMAPFILEHEADER);

	if(tDib)
		delete[] tDib;

	tDib = new unsigned char[dibSize];

	fread(tDib, dibSize, 1, file);
	
	tempInfo = (BITMAPINFO *) tDib;

	tempData =  tDib + sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * tempInfo->bmiHeader.biClrUsed;

	if(pDib==NULL)
		return;

	for(int i=0; i<Width*Height; i++)
	{
		dstData[i] = Clip(srcData[i]+tempData[i],0,255);
	}

	fclose(file);

	Invalidate(FALSE);
}

void CChildView::OnGeometryFlipH()
{
	if(pDib==NULL)
		return;
	for(int i=0; i<Height; i++)
	{
		int Hlevel = i*Step;
		for(int j=0; j<Width; j++)
		{
			dstData[Hlevel+j]=srcData[Hlevel+(Width-j-1)];
		}
	}
	Invalidate(FALSE);
}

void CChildView::OnGeometryFlipV()
{
	if(pDib==NULL)
		return;
	for(int i=0; i<Height; i++)
	{
		memcpy(dstData+i*Step,srcData+(Height-i-1)*Step,Step);
	}
	Invalidate(FALSE);
}

void CChildView::OnGeometryRotateL()
{
	if(pDib==NULL)
		return;
	for(int i=0; i<Height; i++)
	{
		for(int j=0; j<Width; j++)
		{
			dstData[i*Step+j]=srcData[(Step-j-1)*Step+i];
		}
	}
	Invalidate(FALSE);
}

void CChildView::OnGeometryRotateR()
{
	if(pDib==NULL)
		return;
	for(int i=0; i<Height; i++)
	{
		for(int j=0; j<Width; j++)
		{
			dstData[i*Step+j]=srcData[j*Step+Height-i-1];
		}
	}
	Invalidate(FALSE);
}

int CChildView::GetRealWidth(int width)
{
	int real_width;
	div_t r;
	r = div(width,4);
	if(r.rem !=0)
	{
		real_width = ((r.quot+1)*4);
		return real_width;
	}
	else
		return width;
}

unsigned char CChildView::Clip(int value, int low, int high)
{
	if(value<low)
		return value = (unsigned char)low;
	else if(value > high)
		return value = (unsigned char)high;
	else
		return (unsigned char)value;
}