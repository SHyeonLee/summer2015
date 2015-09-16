
// ChildView.cpp : implementation of the CChildView class
//

#include "stdafx.h"
#include "practice_imgProc.h"
#include "ChildView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CChildView

CChildView::CChildView()
{
	dibData     = NULL;
	dibImage    = NULL;

	dstData     = NULL;
	srcData     = NULL;

	rBtnDown	= FALSE;
	rClick		= FALSE;
	rMove		= FALSE;
}

CChildView::~CChildView()
{
	if(dibData)
		delete[] dibData;
	if(dstData)
		delete[] dstData;
	if(srcData)
		delete[] srcData;
}


BEGIN_MESSAGE_MAP(CChildView, CWnd)
	ON_WM_PAINT()
	ON_COMMAND(ID_FILE_OPEN,			&CChildView::OnFileOpen)
	ON_COMMAND(ID_ARITHMETIC_ADDC,		&CChildView::OnArithmeticAddC)
	ON_COMMAND(ID_ARITHMETIC_SUBC,		&CChildView::OnArithmeticSubC)
	ON_COMMAND(ID_ARITHMETIC_MULC,		&CChildView::OnArithmeticMulC)
	ON_COMMAND(ID_ARITHMETIC_DIVC,		&CChildView::OnArithmeticDivC)
	ON_COMMAND(ID_GEOMETRY_FLIPH,		&CChildView::OnGeometryFlipH)
	ON_COMMAND(ID_GEOMETRY_FLIPV,		&CChildView::OnGeometryFlipV)
	ON_COMMAND(ID_GEOMETRY_ROTATER,		&CChildView::OnGeometryRotateR)
	ON_COMMAND(ID_GEOMETRY_ROTATEL,		&CChildView::OnGeometryRotateL)
	ON_COMMAND(ID_FILTER_AVR,			&CChildView::OnFilterAvr)
	ON_COMMAND(ID_FILTER_SHARP,			&CChildView::OnFilterSharpen)

//	ON_WM_RBUTTONDBLCLK()
	ON_WM_RBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_RBUTTONUP()
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

	if(dibData==NULL)
		return;

	::SetDIBitsToDevice(dc.m_hDC,
		0,0,srcStep,height,
		0,0,0,height,
		dibImage,bitmapInfo,DIB_RGB_COLORS);
}

void CChildView::OnFileOpen()
{
	CString szFilter = _T ("DICOM Files (*.dcm)|*.dcm|bitmap Files (*.bmp)|*.bmp|All Files (*.*)|*.*|");

	CFileDialog dlg (TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT |
		OFN_ALLOWMULTISELECT, szFilter, this);
	if(dlg.DoModal () == IDCANCEL)
		return;

	CString ext = dlg.GetFileExt();

	ext.MakeUpper();	//capital character
	if(ext == L"BMP")
		OpenBITMAPFile(dlg.GetPathName());
	else if(ext == L"DCM")
		OpenDICOMFile(dlg.GetPathName());

	Invalidate(FALSE);
}

void CChildView::OpenBITMAPFile(CString path)
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
	width = bitmapInfo->bmiHeader.biWidth;
	height = bitmapInfo->bmiHeader.biHeight;
	srcStep = GetRealWidth(width);
	srcData =  dibData + sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * bitmapInfo->bmiHeader.biClrUsed;

	//Allocate destination memory
	if(dstData)
		delete[] dstData;

	dstData = new unsigned char[srcStep*height];

	memcpy(dstData, srcData, srcStep*height);

	memcpy(dibImage, dstData, srcStep*height);

	//close file
	fclose(file);
	
	//Repaint client area
	Invalidate(FALSE);
}

