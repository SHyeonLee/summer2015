
// ChildView.cpp : implementation of the CChildView class
//

#include "stdafx.h"
#include "OpenBitmap.h"
#include "ChildView.h"
#include "math.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CChildView

CChildView::CChildView()
{
	dibData = NULL;
	dstData = NULL;
	tDib = NULL;
	rightButtonDown = FALSE;
}

CChildView::~CChildView()
{
	if(dibData)
		delete[] dibData;

	if(tDib)
		delete[] tDib;


	if(dstData)
		delete[] dstData;

}


BEGIN_MESSAGE_MAP(CChildView, CWnd)
	ON_WM_PAINT()
	ON_COMMAND(ID_TEST_HELLOWORLD,			&CChildView::OnTestHelloworld)
	ON_COMMAND(ID_FILE_OPEN,				&CChildView::OnFileOpen)
	ON_COMMAND(ID_ARITHMETIC_ADD,			&CChildView::OnArithmeticAdd)
	ON_COMMAND(ID_ARITHMETIC_SUB,			&CChildView::OnArithmeticSub)
	ON_COMMAND(ID_ARITHMETIC_MULTIPLY,		&CChildView::OnArithmeticMultiply)
	ON_COMMAND(ID_ARITHMETIC_DIVIDE,		&CChildView::OnArithmeticDivide)
	ON_COMMAND(ID_ARITHMETIC_NEGATIVE,		&CChildView::OnArithmeticNegative)
	ON_COMMAND(ID_ARITHMETIC_PIC,			&CChildView::OnArithmeticPic)
	ON_COMMAND(ID_GEOMETRY_FLIPH,			&CChildView::OnGeometryFlipH)
	ON_COMMAND(ID_GEOMETRY_FLIPV,			&CChildView::OnGeometryFlipV)
	ON_COMMAND(ID_GEOMETRY_ROTATEL,			&CChildView::OnGeometryRotateL)
	ON_COMMAND(ID_GEOMETRY_ROTATER,			&CChildView::OnGeometryRotateR)
	ON_COMMAND(ID_FILTER_BLUR,				&CChildView::OnFilterBlur)
	ON_COMMAND(ID_FILTER_SHARPEN,			&CChildView::OnFilterSharpen)

	ON_UPDATE_COMMAND_UI(ID_LUT_ADD,		&CChildView::OnUpdateLutAdd)
	ON_COMMAND(ID_LUT_ADD,					&CChildView::OnLutAdd)
	ON_COMMAND(ID_LUT_NEG,					&CChildView::OnLutNegative)
	ON_COMMAND(ID_LUT_GAMMA,				&CChildView::OnLutGamma)
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEMOVE()
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

	/*CPen Pen;
	Pen.CreatePen(PS_SOLID, 1, RGB(0,0,255));
	CPen *pOldPen;
	pOldPen = dc.SelectObject(&Pen);

	CBrush Brush;
	Brush.CreateSolidBrush(RGB(0,255,0));
	CBrush *pOldBrush = dc.SelectObject(&Brush);
	{
	dc.MoveTo(0,0);
	dc.LineTo(100,100);

	dc.Rectangle(100,100,200,200);
	}
	dc.SelectObject(pOldPen);
	dc.SelectObject(pOldBrush);
	Brush.DeleteObject();
	Pen.DeleteObject();*/

	if(dibData==NULL)
		return;

	::SetDIBitsToDevice(dc.m_hDC,
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
	CString szFilter = _T ("dicom Files (*.dcm)|*.dcm|bitmap Files (*.bmp)|*.bmp|All Files (*.*)|*.*|");

	CFileDialog dlg (TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT |
		OFN_ALLOWMULTISELECT, szFilter, this);
	if(dlg.DoModal () == IDCANCEL)
		return;

	CString ext =dlg.GetFileExt();
	ext.MakeUpper();
	if(ext == L"BMP")
		OpenBMPFile(dlg.GetPathName());
	else if(ext == L"DCM")
		OpenDCMFile(dlg.GetPathName());

		//close file
}

