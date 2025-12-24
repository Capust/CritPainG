#pragma once

#include "AbstrFunc.h"
#include "AbstrOptim.h"
#include "AbstrCriterial.h"
#include <deque>
#include <memory>
#include <vector>

class CCritPainGDoc : public CDocument
{
protected:
	CCritPainGDoc() noexcept;
	DECLARE_DYNCREATE(CCritPainGDoc)

public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
#ifdef SHARED_HANDLERS
	virtual void InitializeSearchContent();
	virtual void OnDrawThumbnail(CDC& dc, LPRECT lprcBounds);
#endif

	virtual ~CCritPainGDoc();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	DECLARE_MESSAGE_MAP()

#ifdef SHARED_HANDLERS
		void SetSearchContent(const CString& value);
#endif

private:
	// Данные оптимизации
	std::unique_ptr<AbstrFunc> m_currentFunc;
	std::unique_ptr<AbstrOptim> m_optimizer;
	std::unique_ptr<AbstrCriterial> m_criterial;

	// Траектория и результаты
	std::deque<std::vector<double>> m_trajectory;
	std::vector<double> m_initialPoint;
	std::vector<double> m_finalPoint;
	double m_finalValue;
	int m_iterations;
	static constexpr int DEFAULT_MAX_ITERATIONS = 10000;
	static constexpr double DEFAULT_FUNC_CHANGE_EPS = 1e-6;
	static constexpr double DEFAULT_POINT_CHANGE_EPS = 1e-6;
	std::string m_stopReason;

	// Параметры оптимизации
	double m_xMin, m_xMax, m_yMin, m_yMax;
	int m_typeOpt;           // 0 - градиенты с ограничениями, 1 - случайный поиск
	double m_delta, m_p, m_alpha, m_epsilon;
	int m_selectedCriterial;
	int m_selectedFunction;

	// Состояние
	bool m_hasFunction;
	bool m_hasTrajectory;

	// Вспомогательные методы
	void Create2DFunction(int funcIndex);
	void SetupOptimizer();

public:
	// Методы для работы с данными
	void SetOptimizationParams(double x1, double x2, double y1, double y2,
		int typeOpt, double delta, double p, double alpha,
		double eps, int maxit, int funcIndex);
	virtual BOOL SaveModified() override { return TRUE; }
	int GetSelectedCriterial() const { return m_selectedCriterial; }
	int GetSelectedFunction() const { return m_selectedFunction; }

	int GetDefaultMaxIterations() const { return DEFAULT_MAX_ITERATIONS; }
	void SetSelectedCriterial(int value) { m_selectedCriterial = value; }
	// Получение параметров для передачи в диалог
	void GetOptimizationParams(double& x1, double& x2, double& y1, double& y2,
		int& typeOpt, double& delta, double& p, double& alpha,
		double& eps, int& maxit, int& funcIndex) const;

	void SetInitialPoint(double x, double y);
	bool StartOptimization();

	// Геттеры для View
	const std::deque<std::vector<double>>& GetTrajectory() const { return m_trajectory; }
	const std::vector<double>& GetInitialPoint() const { return m_initialPoint; }
	const std::vector<double>& GetFinalPoint() const { return m_finalPoint; }
	double GetFinalValue() const { return m_finalValue; }
	int GetIterations() const { return m_iterations; }
	std::string GetStopReason() const { return m_stopReason; }
	
	double GetDelta() const { return m_delta; }
	double GetP() const { return m_p;  }
	double GetAlpha() const { return m_alpha; }
	double GetEpsilon() const { return m_epsilon; }

	double CalculateFunctionValueAtInitialPoint() const
	{
		if (m_currentFunc && !m_initialPoint.empty())
			return (*m_currentFunc)(m_initialPoint);
		return 0.0;
	}

	double GetXMin() const { return m_xMin; }	
	double GetXMax() const { return m_xMax; }
	double GetYMin() const { return m_yMin; }
	double GetYMax() const { return m_yMax; }

	int GetTypeOpt() const { return m_typeOpt; }
	bool HasFunction() const { return m_hasFunction; }
	bool HasTrajectory() const { return m_hasTrajectory; }

	double CalculateFunctionValue(double x, double y) const;

	afx_msg void OnSettings();
	afx_msg void OnOptimizationStart();
};