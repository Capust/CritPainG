
// CritPainGView.cpp : implementation of the CCritPainGView class
//

#include "pch.h"
#include "framework.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "CritPainG.h"
#endif

#include "CritPainGDoc.h"
#include "CritPainGView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CCritPainGView

IMPLEMENT_DYNCREATE(CCritPainGView, CView)

BEGIN_MESSAGE_MAP(CCritPainGView, CView)
	// Standard printing commands
	ON_WM_LBUTTONDOWN()
	ON_WM_SIZE()
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CCritPainGView::OnFilePrintPreview)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
END_MESSAGE_MAP()

// CCritPainGView construction/destruction


CCritPainGView::CCritPainGView() noexcept
{
	m_lastBufferSize = CSize(0, 0);
}
CCritPainGView::~CCritPainGView()
{
}
void CCritPainGView::OnInitialUpdate()
{
	CView::OnInitialUpdate();
}

BOOL CCritPainGView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

// CCritPainGView drawing

void CCritPainGView::OnDraw(CDC* pDC)
{
    CCritPainGDoc* pDoc = GetDocument();
    ASSERT_VALID(pDoc);
    if (!pDoc)
        return;

    CRect rect;
    GetClientRect(&rect);

    // Создаем совместимый контекст для двойной буферизации
    CDC dcMem;
    dcMem.CreateCompatibleDC(pDC);

    // Создаем или обновляем битмап буфера
    if (m_bufferBitmap.m_hObject == NULL ||
        m_lastBufferSize != rect.Size())
    {
        m_bufferBitmap.DeleteObject();
        m_bufferBitmap.CreateCompatibleBitmap(pDC, rect.Width(), rect.Height());
        m_lastBufferSize = rect.Size();
    }

    CBitmap* pOldBitmap = dcMem.SelectObject(&m_bufferBitmap);

    // Очищаем фон
    dcMem.FillSolidRect(rect, RGB(255, 255, 255));

    // Рисуем карту функции
    DrawFunctionMap(dcMem, rect);

    // Рисуем траекторию, если есть
    if (pDoc->HasTrajectory() && pDoc->GetTrajectory().size() > 1)
    {
        DrawTrajectory(dcMem, rect);
    }

    // Выводим информацию
    CFont font;
    font.CreatePointFont(100, _T("Arial"));
    CFont* pOldFont = dcMem.SelectObject(&font);

    dcMem.SetTextColor(RGB(0, 0, 0));
    dcMem.SetBkMode(TRANSPARENT);

    CString info;
    const auto& initialPoint = pDoc->GetInitialPoint();
    const auto& finalPoint = pDoc->GetFinalPoint();

    if (pDoc->HasTrajectory())
    {
        info.Format(_T("Начальная точка: (%.3f, %.3f)\n")
            _T("Финальная точка: (%.3f, %.3f)\n")
            _T("Значение функции: %.6f\n")
            _T("Итераций: %d"),
            initialPoint[0], initialPoint[1],  // Начальная точка
            finalPoint[0], finalPoint[1],      // Финальная точка
            pDoc->GetFinalValue(),
            pDoc->GetIterations());
    }
    else
    {
        // Показываем только начальную точку
        info.Format(_T("Начальная точка: (%.3f, %.3f)\n")
            _T("Щелкните мышью для выбора начальной точки"),
            initialPoint[0], initialPoint[1]);
    }

    CRect textRect(rect);
    textRect.DeflateRect(10, 10);
    dcMem.DrawText(info, textRect, DT_LEFT | DT_TOP);

    dcMem.SelectObject(pOldFont);

    // Копируем из буфера на экран
    pDC->BitBlt(0, 0, rect.Width(), rect.Height(), &dcMem, 0, 0, SRCCOPY);

    dcMem.SelectObject(pOldBitmap);
}
// CCritPainGView printing


void CCritPainGView::OnFilePrintPreview()
{
#ifndef SHARED_HANDLERS
	AFXPrintPreview(this);
#endif
}

