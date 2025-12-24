#include "pch.h"
#include "AbstrOptim.h"
#include <algorithm>
#include <stdexcept>
#include <limits>
#include <cmath>

#ifndef MaxI
#define MaxI 100000
#endif

AbstrOptim::AbstrOptim(const AbstrFunc* f, std::unique_ptr<const AbstrCriterial> c,
    const std::vector<double>& x0)
    : func(f), criterial(std::move(c)), initialPoint(x0) {
    if (!x0.empty()) {
        trajectory.push_back(x0);
    }
}

RandomSearchOptim::RandomSearchOptim(const AbstrFunc* f,
    std::unique_ptr<const AbstrCriterial> c,
    const std::vector<double>& x0,
    const std::vector<double>& lb,
    const std::vector<double>& ub,
    double d,
    unsigned int seed, double p_value, double alpha_value)
    : AbstrOptim(f, std::move(c), x0), lower_bounds(lb), upper_bounds(ub), delta(d), gen(seed), p(p_value), alpha(alpha_value) {

    if (lb.size() != ub.size() || lb.size() != x0.size()) {
        throw std::invalid_argument("Sizes of bounds and initial point must match.");
    }
    for (size_t i = 0; i < lb.size(); ++i) {
        if (lb[i] > ub[i]) {
            throw std::invalid_argument("Lower bound must be <= upper bound.");
        }

        if (x0[i] < lb[i] || x0[i] > ub[i]) {
            throw std::invalid_argument("Initial point must be inside the box D.");
        }
    }

    if (delta <= 0) {
        throw std::invalid_argument("Delta must be positive.");
    }

    if (p_value <= 0.0 || p_value >= 1.0) {
        throw std::invalid_argument("P must be in range (0, 1).");
    }

    if (alpha_value <= 0.0 || alpha_value >= 1.0) {
        throw std::invalid_argument("Alpha must be in range (0, 1).");
    }
}

AbstrOptim::Result RandomSearchOptim::optimize() {
    trajectory.clear();

    std::vector<double> current_point = initialPoint;
    double current_value = (*func)(current_point);
    int iteration = 0;
    addPointToTrajectory(current_point);

    std::vector<double> best_point = current_point;
    double best_value = current_value;

    const int max_fallback_iterations = MaxI;
    double current_delta = delta;

    // Распределения
    std::uniform_real_distribution<double> prob_dis(0.0, 1.0);
    std::uniform_real_distribution<double> global_dis(0.0, 1.0);
    const int max_no_improvement = 50;
    int no_improvement_count = 0;   

    while (!criterial->isSatisfied(current_point, current_value, iteration)) {
        if (iteration >= max_fallback_iterations) {
            return { best_point, best_value, iteration,
                     "Fallback: reached maximum iterations", trajectory };
        }

        std::vector<double> candidate_point(current_point.size());
        bool is_local_search = (prob_dis(gen) < p);
        bool improvement = false;

        if (is_local_search) {
            std::uniform_real_distribution<double> coord_dis;

            for (size_t i = 0; i < current_point.size(); ++i) {
                coord_dis.param(std::uniform_real_distribution<double>::param_type(
                    -current_delta, current_delta
                ));

                double offset = coord_dis(gen);
                candidate_point[i] = current_point[i] + offset;

                // Ограничение границами D
                candidate_point[i] = (std::max)(lower_bounds[i], (std::min)(upper_bounds[i], candidate_point[i]));
            }
        }
        else {
            // Глобальный поиск
            for (size_t i = 0; i < current_point.size(); ++i) {
                candidate_point[i] = lower_bounds[i] +
                    global_dis(gen) * (upper_bounds[i] - lower_bounds[i]);
            }
        }

        double candidate_value = (*func)(candidate_point);

        if (candidate_value < current_value) {
            current_point = candidate_point;
            current_value = candidate_value;
            improvement = true;

            no_improvement_count = 0; 
            addPointToTrajectory(current_point);
            // Уменьшаем delta ТОЛЬКО при улучшении из локального поиска
            if (is_local_search) {
                current_delta *= alpha;
            }

            if (candidate_value < best_value) {
                best_point = candidate_point;
                best_value = candidate_value;
            }
        }
        else {
            // Не было улучшения
            no_improvement_count++;

            // Увеличиваем delta при долгом отсутствии улучшений
            if (no_improvement_count >= max_no_improvement) {
                current_delta /= alpha;

                // Ограничиваем максимальный delta
                double max_possible_delta = 0.0;
                for (size_t i = 0; i < upper_bounds.size(); ++i) {
                    max_possible_delta = (std::max)(max_possible_delta,
                        upper_bounds[i] - lower_bounds[i]);
                }

                if (current_delta > max_possible_delta) {
                    current_delta = max_possible_delta;
                }

                no_improvement_count = 0;  // Сброс после увеличения delta
            }
        }

        iteration++;
    }

    return { best_point, best_value, iteration, "Criterial satisfied", trajectory };
}

