#ifndef ABSTROPTIM_H
#define ABSTROPTIM_H

#include "AbstrFunc.h"
#include "AbstrCriterial.h"
#include <vector>
#include <memory>
#include <random>
#include <deque>

class AbstrOptim {
public:
    struct Result {
        std::vector<double> point;
        double value;
        int iterations;
        std::string stop_reason;
        std::deque<std::vector<double>> trajectory;  

        Result() : value(0.0), iterations(0), stop_reason("") {}
        Result(const std::vector<double>& p, double v, int iter,
            const std::string& reason, const std::deque<std::vector<double>>& traj)
            : point(p), value(v), iterations(iter), stop_reason(reason), trajectory(traj) {}
    };
protected:
    const AbstrFunc* func;
    std::unique_ptr<const AbstrCriterial> criterial;
    std::vector<double> initialPoint;
    std::deque<std::vector<double>> trajectory;  

public:
    AbstrOptim(const AbstrFunc* f, std::unique_ptr<const AbstrCriterial> c,
        const std::vector<double>& x0);
    virtual ~AbstrOptim() = default;
    virtual Result optimize() = 0;

    const AbstrCriterial* getCriterial() const { return criterial.get(); }
    const AbstrFunc* getFunc() const { return func; }
    const std::vector<double>& getInitialPoint() const { return initialPoint; }
    const std::deque<std::vector<double>>& getTrajectory() const { return trajectory; }  

    void clearTrajectory() { trajectory.clear(); }  
    void addPointToTrajectory(const std::vector<double>& point) { trajectory.push_back(point); }    
};

class RandomSearchOptim : public AbstrOptim {
private:
    std::vector<double> lower_bounds;
    std::vector<double> upper_bounds;
    double delta;
    mutable std::mt19937 gen;
    double p;
    double alpha;
public:
    RandomSearchOptim(const AbstrFunc* f, std::unique_ptr<const AbstrCriterial> c,
        const std::vector<double>& x0, const std::vector<double>& lb,
        const std::vector<double>& ub, double d, unsigned int seed = std::random_device{}(), double p_value = 0.2, double alpha_value = 0.8);
    Result optimize() override;
};


class ConjugateGradientFR : public AbstrOptim {
private:
    double line_search_tolerance;
    int max_line_search_iter;
    double grad_epsilon;
    double line_search(const std::vector<double>& x, const std::vector<double>& p) const;

public:
    ConjugateGradientFR(const AbstrFunc* f, std::unique_ptr<const AbstrCriterial> c,
        const std::vector<double>& x0, double ls_tolerance = 1e-6,
        int max_ls_iter = 100, double grad_eps = 1e-8);
    Result optimize() override;
};

class ConjugateGradientFRConstrained : public AbstrOptim {
private:
    std::vector<double> lower_bounds;
    std::vector<double> upper_bounds;
    double line_search_tolerance; // Точность линейного поиска (1e-6)
    int max_line_search_iter;
    double grad_epsilon; // Порог нормы градиента (1e-8)

    double line_search(const std::vector<double>& x, const std::vector<double>& p) const;
    std::vector<double> projectToBounds(const std::vector<double>& x) const;

public:
    ConjugateGradientFRConstrained(const AbstrFunc* f, std::unique_ptr<const AbstrCriterial> c,
        const std::vector<double>& x0, const std::vector<double>& lb,
        const std::vector<double>& ub, double ls_tolerance = 1e-6,
        int max_ls_iter = 100, double grad_eps = 1e-8);
    Result optimize() override;
};
#endif
