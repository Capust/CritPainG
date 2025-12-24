#include "pch.h"
#include "CritPainG.h"

#include "CritPainGDoc.h"
#include "OptimizationVisualizerDlg.h"

#include <propkey.h>
#include <sstream>
#include <iomanip>
#include <random>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CCritPainGDoc

IMPLEMENT_DYNCREATE(CCritPainGDoc, CDocument)

BEGIN_MESSAGE_MAP(CCritPainGDoc, CDocument)
	ON_COMMAND(ID_SETTINGS, &CCritPainGDoc::OnSettings)
	ON_COMMAND(ID_START, &CCritPainGDoc::OnOptimizationStart)
END_MESSAGE_MAP()

// CCritPainGDoc construction/destruction
constexpr int CCritPainGDoc::DEFAULT_MAX_ITERATIONS;
constexpr double CCritPainGDoc::DEFAULT_FUNC_CHANGE_EPS;
constexpr double CCritPainGDoc::DEFAULT_POINT_CHANGE_EPS;


CCritPainGDoc::CCritPainGDoc() noexcept
	: m_xMin(-5.0), m_xMax(5.0), m_yMin(-5.0), m_yMax(5.0)
	, m_typeOpt(0)
	, m_delta(0.5), m_p(0.2), m_alpha(0.8), m_epsilon(1e-6)
	, m_selectedFunction(0)
	, m_selectedCriterial(0)  // По умолчанию: 1000 итераций
	, m_finalValue(0.0)
	, m_iterations(0)
	, m_hasFunction(false)
	, m_hasTrajectory(false)
{
	// Начальная точка по умолчанию в центре области
	m_initialPoint = {
		(m_xMin + m_xMax) / 2.0,
		(m_yMin + m_yMax) / 2.0
	};
	m_finalPoint = m_initialPoint;

	// Создаем функцию по умолчанию
	Create2DFunction(m_selectedFunction);

	// Инициализируем траекторию начальной точкой
	m_trajectory.push_back(m_initialPoint);
	m_hasTrajectory = true;

	// Вычисляем начальное значение функции
	if (m_currentFunc)
	{
		m_finalValue = (*m_currentFunc)(m_initialPoint);
	}
}

CCritPainGDoc::~CCritPainGDoc()
{
}

BOOL CCritPainGDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// Инициализация нового документа
	m_trajectory.clear();
	m_initialPoint = {
		(m_xMin + m_xMax) / 2.0,
		(m_yMin + m_yMax) / 2.0
	};
	m_finalPoint = m_initialPoint;
	m_finalValue = 0.0;
	m_iterations = 0;
	m_stopReason = "";
	m_hasTrajectory = false;

	// Создаем функцию
	Create2DFunction(m_selectedFunction);

	// Инициализируем траекторию начальной точкой
	m_trajectory.push_back(m_initialPoint);
	m_hasTrajectory = true;

	if (m_currentFunc)
	{
		m_finalValue = (*m_currentFunc)(m_initialPoint);
	}

	return TRUE;
}

// CCritPainGDoc serialization

void CCritPainGDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// Сохраняем параметры
		ar << m_xMin << m_xMax << m_yMin << m_yMax;
		ar << m_typeOpt;
		ar << m_delta << m_p << m_alpha << m_epsilon;
		ar << m_selectedCriterial << m_selectedFunction;

		// Сохраняем начальную точку
		ar << static_cast<int>(m_initialPoint.size());
		for (const auto& val : m_initialPoint)
			ar << val;

		// Сохраняем траекторию
		ar << static_cast<int>(m_trajectory.size());
		for (const auto& point : m_trajectory)
		{
			ar << static_cast<int>(point.size());
			for (const auto& val : point)
				ar << val;
		}
	}
	else
	{
		// Загружаем параметры
		ar >> m_xMin >> m_xMax >> m_yMin >> m_yMax;
		ar >> m_typeOpt;
		ar >> m_delta >> m_p >> m_alpha >> m_epsilon;
		ar >> m_selectedCriterial >> m_selectedFunction;

		// Загружаем начальную точку
		int size;
		ar >> size;
		m_initialPoint.resize(size);
		for (auto& val : m_initialPoint)
			ar >> val;

		// Загружаем траекторию
		int trajSize;
		ar >> trajSize;
		m_trajectory.clear();
		for (int i = 0; i < trajSize; ++i)
		{
			int pointSize;
			ar >> pointSize;
			std::vector<double> point(pointSize);
			for (auto& val : point)
				ar >> val;
			m_trajectory.push_back(point);
		}

		m_hasTrajectory = !m_trajectory.empty();

		// Обновляем функцию
		Create2DFunction(m_selectedFunction);

		// Если есть траектория, устанавливаем конечную точку
		if (m_hasTrajectory)
		{
			m_finalPoint = m_trajectory.back();
			if (m_currentFunc)
				m_finalValue = (*m_currentFunc)(m_finalPoint);
		}
	}
}