double ConjugateGradientFR::line_search(const std::vector<double>& x, const std::vector<double>& p) const {
    const double initial_alpha = 1.0;
    const double reduction_factor = 0.5;
    const int max_tries = 20;

    double alpha = initial_alpha;
    double f_current = (*func)(x);

    std::vector<double> new_point(x.size());

    for (int try_count = 0; try_count < max_tries; ++try_count) {
        // Вычисляем новую точку
        for (size_t i = 0; i < x.size(); ++i) {
            new_point[i] = x[i] + alpha * p[i];
        }

        double f_new = (*func)(new_point);

        // Проверяем условие уменьшения функции
        if (f_new < f_current) {
            return alpha;
        }

        // Уменьшаем шаг
        alpha *= reduction_factor;
    }

    return alpha; // Возвращаем последний alpha, даже если не нашли улучшения
}

ConjugateGradientFR::ConjugateGradientFR(const AbstrFunc* f,
    std::unique_ptr<const AbstrCriterial> c,
    const std::vector<double>& x0,
    double ls_tolerance,
    int max_ls_iter,
    double grad_eps)
    : AbstrOptim(f, std::move(c), x0), line_search_tolerance(ls_tolerance),
    max_line_search_iter(max_ls_iter), grad_epsilon(grad_eps) {}

AbstrOptim::Result ConjugateGradientFR::optimize() {
    trajectory.clear();
    std::vector<double> x = initialPoint;
    double f_val = (*func)(x);
    int iteration = 0;
    addPointToTrajectory(x);
    // Вычисляем начальный градиент
    std::vector<double> grad;
    try {
        grad = func->getGradient(x);
    }
    catch (...) {
        // Если аналитический градиент не работает, используем численный
        grad = numericalGradient(*func, x);
    }

    std::vector<double> p = grad;
    for (double& val : p) val = -val; 

    std::vector<double> best_x = x;
    double best_f_val = f_val;

    const int max_fallback_iterations = MaxI;

    while (!criterial->isSatisfied(x, f_val, iteration)) {
       
        if (iteration >= max_fallback_iterations) {
            return { best_x, best_f_val, iteration,
                     "Fallback: reached maximum iterations", trajectory };
        }


        // Проверяем норму градиента
        double grad_norm_sq = 0.0;
        for (double g : grad) grad_norm_sq += g * g;
        if (std::sqrt(grad_norm_sq) < grad_epsilon) {
            return { best_x, best_f_val, iteration,
                     "Gradient norm below threshold", trajectory };
        }

        // Линейный поиск
        double alpha = line_search(x, p);

        // Обновляем точку
        std::vector<double> x_new = x;
        for (size_t i = 0; i < x.size(); ++i) {
            x_new[i] += alpha * p[i];
        }

        double f_val_new = (*func)(x_new);
        addPointToTrajectory(x_new);
        // Обновляем лучшую точку
        if (f_val_new < best_f_val) {
            best_x = x_new;
            best_f_val = f_val_new;
        }

        // Вычисляем новый градиент
        std::vector<double> grad_new;
        try {
            grad_new = func->getGradient(x_new);
        }
        catch (...) {
            grad_new = numericalGradient(*func, x_new);
        }

        // Проверяем на NaN в градиенте
        bool grad_has_nan = false;
        for (double g : grad_new) {
            if (std::isnan(g)) {
                grad_has_nan = true;
                break;
            }
        }

        if (grad_has_nan) {
            return { best_x, best_f_val, iteration,
                     "Gradient contains NaN", trajectory };
        }

        // Вычисляем beta по Fletcher-Reeves
        double grad_norm_sq_old = 0.0, grad_norm_sq_new = 0.0;
        for (size_t i = 0; i < grad.size(); ++i) {
            grad_norm_sq_old += grad[i] * grad[i];
            grad_norm_sq_new += grad_new[i] * grad_new[i];
        }

        double beta = 0.0;
        if (grad_norm_sq_old > 0) {
            beta = grad_norm_sq_new / grad_norm_sq_old;
        }

        // Обновляем направление
        for (size_t i = 0; i < p.size(); ++i) {
            p[i] = -grad_new[i] + beta * p[i];
        }

        // Обновляем переменные для следующей итерации
        x = x_new;
        f_val = f_val_new;
        grad = grad_new;
        iteration++;
    }

    return { best_x, best_f_val, iteration, "Criterial satisfied", trajectory };
}