void CChildView::OpenBMPFile(CString path)
{
	FILE *file;
	_wfopen_s(&file, path, L"rb");

	BITMAPFILEHEADER bitmapFileHeader;

	fread(&bitmapFileHeader, sizeof(BITMAPFILEHEADER), 1, file);

	int dibSize = bitmapFileHeader.bfSize-sizeof(BITMAPFILEHEADER);

	if(dibData)
		delete[] dibData;

	dibData = new unsigned char[dibSize];

	fread(dibData, dibSize, 1, file);

	//Important Variables
	bitmapInfo = (BITMAPINFO *) dibData;
	samplePerPixel = bitmapInfo->bmiHeader.biBitCount/8;
	Width = bitmapInfo->bmiHeader.biWidth;
	Height = bitmapInfo->bmiHeader.biHeight;
	srcData =  dibData + sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * bitmapInfo->bmiHeader.biClrUsed;

	//Allocate destination memory
	if(dstData)
		delete[] dstData;

	dstData = new unsigned char[Step*Height];

	//compy src to dst
	memcpy(dstData, srcData, Step*Height);

	fclose(file);

	//Repaint client area
	Invalidate(FALSE);

}

void CChildView::OpenDCMFile(CString path)
{
	KDicomDS * ds = new KDicomDS; //DS : DataSet

	ds->LoadDS(path, FALSE);

	KDicomElement * de;

	// Sample per pixel
	de = ds->GetElement(0x0028, 0x0002);
	if(de)
	{
		samplePerPixel = de->GetValueUS(0);
	}

	// Photometric Representation
	de = ds->GetElement(0x0028,0x0004);
	if(de)
	{
		Photometric = de->GetValueCS(0);
	}

	// Width
	de = ds->GetElement(0x0028,0x0011);
	if(de)
	{
		Width = de->GetValueUS(0);
	}

	// Height
	de = ds->GetElement(0x0028,0x0010);
	if(de)
	{
		Height = de->GetValueUS(0);
	}

	// Bits Allocated
	de = ds->GetElement(0x0028,0x00100);
	if(de)
	{
		BitsAllocated = de->GetValueUS(0);
	}

	// Bits Stored
	de = ds->GetElement(0x0028,0x0101);
	if(de)
	{
		BitsStored = de->GetValueUS(0);
	}

	// Pixel Representation
	de = ds->GetElement(0x0028,0x0103);
	if(de)
	{
		PixelRepresentation = de->GetValueUS(0);
	}

	// Window Width
	de = ds->GetElement(0x0028,0x1051);
	if(de)
	{
		WindowWidth = _wtof(de->GetValueDS(0));
	}

	// Window Center
	de = ds->GetElement(0x0028,0x1050);
	if(de)
	{
		WindowCenter = _wtof(de->GetValueDS(0));
	}

	// Image Data
	de = ds->GetElement(0x7FE0, 0x0010);
	if(de)
	{
		unsigned char * temp = de->GetValueOB();

		srcData = new unsigned char [Width*Height*2];
		memcpy(srcData, temp, Width*Height*2);
	}

	CreateDIB();

	Step = GetRealWidth(Width);

	if(dstData)
		delete[] dstData;
	dstData = new unsigned char[Step*Height];
	
	// 16bit to 8bit

	src = (short *) srcData;
	short imagevalue;

	short WindowLow		= WindowCenter - WindowWidth / 2;
	short WindowHigh	= WindowCenter + WindowWidth / 2;
	double ratioLevel	= 255/WindowWidth;

	for(int i=0; i<Height; i++){
		for(int j=0; j<Width; j++){
			imagevalue = src[Height*i+j];
			dstData[(Height-1-i)*Step+j] = bit16to8(imagevalue, WindowHigh, WindowLow,ratioLevel);
		}
	}
	//test
	
	delete ds;

	Invalidate(FALSE);
}

unsigned char CChildView::bit16to8(short ivalue, short High, short Low, double ratio)
{
	if(ivalue < Low)
		return 0;
	else if(ivalue > High)
		return 255;
	else
		return (ivalue - Low)* ratio;
}