BOOL CCritPainGView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CCritPainGView::DrawFunctionMap(CDC& dc, const CRect& rect)
{
    CCritPainGDoc* pDoc = GetDocument();
    if (!pDoc || !pDoc->HasFunction() || rect.Width() <= 0 || rect.Height() <= 0)
        return;

    double xMin = pDoc->GetXMin();
    double xMax = pDoc->GetXMax();
    double yMin = pDoc->GetYMin();
    double yMax = pDoc->GetYMax();

    // Размер сетки
    const int GRID_X = 200;
    const int GRID_Y = 200;

    // Найдем диапазон значений функции
    double fMin = DBL_MAX;
    double fMax = -DBL_MAX;

    // Быстрый проход для определения диапазона
    for (int i = 0; i < GRID_X; i += 10)
    {
        double x = xMin + (xMax - xMin) * i / (GRID_X - 1);
        for (int j = 0; j < GRID_Y; j += 10)
        {
            double y = yMin + (yMax - yMin) * j / (GRID_Y - 1);
            double val = pDoc->CalculateFunctionValue(x, y);
            fMin = min(fMin, val);
            fMax = max(fMax, val);
        }
    }

    // Если все значения одинаковые
    if (fabs(fMax - fMin) < 1e-10)
    {
        fMin -= 1.0;
        fMax += 1.0;
    }

    // Рисуем каждый пиксель
    for (int sx = 0; sx < rect.Width(); sx++)
    {
        double x = xMin + (xMax - xMin) * sx / rect.Width();

        for (int sy = 0; sy < rect.Height(); sy++)
        {
            double y = yMax - (yMax - yMin) * sy / rect.Height();

            double val = pDoc->CalculateFunctionValue(x, y);

            // Нормализуем значение
            double t = (val - fMin) / (fMax - fMin);
            t = max(0.0, min(1.0, t));

            // Градиент: от синего (мин) к красному (макс)
            int r = static_cast<int>(255 * t);
            int g = static_cast<int>(100 * (1 - t));
            int b = static_cast<int>(255 * (1 - t));

            dc.SetPixel(rect.left + sx, rect.top + sy, RGB(r, g, b));
        }
    }

    // Рамка области
    CPen borderPen(PS_SOLID, 2, RGB(0, 0, 0));
    CPen* pOldPen = dc.SelectObject(&borderPen);

    // Углы в экранных координатах
    CPoint topLeft(rect.left, rect.top);
    CPoint topRight(rect.right, rect.top);
    CPoint bottomLeft(rect.left, rect.bottom);
    CPoint bottomRight(rect.right, rect.bottom);

    // Рисуем рамку
    dc.MoveTo(topLeft);
    dc.LineTo(topRight);
    dc.LineTo(bottomRight);
    dc.LineTo(bottomLeft);
    dc.LineTo(topLeft);

    dc.SelectObject(pOldPen);
}

void CCritPainGView::DrawTrajectory(CDC& dc, const CRect& rect)
{
    CCritPainGDoc* pDoc = GetDocument();
    if (!pDoc || !pDoc->HasTrajectory())
        return;

    const auto& trajectory = pDoc->GetTrajectory();
    if (trajectory.size() < 2)
        return;

    // Рисуем линии траектории
    CPen trajectoryPen(PS_SOLID, 2, RGB(255, 0, 0));
    CPen* pOldPen = dc.SelectObject(&trajectoryPen);

    for (size_t i = 0; i < trajectory.size() - 1; i++)
    {
        const auto& pt1 = trajectory[i];
        const auto& pt2 = trajectory[i + 1];

        CPoint screen1 = WorldToScreen(pt1[0], pt1[1], rect);
        CPoint screen2 = WorldToScreen(pt2[0], pt2[1], rect);

        dc.MoveTo(screen1);
        dc.LineTo(screen2);
    }

    // Рисуем начальную точку (зеленый)
    const auto& startPt = trajectory.front();
    CPoint startScreen = WorldToScreen(startPt[0], startPt[1], rect);
    CBrush startBrush(RGB(0, 255, 0));
    CBrush* pOldBrush = dc.SelectObject(&startBrush);
    dc.Ellipse(startScreen.x - 5, startScreen.y - 5,
        startScreen.x + 5, startScreen.y + 5);

    // Рисуем конечную точку (красный)
    const auto& endPt = trajectory.back();
    CPoint endScreen = WorldToScreen(endPt[0], endPt[1], rect);
    CBrush endBrush(RGB(255, 0, 0));
    dc.SelectObject(&endBrush);
    dc.Ellipse(endScreen.x - 5, endScreen.y - 5,
        endScreen.x + 5, endScreen.y + 5);

    dc.SelectObject(pOldBrush);
    dc.SelectObject(pOldPen);
}