ConjugateGradientFRConstrained::ConjugateGradientFRConstrained(const AbstrFunc* f,
    std::unique_ptr<const AbstrCriterial> c,
    const std::vector<double>& x0, const std::vector<double>& lb,
    const std::vector<double>& ub, double ls_tolerance,
    int max_ls_iter, double grad_eps)
    : AbstrOptim(f, std::move(c), x0), lower_bounds(lb), upper_bounds(ub),
    line_search_tolerance(ls_tolerance), max_line_search_iter(max_ls_iter),
    grad_epsilon(grad_eps) {

    if (lb.size() != ub.size() || lb.size() != x0.size()) {
        throw std::invalid_argument("Sizes of bounds and initial point must match.");
    }

    for (size_t i = 0; i < lb.size(); ++i) {
        if (lb[i] > ub[i]) {
            throw std::invalid_argument("Lower bound must be <= upper bound.");
        }
    }
}

std::vector<double> ConjugateGradientFRConstrained::projectToBounds(const std::vector<double>& x) const {
    std::vector<double> projected = x;
    for (size_t i = 0; i < x.size(); ++i) {
        projected[i] = (std::max)(lower_bounds[i], (std::min)(upper_bounds[i], x[i]));
    }
    return projected;
}

double ConjugateGradientFRConstrained::line_search(const std::vector<double>& x, const std::vector<double>& p) const {
    const double initial_alpha = 1.0;
    const double reduction_factor = 0.5;
    const int max_tries = 20;

    double alpha = initial_alpha;
    double f_current = (*func)(x);

    for (int try_count = 0; try_count < max_tries; ++try_count) {
        // Вычисляем новую точку и проецируем на границы
        std::vector<double> new_point = x;
        for (size_t i = 0; i < x.size(); ++i) {
            new_point[i] += alpha * p[i];
        }
        new_point = projectToBounds(new_point);

        double f_new = (*func)(new_point);

        // Проверяем условие уменьшения функции
        if (f_new < f_current) {
            return alpha;
        }

        // Уменьшаем шаг
        alpha *= reduction_factor;
    }

    return alpha;
}

AbstrOptim::Result ConjugateGradientFRConstrained::optimize() {
    trajectory.clear();

    // Начальная точка должна быть в границах
    std::vector<double> x = projectToBounds(initialPoint);
    double f_val = (*func)(x);
    int iteration = 0;
    addPointToTrajectory(x);
    
    // Всегда используем численный градиент
    std::vector<double> grad = numericalGradient(*func, x);
    std::vector<double> p = grad;
    for (double& val : p) val = -val;

    std::vector<double> best_x = x;
    double best_f_val = f_val;

    const int max_fallback_iterations = MaxI;

    while (!criterial->isSatisfied(x, f_val, iteration)) {
        // если слишком много итераций
        if (iteration >= max_fallback_iterations) {
            addPointToTrajectory(best_x);

            return { best_x, best_f_val, iteration,
                     "Reached maximum iterations", trajectory };
        }

        // Проверяем норму градиента
        double grad_norm_sq = 0.0;
        for (double g : grad) grad_norm_sq += g * g;
        if (std::sqrt(grad_norm_sq) < grad_epsilon) {
            addPointToTrajectory(best_x);

            return { best_x, best_f_val, iteration,
                         "Gradient norm below threshold", trajectory };
            }

        // Линейный поиск
        double alpha = line_search(x, p);

        // Обновляем точку и ПРОЕЦИРУЕМ на границы
        std::vector<double> x_new = x;
        for (size_t i = 0; i < x.size(); ++i) {
            x_new[i] += alpha * p[i];
        }
        x_new = projectToBounds(x_new); // Проекция на допустимую область

        double f_val_new = (*func)(x_new);

        // Обновляем лучшую точку
        if (f_val_new < best_f_val) {
            best_x = x_new;
            best_f_val = f_val_new;
        }

        // Вычисляем новый градиент
        std::vector<double> grad_new = numericalGradient(*func, x_new);

        // Проверяем на NaN в градиенте
        bool grad_has_nan = false;
        for (double g : grad_new) {
            if (std::isnan(g)) {
                grad_has_nan = true;
                break;
            }
        }

        if (grad_has_nan) {
            addPointToTrajectory(best_x);

            return { best_x, best_f_val, iteration,
                     "Gradient contains NaN", trajectory };
        }

        // Fletcher-Reeves beta
        double grad_norm_sq_old = 0.0, grad_norm_sq_new = 0.0;
        for (size_t i = 0; i < grad.size(); ++i) {
            grad_norm_sq_old += grad[i] * grad[i];
            grad_norm_sq_new += grad_new[i] * grad_new[i];
        }

        double beta = 0.0;
        if (grad_norm_sq_old > 0) {
            beta = grad_norm_sq_new / grad_norm_sq_old;
        }

        // Обновляем направление
        for (size_t i = 0; i < p.size(); ++i) {
            p[i] = -grad_new[i] + beta * p[i];
        }
        x = x_new;
        f_val = f_val_new;
        grad = grad_new;
        iteration++;
    }
    addPointToTrajectory(best_x);

    return { best_x, best_f_val, iteration, "Criterial satisfied", trajectory };
}