void CCritPainGDoc::Create2DFunction(int funcIndex)
{
	m_selectedFunction = funcIndex;
	m_currentFunc.reset();

	switch (funcIndex)
	{
	case 0: // Quadratic 2D
		m_currentFunc = std::make_unique<QuadraticFunc2D>();
		break;
	case 1: // Sphere 2D - нужно создать адаптер для 2D
		// Временно используем сферу с двумя измерениями
		// Создаем простую сферу для 2D
		class SphereFunc2D : public AbstrFunc {
		public:
			double operator()(const std::vector<double>& x) const override {
				if (x.size() != 2) throw std::invalid_argument("Need 2D point");
				return x[0] * x[0] + x[1] * x[1];
			}
			std::vector<double> getGradient(const std::vector<double>& x) const override {
				if (x.size() != 2) throw std::invalid_argument("Need 2D point");
				return { 2 * x[0], 2 * x[1] };
			}
			std::string getName() const override { return "Sphere 2D: f(x,y)=x^2+y^2"; }
			int getDimension() const override { return 2; }
		};
		m_currentFunc = std::make_unique<SphereFunc2D>();
		break;
	case 2: // Rastrigin 2D - нужно создать адаптер для 2D
		// Временно используем Rastrigin для 2D
		class RastriginFunc2D : public AbstrFunc {
		public:
			double operator()(const std::vector<double>& x) const override {
				if (x.size() != 2) throw std::invalid_argument("Need 2D point");
				const double A = 10.0;
				const double PI = 3.14159265358979323846;
				return 2 * A + x[0] * x[0] - A * cos(2 * PI * x[0])
					+ x[1] * x[1] - A * cos(2 * PI * x[1]);
			}
			std::vector<double> getGradient(const std::vector<double>& x) const override {
				if (x.size() != 2) throw std::invalid_argument("Need 2D point");
				const double A = 10.0;
				const double PI = 3.14159265358979323846;
				return {
					2 * x[0] + 2 * PI * A * sin(2 * PI * x[0]),
					2 * x[1] + 2 * PI * A * sin(2 * PI * x[1])
				};
			}
			std::string getName() const override {
				return "Rastrigin 2D: f(x,y)=20+x^2+y^2-10(cos(2*pi*x)+cos(2*pi*y))";
			}
			int getDimension() const override { return 2; }
		};
		m_currentFunc = std::make_unique<RastriginFunc2D>();
		break;
	default:
		m_currentFunc = std::make_unique<QuadraticFunc2D>();
		break;
	}

	m_hasFunction = (m_currentFunc != nullptr);
}

void CCritPainGDoc::SetupOptimizer()
{
	if (!m_currentFunc || m_initialPoint.empty())
		return;

	// Создаем критерий остановки в зависимости от выбора
	std::unique_ptr<AbstrCriterial> criterial;

	switch (m_selectedCriterial)
	{
	case 0: // 1000 итераций (но используем DEFAULT_MAX_ITERATIONS = 10000)
		criterial = std::make_unique<CriterialMaxIter>(nullptr, 1000);
		break;

	case 1: // Изменение функции < 1e-6
		criterial = std::make_unique<CriterialFunctionChange>(nullptr, DEFAULT_FUNC_CHANGE_EPS);
		break;

	case 2: // Изменение точки < 1e-6
		criterial = std::make_unique<CriterialPointChange>(nullptr, DEFAULT_POINT_CHANGE_EPS);
		break;

	default:
		// По умолчанию: 10000 итераций
		criterial = std::make_unique<CriterialMaxIter>(nullptr, DEFAULT_MAX_ITERATIONS);
		break;
	}

	// Создаем границы
	std::vector<double> lower_bounds = { m_xMin, m_yMin };
	std::vector<double> upper_bounds = { m_xMax, m_yMax };

	// Создаем оптимизатор
	if (m_typeOpt == 0) // Градиенты с ограничениями
	{
		m_optimizer = std::make_unique<ConjugateGradientFRConstrained>(
			m_currentFunc.get(),
			std::move(criterial),
			m_initialPoint,
			lower_bounds,
			upper_bounds,
			1e-6,    // line_search_tolerance
			100,     // max_line_search_iter
			m_epsilon // grad_epsilon
			);
	}
	else if (m_typeOpt == 1) // Случайный поиск
	{
		m_optimizer = std::make_unique<RandomSearchOptim>(
			m_currentFunc.get(),
			std::move(criterial),
			m_initialPoint,
			lower_bounds,
			upper_bounds,
			m_delta,
			std::random_device{}(),
			m_p,
			m_alpha
			);
	}
}