BOOL CChildView::CreateDIB()
{
	int colorNum = 256;

	//Calculate DIB Size
	int dibSize = sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD)*colorNum;

	//Allocate
	if(dibData)
		delete[] dibData;
	dibData = new unsigned char [dibSize];

	bitmapInfo = (BITMAPINFO *) dibData;
	bitmapInfo->bmiHeader.biSize			= sizeof(BITMAPINFOHEADER);
	bitmapInfo->bmiHeader.biWidth			= Width;
	bitmapInfo->bmiHeader.biHeight			= Height;
	bitmapInfo->bmiHeader.biPlanes			= 1;
	bitmapInfo->bmiHeader.biBitCount		= WORD(samplePerPixel * 8);
	bitmapInfo->bmiHeader.biCompression		= 0;
	bitmapInfo->bmiHeader.biSizeImage		= Step * Height;
	bitmapInfo->bmiHeader.biXPelsPerMeter	= 0;
	bitmapInfo->bmiHeader.biYPelsPerMeter	= 0;
	bitmapInfo->bmiHeader.biClrUsed			= 0;
	bitmapInfo->bmiHeader.biClrImportant	= 0;

	for(int i=0; i<colorNum; i++)
	{
		bitmapInfo->bmiColors[i].rgbRed			= i;
		bitmapInfo->bmiColors[i].rgbGreen		= i;
		bitmapInfo->bmiColors[i].rgbBlue		= i;
		bitmapInfo->bmiColors[i].rgbReserved	= 0;
	}
	return TRUE;
}

void CChildView::OnArithmeticAdd()
{
	if(dibData == NULL)
		return;

	for(int i=0; i<Width*Height; i++)
	{
		dstData[i] = Clip(srcData[i] + 100, 0, 255);
	}
	Invalidate(FALSE);
}

void CChildView::OnArithmeticSub()
{
	if(dibData==NULL)
		return;
	for(int i=0; i<Width*Height; i++)
	{
		dstData[i] = Clip(srcData[i] - 100, 0, 255);
	}
	Invalidate(FALSE);
}

void CChildView::OnArithmeticMultiply()
{
	if(dibData==NULL)
		return;
	for(int i=0; i<Width*Height; i++)
	{
		dstData[i] = Clip(srcData[i]*2, 0, 255);
	}
	Invalidate(FALSE);
}

void CChildView::OnArithmeticDivide()
{
	if(dibData==NULL)
		return;
	for(int i=0; i<Width*Height; i++)
	{
		dstData[i] = Clip(srcData[i]/2, 0, 255);
	}
	Invalidate(FALSE);
}

void CChildView::OnArithmeticNegative()
{
	if(dibData==NULL)
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

	if(dibData==NULL)
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
	if(dibData==NULL)
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
	if(dibData==NULL)
		return;
	for(int i=0; i<Height; i++)
	{
		memcpy(dstData+i*Step,srcData+(Height-i-1)*Step,Step);
	}
	Invalidate(FALSE);
}

