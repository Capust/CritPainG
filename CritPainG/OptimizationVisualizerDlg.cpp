// OptimizationVisualizerDlg.cpp : implementation file
//

#include "pch.h"
#include "CritPainG.h"
#include "afxdialogex.h"
#include "OptimizationVisualizerDlg.h"
#include "resource.h"

// OptimizationVisualizerDlg dialog

IMPLEMENT_DYNAMIC(OptimizationVisualizerDlg, CDialog)

OptimizationVisualizerDlg::OptimizationVisualizerDlg(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_DIALOG1, pParent)
	, m_X1(-5.0), m_X2(5.0), m_Y1(-5.0), m_Y2(5.0)
	, m_TypeOpt(0)
	, m_DELTA(0.5), m_P(0.2), m_ALPA(0.8), m_EPS(1e-6)
	, m_CriterialType(0)        // По умолчанию: 1000 итераций
	, m_SelectedFunction(0)
{
}

OptimizationVisualizerDlg::~OptimizationVisualizerDlg()
{
}

void OptimizationVisualizerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, X1, m_X1);
	DDX_Text(pDX, X2, m_X2);
	DDX_Text(pDX, Y1, m_Y1);
	DDX_Text(pDX, Y2, m_Y2);
	DDX_Radio(pDX, ConjGr, m_TypeOpt);
	DDX_Text(pDX, DELTA, m_DELTA);
	DDV_MinMaxDouble(pDX, m_DELTA, 0, 1);
	DDX_Text(pDX, PROB, m_P);
	DDV_MinMaxDouble(pDX, m_P, 0, 1);
	DDX_Text(pDX, ALPHA, m_ALPA);
	DDV_MinMaxDouble(pDX, m_ALPA, 0, 1);
	DDX_Text(pDX, EPS, m_EPS);
	DDV_MinMaxDouble(pDX, m_EPS, 0, 1);
	DDX_Radio(pDX, MAXIT, m_CriterialType);
	DDX_Control(pDX, FUNCCONT, m_funcCombo);
}


BEGIN_MESSAGE_MAP(OptimizationVisualizerDlg, CDialog)
	ON_BN_CLICKED(IDOK, &OptimizationVisualizerDlg::OnBnClickedOk)
END_MESSAGE_MAP()

void OptimizationVisualizerDlg::OnBnClickedOk()
{
	m_SelectedFunction = m_funcCombo.GetCurSel();
	// Получаем данные из элементов управления
	UpdateData(TRUE);

	if (m_X1 >= m_X2)
	{
		AfxMessageBox(_T("X2 must be greater than X1"));
		return;
	}

	if (m_Y1 >= m_Y2)
	{
		AfxMessageBox(_T("Y2 must be greater than Y1"));
		return;
	}

	
	if (m_TypeOpt == 1) // Если выбран случайный поиск
	{
		if (m_DELTA <= 0)
		{
			AfxMessageBox(_T("Delta must be positive"));
			return;
		}

		if (m_P <= 0 || m_P >= 1)
		{
			AfxMessageBox(_T("Probability P must be between 0 and 1"));
			return;
		}

		if (m_ALPA <= 0 || m_ALPA >= 1)
		{
			AfxMessageBox(_T("Alpha must be between 0 and 1"));
			return;
		}
	}

	// Проверка параметров градиентов
	if (m_TypeOpt == 0) // Если выбраны градиенты
	{
		if (m_EPS <= 0)
		{
			AfxMessageBox(_T("Epsilon must be positive"));
			return;
		}
	}
	
	// Все проверки пройдены - закрываем диалог
	CDialog::OnOK();
}

BOOL OptimizationVisualizerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Заполняем комбобокс
	m_funcCombo.AddString(_T("Quadratic: f(x,y) = (x-3)^2 + (y+1)^2"));
	m_funcCombo.AddString(_T("Sphere: f(x,y) = x^2 + y^2"));
	m_funcCombo.AddString(_T("Rastrigin: f(x,y) = 20 + x^2 + y^2 - 10(cos(2*pi*x) + cos(2*pi*y))"));

	// Устанавливаем текущий выбор
	if (m_SelectedFunction >= 0 && m_SelectedFunction < 3)
		m_funcCombo.SetCurSel(m_SelectedFunction);
	else
		m_funcCombo.SetCurSel(0);

	UpdateData(FALSE);

	return TRUE;
}
