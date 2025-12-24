#pragma once
#include "afxdialogex.h"

class OptimizationVisualizerDlg : public CDialog
{
	DECLARE_DYNAMIC(OptimizationVisualizerDlg)

public:
	OptimizationVisualizerDlg(CWnd* pParent = nullptr);
	virtual ~OptimizationVisualizerDlg();

	// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG1 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX) override;
	virtual BOOL OnInitDialog() override;

	DECLARE_MESSAGE_MAP()

public:
	// Dialog control variables
	double m_X1, m_X2, m_Y1, m_Y2;
	int m_TypeOpt;
	double m_DELTA, m_P, m_ALPA, m_EPS;
	int m_CriterialType;
	int m_SelectedFunction;

	CComboBox m_funcCombo;

	// Обработчик OK для валидации
	afx_msg void OnBnClickedOk();
};