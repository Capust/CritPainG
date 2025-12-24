
// CritPainGView.h : interface of the CCritPainGView class
//

#pragma once


class CCritPainGView : public CView
{
protected: // create from serialization only
	CCritPainGView() noexcept;
	DECLARE_DYNCREATE(CCritPainGView)

// Attributes
public:
	CCritPainGDoc* GetDocument() const;

// Operations
public:

// Overrides
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual void OnInitialUpdate() override;
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// Implementation
public:
	virtual ~CCritPainGView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	void DrawFunctionMap(CDC& dc, const CRect& rect);
	void DrawTrajectory(CDC& dc, const CRect& rect);
	CPoint WorldToScreen(double x, double y, const CRect& rect);

	// Битовый буфер для двойной буферизации
	CBitmap m_bufferBitmap;
	CSize m_lastBufferSize;

// Generated message map functions
protected:
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnFilePrintPreview();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in CritPainGView.cpp
inline CCritPainGDoc* CCritPainGView::GetDocument() const
   { return reinterpret_cast<CCritPainGDoc*>(m_pDocument); }
#endif

