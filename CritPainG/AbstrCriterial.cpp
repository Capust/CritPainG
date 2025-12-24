#include "pch.h"
#include "AbstrCriterial.h"
#include "AbstrOptim.h"
#include "AbstrFunc.h"
#include <cmath>
#include <iostream>
#include <algorithm>

AbstrCriterial::AbstrCriterial(const AbstrOptim* opt) : optimizer(opt) {}

CriterialMaxIter::CriterialMaxIter(const AbstrOptim* opt, int max_iter)
    : AbstrCriterial(opt), max_iterations(max_iter) {}

bool CriterialMaxIter::isSatisfied(const std::vector<double>&, double, int current_iteration) const {
    return current_iteration >= max_iterations;
}

std::string CriterialMaxIter::getName() const {
    return "Max Iterations: " + std::to_string(max_iterations);
}

std::unique_ptr<AbstrCriterial> CriterialMaxIter::clone() const {
    return std::make_unique<CriterialMaxIter>(optimizer, max_iterations);
}

CriterialLastImprovement::CriterialLastImprovement(const AbstrOptim* opt, int max_iter_no_imp)
    : AbstrCriterial(opt), max_iterations_without_improvement(max_iter_no_imp),
    last_improvement_iteration(0), best_value_so_far(0.0), first_call(true) {}

bool CriterialLastImprovement::isSatisfied(const std::vector<double>&, double current_value, int current_iteration) const {
    if (first_call) {
        best_value_so_far = current_value;
        last_improvement_iteration = current_iteration;
        first_call = false;
        return false;
    }

    if (current_value < best_value_so_far) {
        best_value_so_far = current_value;
        last_improvement_iteration = current_iteration;
    }

    return (current_iteration - last_improvement_iteration) >= max_iterations_without_improvement;
}

std::string CriterialLastImprovement::getName() const {
    return "No Improvement for: " + std::to_string(max_iterations_without_improvement) + " iterations";
}

std::unique_ptr<AbstrCriterial> CriterialLastImprovement::clone() const {
    return std::make_unique<CriterialLastImprovement>(optimizer, max_iterations_without_improvement);
}

CriterialFunctionChange::CriterialFunctionChange(const AbstrOptim* opt, double eps)
    : AbstrCriterial(opt), epsilon(eps),
    previous_value(0.0),
    previous_point(),
    first_call(true) {}

bool CriterialFunctionChange::isSatisfied(const std::vector<double>& current_point,
    double current_value,
    int current_iteration) const {
    if (first_call) {
        previous_value = current_value;
        previous_point = current_point;
        first_call = false;
        return false;
    }

    // Проверяем, изменилась ли точка (не тривиальное сравнение)
    bool point_changed = false;
    if (previous_point.size() == current_point.size()) {
        for (size_t i = 0; i < current_point.size(); ++i) {
            if (std::abs(current_point[i] - previous_point[i]) > 1e-15) {
                point_changed = true;
                break;
            }
        }
    }
    else {
        point_changed = true;
    }

    // Если точка не изменилась, не проверяем критерий
    if (!point_changed) {
        return false;
    }

    double change = std::abs(current_value - previous_value);
    bool satisfied = change < epsilon;

    previous_value = current_value;
    previous_point = current_point;  // ← ОБНОВИТЬ точку
    return satisfied;
}

std::string CriterialFunctionChange::getName() const {
    return "Function Change < " + std::to_string(epsilon);
}

std::unique_ptr<AbstrCriterial> CriterialFunctionChange::clone() const {
    return std::make_unique<CriterialFunctionChange>(optimizer, epsilon);
}

CriterialGradientNorm::CriterialGradientNorm(const AbstrOptim* opt, double eps)
    : AbstrCriterial(opt), epsilon(eps), first_call(true) {}

bool CriterialGradientNorm::isSatisfied(const std::vector<double>& current_point, double, int) const {
    if (!optimizer || !optimizer->getFunc()) {
        return false;
    }

    // Вычисляем градиент численно
    auto grad = numericalGradient(*(optimizer->getFunc()), current_point);

    double norm_sq = 0.0;
    for (double g : grad) {
        norm_sq += g * g;
    }

    return std::sqrt(norm_sq) < epsilon;
}

std::string CriterialGradientNorm::getName() const {
    return "Gradient Norm < " + std::to_string(epsilon);
}

std::unique_ptr<AbstrCriterial> CriterialGradientNorm::clone() const {
    return std::make_unique<CriterialGradientNorm>(optimizer, epsilon);
}

CriterialPointChange::CriterialPointChange(const AbstrOptim* opt, double eps)
    : AbstrCriterial(opt), epsilon(eps), first_call(true) {}

bool CriterialPointChange::isSatisfied(const std::vector<double>& current_point, double, int) const {
    if (first_call) {
        previous_point = current_point;
        first_call = false;
        return false;
    }

    if (previous_point.size() != current_point.size()) {
        previous_point = current_point;
        return false;
    }

    double max_change = 0.0;
    for (size_t i = 0; i < current_point.size(); ++i) {
        double change = std::abs(current_point[i] - previous_point[i]);
        max_change = (std::max)(max_change, change);
    }

    previous_point = current_point;
    return max_change < epsilon;
}

std::string CriterialPointChange::getName() const {
    return "Point Change < " + std::to_string(epsilon);
}

std::unique_ptr<AbstrCriterial> CriterialPointChange::clone() const {
    return std::make_unique<CriterialPointChange>(optimizer, epsilon);
}