void CChildView::OpenDICOMFile(CString path)
{
	//Allocate KDicomDS object
	KDicomDS *dicomDS = new KDicomDS;

	//Load File
	dicomDS-> LoadDS(path);

	//Get Parameters
	samplePerPixel		= dicomDS->m_nSamplePerPixel;
	photometric			= dicomDS->m_nPhotometric;
	width				= dicomDS->m_nWidth;
	height				= dicomDS->m_nHeight;
	bitsAllocated		= dicomDS->m_nBitsAllocated;
	bitsStored			= dicomDS->m_nBitsStored;
	pixelRepresentation	= dicomDS->m_nRepresentation;
	windowCenter		= dicomDS->m_dWindowCenter;
	windowWidth			= dicomDS->m_dWindowWidth;

	if(bitsAllocated == 8)
		srcStep	= width*samplePerPixel;
	else
		srcStep	= width*2;

	//Allocate image processing source memory
	if(srcData)
		delete[] srcData;

	srcData = new unsigned char[srcStep*height];
	dicomDS-> GetImageData(srcData);

	//Allocate image processing destination memory
	if(dstData)
		delete[] dstData;
	dstData = new unsigned char[srcStep*height];
	memcpy(dstData, srcData, srcStep*height);

	//Delete DicomDS
	delete dicomDS;
	
	CreateDIB();
	Trans16to8();
}

BOOL CChildView::CreateDIB()
{
	int colorNum	= 256;
	dibStep			= GetRealWidth(width);

	//Calculate DIB size
	int dibSize		= sizeof(BITMAPINFOHEADER)+sizeof(RGBQUAD)*colorNum+dibStep*height;

	//Allocated DIB size
	if(dibData)
		delete[] dibData;
	dibData = new unsigned char[dibSize];

	bitmapInfo = (BITMAPINFO*) dibData;

	//Make BITMAPINFOHEADER
	bitmapInfo ->bmiHeader.biSize				= sizeof(BITMAPINFOHEADER);
	bitmapInfo ->bmiHeader.biWidth				= width;
	bitmapInfo ->bmiHeader.biHeight				= height;
	bitmapInfo ->bmiHeader.biPlanes				= 1;
	bitmapInfo ->bmiHeader.biBitCount			= WORD(samplePerPixel*8);
	bitmapInfo ->bmiHeader.biCompression		= 0;
	bitmapInfo ->bmiHeader.biSizeImage			= dibStep*height;
	bitmapInfo ->bmiHeader.biXPelsPerMeter		= 0;
	bitmapInfo ->bmiHeader.biYPelsPerMeter		= 0;
	bitmapInfo ->bmiHeader.biClrUsed			= colorNum;
	bitmapInfo ->bmiHeader.biClrImportant		= 0;

	dibImage = dibData+sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD)*colorNum;

	if(photometric == MONOCHROME1){
		for(int i = 0; i<colorNum; i++){
			bitmapInfo->bmiColors[i].rgbRed			= 255-i;
			bitmapInfo->bmiColors[i].rgbGreen		= 255-i;
			bitmapInfo->bmiColors[i].rgbBlue		= 255-i;
			bitmapInfo->bmiColors[i].rgbReserved	= 0;
		}
	}
	else if(photometric == MONOCHROME2){
		for(int i = 0; i<colorNum; i++){
			bitmapInfo->bmiColors[i].rgbRed			= i;
			bitmapInfo->bmiColors[i].rgbGreen		= i;
			bitmapInfo->bmiColors[i].rgbBlue		= i;
			bitmapInfo->bmiColors[i].rgbReserved	= 0;
		}
	}
	return TRUE;
}

void CChildView::Trans16to8()
{
	if(bitsAllocated == 8)
		return;

	short * src = (short *) dstData;
	int low  = windowCenter - windowWidth / 2;
	int high = windowCenter + windowWidth / 2;
	double ratio = 255 / windowWidth;
	short value;
	for(int i=0;i<height;i++){
		for(int j=0;j<width;j++){
			value = src[i*width + j];
			if(value < low)
				dibImage[(height - 1 - i)*dibStep + j] = 0;
			else if(value > high)
				dibImage[(height - 1 - i)*dibStep + j] = 255;
			else{
				dibImage[(height - 1 - i)*width + j] = (value - low) * ratio;
			}
		}
	}
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