void CCritPainGDoc::SetOptimizationParams(double x1, double x2, double y1, double y2,
	int typeOpt, double delta, double p, double alpha,
	double eps, int criterialType, int funcIndex)  // Изменен параметр!
{
	// Устанавливаем границы области
	m_xMin = (std::min)(x1, x2);
	m_xMax = (std::max)(x1, x2);
	m_yMin = (std::min)(y1, y2);
	m_yMax = (std::max)(y1, y2);

	// Устанавливаем параметры метода
	m_typeOpt = typeOpt;
	m_delta = delta;
	m_p = p;
	m_alpha = alpha;
	m_epsilon = eps;

	// Устанавливаем тип критерия
	m_selectedCriterial = criterialType;

	// Устанавливаем выбранную функцию
	m_selectedFunction = funcIndex;  
	Create2DFunction(funcIndex);     

	// Сбрасываем траекторию
	m_trajectory.clear();
	m_trajectory.push_back(m_initialPoint);
	m_hasTrajectory = true;

	// Обновляем конечную точку и значение
	m_finalPoint = m_initialPoint;
	if (m_currentFunc)
	{
		m_finalValue = (*m_currentFunc)(m_initialPoint);
	}

	// Обновляем все представления
	UpdateAllViews(NULL);
}


void CCritPainGDoc::GetOptimizationParams(double& x1, double& x2, double& y1, double& y2,
	int& typeOpt, double& delta, double& p, double& alpha,
	double& eps, int& criterialType, int& funcIndex) const  // Изменен параметр!
{
	x1 = m_xMin;
	x2 = m_xMax;
	y1 = m_yMin;
	y2 = m_yMax;
	typeOpt = m_typeOpt;
	delta = m_delta;
	p = m_p;
	alpha = m_alpha;
	eps = m_epsilon;
	criterialType = m_selectedCriterial;  // Тип критерия
	funcIndex = m_selectedFunction;
}

void CCritPainGDoc::OnSettings()
{
	OptimizationVisualizerDlg dlg;

	// Минимальная инициализация
	// 1. Границы области
	dlg.m_X1 = GetXMin();
	dlg.m_X2 = GetXMax();
	dlg.m_Y1 = GetYMin();
	dlg.m_Y2 = GetYMax();

	dlg.m_TypeOpt = GetTypeOpt();
	dlg.m_CriterialType = GetSelectedCriterial();
	dlg.m_SelectedFunction = GetSelectedFunction();

	// Пропускаем сложные параметры для теста
	dlg.m_DELTA = GetDelta();
	dlg.m_P = GetP();
	dlg.m_ALPA = GetAlpha();
	dlg.m_EPS = GetEpsilon();

	// Просто показываем диалог без логики
	if (dlg.DoModal() == IDOK)
	{
		SetOptimizationParams(
			dlg.m_X1,    // x1
			dlg.m_X2,    // x2
			dlg.m_Y1,    // y1
			dlg.m_Y2,    // y2
			dlg.m_TypeOpt, // typeOpt
			dlg.m_DELTA, // delta
			dlg.m_P,     // p
			dlg.m_ALPA,  // alpha
			dlg.m_EPS,   // epsilon
			dlg.m_CriterialType, // criterialType 
			dlg.m_SelectedFunction // funcIndex
		);

		UpdateAllViews(NULL);
	}
}


