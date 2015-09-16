
// ChildView.cpp : implementation of the CChildView class
//

#include "stdafx.h"
#include "practice_imgProc.h"
#include "ChildView.h"
#include "ippi.h"
#include "ipps.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

void CChildView::OnArithmeticAddC()
{
	if(dibData == NULL)
		return;

	short *src = (short*)srcData;
	short *dst = (short*)dstData;

	IppiSize roiSize;
	roiSize.height	= height;
	roiSize.width	= width;
	ippiAddC_16s_C1RSfs(src,srcStep,100,dst,srcStep,roiSize,0);

	Trans16to8();
	Invalidate(FALSE);
}


void CChildView::OnArithmeticSubC()
{
	if(dibData == NULL)
		return;

	short *src = (short*)srcData;
	short *dst = (short*)dstData;

	IppiSize roiSize;
	roiSize.height	= height;
	roiSize.width	= width;
	ippiSubC_16s_C1RSfs(src,srcStep,100,dst,srcStep,roiSize,0);
	
	Trans16to8();
	Invalidate(FALSE);
}

void CChildView::OnArithmeticMulC()
{
	if(dibData == NULL)
		return;

	short *src = (short*)srcData;
	short *dst = (short*)dstData;

	IppiSize roiSize;
	roiSize.height	= height;
	roiSize.width	= width;
	ippiMulC_16s_C1RSfs(src,srcStep,2,dst,srcStep,roiSize,0);

	Trans16to8();
	Invalidate(FALSE);
}

void CChildView::OnArithmeticDivC()
{
	if(dibData == NULL)
		return;

	short *src = (short*)srcData;
	short *dst = (short*)dstData;

	IppiSize roiSize;
	roiSize.height	= height;
	roiSize.width	= width;
	ippiDivC_16s_C1RSfs(src,srcStep,2,dst,srcStep,roiSize,0);

	Trans16to8();
	Invalidate(FALSE);
}

void CChildView::OnGeometryFlipH()
{
	if(dibData == NULL)
		return;

	short *src = (short*)srcData;
	short *dst = (short*)dstData;

	IppiSize roiSize;
	roiSize.height	= height;
	roiSize.width	= width;
	ippiMirror_16s_C1R(src,srcStep,dst,srcStep,roiSize,ippAxsHorizontal);

	Trans16to8();
	Invalidate(FALSE);
}

void CChildView::OnGeometryFlipV()
{
	if(dibData == NULL)
		return;

	short *src = (short*)srcData;
	short *dst = (short*)dstData;

	IppiSize roiSize;
	roiSize.height	= height;
	roiSize.width	= width;
	ippiMirror_16s_C1R(src,srcStep,dst,srcStep,roiSize,ippAxsVertical);

	Trans16to8();
	Invalidate(FALSE);
}

void CChildView::OnGeometryRotateR()
{
	if(dibData == NULL)
		return;

	unsigned short *src = (unsigned short*)srcData;
	unsigned short *dst = (unsigned short*)dstData;

	IppiSize roiSize;
	roiSize.height	= height;
	roiSize.width	= width;

	IppiRect srcSize;
	srcSize.height	= height;
	srcSize.width	= width;
	srcSize.x		= 0;
	srcSize.y		= 0;

	ippiRotate_16u_C1R(src,roiSize,srcStep,srcSize,dst,srcStep,srcSize,270,height,0,IPPI_INTER_NN);

	Trans16to8();
	Invalidate(FALSE);
}

void CChildView::OnGeometryRotateL()
{
	if(dibData == NULL)
		return;

	unsigned short *src = (unsigned short*)srcData;
	unsigned short *dst = (unsigned short*)dstData;

	IppiSize roiSize;
	roiSize.height	= height;
	roiSize.width	= width;

	IppiRect srcSize;
	srcSize.height	= height;
	srcSize.width	= width;
	srcSize.x		= 0;
	srcSize.y		= 0;

	ippiRotate_16u_C1R(src,roiSize,srcStep,srcSize,dst,srcStep,srcSize,90,0,width,IPPI_INTER_NN);

	Trans16to8();
	Invalidate(FALSE);
}

void CChildView::OnFilterAvr()
{
	if(dibData == NULL)
		return;

	short *src = (short*)srcData;
	short *dst = (short*)dstData;

	IppiSize roiSize;
	roiSize.height	= height;
	roiSize.width	= width;

	IppiSize maskSize;
	maskSize.height = 5;
	maskSize.width	= 5;
	
	IppiPoint anchor;
	anchor.x = 2;
	anchor.y = 2;
	
	ippiFilterBox_16s_C1R(src,srcStep,dst,srcStep,roiSize,maskSize,anchor);

	Trans16to8();
	Invalidate(FALSE);
}

void CChildView::OnFilterSharpen()
{
	if(dibData == NULL)
		return;

	short *src = (short*)srcData;
	short *dst = (short*)dstData;

	IppiSize roiSize;
	roiSize.height	= height;
	roiSize.width	= width;

	ippiFilterSharpen_16s_C1R(src,srcStep,dst,srcStep,roiSize);

	Trans16to8();
	Invalidate(FALSE);
}