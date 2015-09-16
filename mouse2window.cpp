
// ChildView.cpp : implementation of the CChildView class
//

#include "stdafx.h"
#include "practice_imgProc.h"
#include "ChildView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


//void CChildView::OnRButtonDblClk(UINT nFlags, CPoint point)
//{
//	rClick = point;	
//	CWnd::OnRButtonDblClk(nFlags, point);
//}


void CChildView::OnRButtonDown(UINT nFlags, CPoint point)
{
	rClick		= point;

	windowCenterTemp  = windowCenter;
	windowWidthTemp   = windowWidth;

	rBtnDown	= TRUE;

	CWnd::OnRButtonDown(nFlags, point);
}


void CChildView::OnRButtonUp(UINT nFlags, CPoint point)
{
	rBtnDown = FALSE;

	CWnd::OnRButtonUp(nFlags, point);
}



void CChildView::OnMouseMove(UINT nFlags, CPoint point)
{
	rMove = TRUE;
	if(rBtnDown){
		if(bitsAllocated == 16){
			int dx = (point.x - rClick.x) * 10;
			int dy = (point.y - rClick.y) * 10;

			windowWidth    = windowWidthTemp + dx;
			windowCenter   = windowCenterTemp + dy;

			Trans16to8();
			Invalidate(FALSE);
		}
	}

	CWnd::OnMouseMove(nFlags, point);
}