void CCritPainGDoc::SetInitialPoint(double x, double y)
{
	// Проверяем, что точка внутри области
	if (x >= m_xMin && x <= m_xMax && y >= m_yMin && y <= m_yMax)
	{
		m_initialPoint = { x, y };
		m_finalPoint = m_initialPoint;

		// Сбрасываем траекторию
		m_trajectory.clear();
		m_trajectory.push_back(m_initialPoint);
		m_hasTrajectory = true;

		// Вычисляем значение функции
		if (m_currentFunc)
		{
			m_finalValue = (*m_currentFunc)(m_initialPoint);
		}

		// Помечаем как измененный
		SetModifiedFlag(TRUE);

		// Обновляем представления
		UpdateAllViews(NULL);
	}
}

bool CCritPainGDoc::StartOptimization()
{
	if (!m_currentFunc || m_initialPoint.empty())
	{
		AfxMessageBox(_T("Нет функции или начальной точки!"));
		return false;
	}

	// Создаем оптимизатор с текущими параметрами
	SetupOptimizer();

	if (!m_optimizer)
	{
		AfxMessageBox(_T("Не удалось создать оптимизатор!"));
		return false;
	}

	try
	{
		// Запускаем оптимизацию
		auto result = m_optimizer->optimize();

		// Отладочный вывод
		TRACE(_T("Optimization completed:\n"));
		TRACE(_T("  Iterations: %d\n"), result.iterations);
		TRACE(_T("  Trajectory points: %d\n"), (int)result.trajectory.size());
		TRACE(_T("  Stop reason: %s\n"), CString(result.stop_reason.c_str()));

		// Сохраняем результаты
		m_finalPoint = result.point;
		m_finalValue = result.value;
		m_iterations = result.iterations;
		m_stopReason = result.stop_reason;
		m_trajectory = result.trajectory;
		m_hasTrajectory = !m_trajectory.empty();

		// Если траектория пуста или слишком мала, добавляем начальную точку
		if (m_trajectory.empty())
		{
			m_trajectory.push_back(m_initialPoint);
		}

		if (m_trajectory.size() < 2)
		{
			m_trajectory.push_back(m_finalPoint);
		}

		// Помечаем документ как измененный
		SetModifiedFlag(TRUE);

		return true;
	}
	catch (const std::exception& e)
	{
		m_stopReason = std::string("Ошибка: ") + e.what();
		AfxMessageBox(CString("Ошибка оптимизации: ") + e.what());
		return false;
	}
	catch (...)
	{
		m_stopReason = "Неизвестная ошибка";
		AfxMessageBox(_T("Неизвестная ошибка при оптимизации!"));
		return false;
	}
}
double CCritPainGDoc::CalculateFunctionValue(double x, double y) const
{
	if (!m_currentFunc)
		return 0.0;

	return (*m_currentFunc)({ x, y });
}

// Обработчики команд
void CCritPainGDoc::OnOptimizationStart()
{
	if (StartOptimization())
	{
		UpdateAllViews(NULL);

		
		CString message;
		message.Format(_T("Оптимизация завершена!\n\nИтераций: %d\nФинальная точка: (%.4f, %.4f)\nЗначение функции: %.6f\nПричина остановки: %s"),
			m_iterations,
			m_finalPoint[0],
			m_finalPoint[1],
			m_finalValue,
			CString(m_stopReason.c_str()));

		AfxMessageBox(message, MB_OK | MB_ICONINFORMATION);
	}
}
#ifdef _DEBUG
void CCritPainGDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CCritPainGDoc::Dump(CDumpContext& dc) const
{
	dc << "Optimization Document\n";
	dc << "Bounds: [" << m_xMin << ", " << m_xMax << "] x [" << m_yMin << ", " << m_yMax << "]\n";
	dc << "Method: " << (m_typeOpt == 0 ? "Constrained Gradients" : "Random Search") << "\n";
	dc << "Initial point: (" << m_initialPoint[0] << ", " << m_initialPoint[1] << ")\n";
	dc << "Final point: (" << m_finalPoint[0] << ", " << m_finalPoint[1] << ")\n";
	dc << "Final value: " << m_finalValue << "\n";
	dc << "Iterations: " << m_iterations << "\n";
	dc << "Trajectory points: " << m_trajectory.size() << "\n";

	CDocument::Dump(dc);
}
#endif // _DEBUG