CPoint CCritPainGView::WorldToScreen(double x, double y, const CRect& rect)
{
    CCritPainGDoc* pDoc = GetDocument();
    if (!pDoc || rect.Width() <= 0 || rect.Height() <= 0)
        return CPoint(rect.left + rect.Width() / 2, rect.top + rect.Height() / 2);

    double xMin = pDoc->GetXMin();
    double xMax = pDoc->GetXMax();
    double yMin = pDoc->GetYMin();
    double yMax = pDoc->GetYMax();

    // Нормализуем координаты
    double normX = (x - xMin) / (xMax - xMin + 1e-10);
    double normY = (y - yMin) / (yMax - yMin + 1e-10);

    // Преобразуем в экранные:
    // X: нормально (слева направо)
    // Y: инвертируем (в мировых координатах Y растет вверх, 
    //    в экранных - вниз)
    int screenX = rect.left + static_cast<int>(normX * rect.Width());
    int screenY = rect.bottom - static_cast<int>(normY * rect.Height());

    // Ограничиваем область
    screenX = max(rect.left, min(rect.right - 1, screenX));
    screenY = max(rect.top, min(rect.bottom - 1, screenY));

    return CPoint(screenX, screenY);
}

void CCritPainGView::OnLButtonDown(UINT nFlags, CPoint point)
{
    CCritPainGDoc* pDoc = GetDocument();
    if (!pDoc)
        return;

    CRect rect;
    GetClientRect(&rect);

    // Преобразуем экранные координаты в мировые
    double xMin = pDoc->GetXMin();
    double xMax = pDoc->GetXMax();
    double yMin = pDoc->GetYMin();
    double yMax = pDoc->GetYMax();

    if (rect.Width() <= 0 || rect.Height() <= 0)
        return;

    double normalizedX = static_cast<double>(point.x - rect.left) / rect.Width();
    double normalizedY = static_cast<double>(rect.bottom - point.y) / rect.Height();

    double worldX = xMin + normalizedX * (xMax - xMin);
    double worldY = yMin + normalizedY * (yMax - yMin);

    // Ограничиваем точку областью
    worldX = max(xMin, min(xMax, worldX));
    worldY = max(yMin, min(yMax, worldY));

    // Устанавливаем начальную точку
    pDoc->SetInitialPoint(worldX, worldY);

    // Запускаем оптимизацию
    if (pDoc->StartOptimization())
    {
        // Обновляем отображение
        Invalidate();
        UpdateWindow();
    }

    CView::OnLButtonDown(nFlags, point);
}
void CCritPainGView::OnSize(UINT nType, int cx, int cy)
{
    CView::OnSize(nType, cx, cy);

    // При изменении размера окна пересоздаем буфер
    m_bufferBitmap.DeleteObject();
    m_lastBufferSize = CSize(0, 0);
    Invalidate();
}

void CCritPainGView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CCritPainGView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

void CCritPainGView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CCritPainGView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}


// CCritPainGView diagnostics

#ifdef _DEBUG
void CCritPainGView::AssertValid() const
{
	CView::AssertValid();
}

void CCritPainGView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CCritPainGDoc* CCritPainGView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CCritPainGDoc)));
	return (CCritPainGDoc*)m_pDocument;
}
#endif //_DEBUG