void CChildView::OnGeometryRotateL()
{
	if(dibData==NULL)
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
	if(dibData==NULL)
		return;
	for(int i=0; i<Height; i++)
	{
		for(int j=0; j<Width; j++)
		{
			dstData[i*Step+j]=srcData[j*Step+(Height-i-1)];
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

void CChildView::OnUpdateLutAdd(CCmdUI *pCmdUI)
{
	if(dibData==NULL)
		pCmdUI->Enable(FALSE);
}


void CChildView::OnLutAdd()
{
	unsigned char lut[256];

	for(int i=0; i<256; i++){
		lut[i] = Clip(i+100,0,255);
	}

	for(int i=0; i < Height*Step; i++){
		dstData[i] = lut[srcData[i]];
	}
	Invalidate(FALSE);
}

void CChildView::OnLutNegative()
{
	unsigned char lut[256];

	for(int i=0; i<256; i++){
		lut[i] = 255-i;
	}

	for(int i=0; i < Height*Step; i++){
		dstData[i] = lut[srcData[i]];
	}
	Invalidate(FALSE);
}

void CChildView::OnLutGamma()
{
	unsigned char lut[256];

	for(int i=0; i<256; i++){
		lut[i] = Clip(pow(i/255.,2.)*255,0,255);
	}

	for(int i=0; i < Height*Step; i++){
		dstData[i] = lut[srcData[i]];
	}
	Invalidate(FALSE);
}

//int CChildView::OnCreate(LPCREATESTRUCT lpCreateStruct)
//{
//	if (CWnd::OnCreate(lpCreateStruct) == -1)
//		return -1;
//
//	scrollBar.Create(SBS_HORZ | WS_CHILD | WS_VISIBLE,CRect(0, 520, 500, 520 + 10),this, 9999);
//	scrollBar.SetScrollRange(0, 100);
//	scrollBar.SetScrollPos(0);
//
//	return 0;
//}
//
//
//void CChildView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
//{
//	double gamma = 1. - nPos/200.;
//	GammaCorrection(gamma);
//
//	CWnd::OnHScroll(nSBCode, nPos, pScrollBar);
//}
//
//void CChildView::GammaCorrection(double gamma)
//{
//	unsigned char lut[256];
//
//	double exp = 1. / gamma;
//
//	for(int i=0; i<256; i++){
//		lut[i] = Clip(pow(i/255.,exp)*255,0,255);
//	}
//
//	for(int i=0; i < Height*Step; i++){
//		dstData[i] = lut[srcData[i]];
//	}
//	Invalidate(FALSE);
//}

void CChildView::OnFilterBlur()
{
	double mask[9] = {1/9., 1/9., 1/9.,
					1/9., 1/9., 1/9.,
					1/9., 1/9., 1/9.};
	SpatialFilter(mask);
}

void CChildView::OnFilterSharpen()
{
	double mask[9] = {0, -1, 0,
					-1, 5, -1,
					0, -1, 0};
	SpatialFilter(mask);
}

void CChildView::SpatialFilter(double *mask)
{
	double sum;
	for(int i=0; i<Width-1; i++)
	{
		for(int j=0; j<Height-1; j++)
		{
			sum = 0;
			for(int k=0; k<3; k++)
			{
				for(int l=0; l<3; l++)
					sum += srcData[(i+k-1)*Step+j+l-1]*mask[k*3+l];
			}
			dstData[i*Step+j]=Clip(int(sum),0,255);
		}
	}
	Invalidate(FALSE);
}

void CChildView::OnRButtonDown(UINT nFlags, CPoint point)
{
	rightButtonDown = TRUE;
	rightbuttonPoint = point;
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.

	CWnd::OnRButtonDown(nFlags, point);
}


void CChildView::OnRButtonUp(UINT nFlags, CPoint point)
{
	rightButtonDown = FALSE;
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.

	CWnd::OnRButtonUp(nFlags, point);
}


void CChildView::OnMouseMove(UINT nFlags, CPoint point)
{
	if(rightButtonDown){
		int xdiff = rightbuttonPoint.x - point.x;
		int ydiff = rightbuttonPoint.y - point.y;

		WindowWidth += ydiff/10;
		WindowCenter += xdiff/10;

		short imagevalue;
		double WindowLow = WindowCenter - WindowWidth/2;
		double WindowHigh = WindowCenter + WindowWidth/2;
		double ratioLevel = 255/WindowWidth;

		for(int i=0; i<Height; i++){
			for(int j=0; j<Width; j++){
				imagevalue = src[Height*i+j];
				dstData[(Height-1-i)*Step+j] = bit16to8(imagevalue, WindowHigh, WindowLow,ratioLevel);
			}
		}
			Invalidate(FALSE);
	}

	//CWindowDC dc(GetParent());
	//CClientDC dc2(this);
	//CString str;
	//str.Format(L"%010d,%010d", WindowCenter, WindowWidth);
	//dc.TextOutW(160,600,str);
	//dc2.TextOutW(160,600,str);
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.

	CWnd::OnMouseMove(nFlags, point);
}
