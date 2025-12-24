#ifndef CONSOLEMENU_H
#define CONSOLEMENU_H

#include "AbstrFunc.h"
#include "AbstrCriterial.h"
#include "AbstrOptim.h"
#include <memory>
#include <vector>
#include <random>

struct OptimizationConfig {
    std::unique_ptr<AbstrFunc> function;
    std::vector<double> lower_bounds;
    std::vector<double> upper_bounds;
    std::unique_ptr<AbstrCriterial> criterial;
    std::vector<double> initial_point;
    double delta = 0.5;
    double grad_epsilon = 1e-8;
    double random_search_p = 0.2;
    double random_search_alpha = 0.8;
    int dimension = 2;
    bool use_random_search = true;
    int max_iterations = 1000;
};

class ConsoleMenu {
public:
    void run();

private:
    void showMainMenu();
    void runOptimizationMenu();
    void selectFunction(OptimizationConfig& config);
    void selectDomain(OptimizationConfig& config);
    void selectCriterial(OptimizationConfig& config);
    void selectInitialPoint(OptimizationConfig& config);
    void selectMethod(OptimizationConfig& config);
    void runOptimization(const OptimizationConfig& config);
    void showResults(const AbstrOptim::Result& result, const OptimizationConfig& config);
    void printPoint(const std::vector<double>& point);
    bool isPointInDomain(const std::vector<double>& point,
        const std::vector<double>& lower_bounds,
        const std::vector<double>& upper_bounds) const;
};
#endif