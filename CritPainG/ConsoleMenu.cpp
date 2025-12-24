#include "ConsoleMenu.h"
#include <iostream>
#include <limits>
#include <cmath>
#include <random>

#ifndef MaxI
#define MaxI 100000
#endif

void ConsoleMenu::run() {
    while (true) {
        showMainMenu();
        int choice;
        std::cin >> choice;

        if (choice == 0) break;

        if (choice == 1) {
            runOptimizationMenu();
        }
        else {
            std::cout << "Invalid option. Please try again." << std::endl;
        }
    }
}

void ConsoleMenu::showMainMenu() {
    std::cout << "\n=== Optimization Methods ===" << std::endl;
    std::cout << "1. Run Optimization" << std::endl;
    std::cout << "0. Exit" << std::endl;
    std::cout << "Select option: ";
}

void ConsoleMenu::runOptimizationMenu() {
    OptimizationConfig config;

    std::cout << "\n=== Optimization Configuration ===" << std::endl;

    selectFunction(config);
    selectDomain(config);
    selectCriterial(config);
    selectInitialPoint(config);
    selectMethod(config);

    runOptimization(config);
}

void ConsoleMenu::selectFunction(OptimizationConfig& config) {
    std::cout << "\n=== Select Test Function ===" << std::endl;
    std::cout << "1. Quadratic 2D" << std::endl;
    std::cout << "2. Sphere 3D" << std::endl;
    std::cout << "3. Rastrigin 4D" << std::endl;
    std::cout << "Select function (1-3): ";

    int choice;
    std::cin >> choice;

    switch (choice) {
    case 1: config.function = std::make_unique<QuadraticFunc2D>(); break;
    case 2: config.function = std::make_unique<SphereFunc3D>(); break;
    case 3: config.function = std::make_unique<RastriginFunc4D>(); break;
    default:
        config.function = std::make_unique<QuadraticFunc2D>();
        std::cout << "Invalid selection, using Quadratic 2D." << std::endl;
        break;
    }

    std::cout << "Selected: " << config.function->getName() << std::endl;
    config.dimension = config.function->getDimension();
}

void ConsoleMenu::selectDomain(OptimizationConfig& config) {
    std::cout << "\n=== Select Domain Type ===" << std::endl;
    std::cout << "1. Easy - Small domain [-5, 5]^n" << std::endl;
    std::cout << "2. Easy - Medium domain [-10, 10]^n" << std::endl;
    std::cout << "3. Easy - Large domain [-50, 50]^n" << std::endl;
    std::cout << "4. Hard - Custom bounds" << std::endl;
    std::cout << "Select domain type (1-4): ";

    int choice;
    std::cin >> choice;

    config.lower_bounds.resize(config.dimension);
    config.upper_bounds.resize(config.dimension);

    switch (choice) {
    case 1:
        for (int i = 0; i < config.dimension; ++i) {
            config.lower_bounds[i] = -5.0;
            config.upper_bounds[i] = 5.0;
        }
        std::cout << "Domain: [-5, 5]^" << config.dimension << std::endl;
        break;
    case 2:
        for (int i = 0; i < config.dimension; ++i) {
            config.lower_bounds[i] = -10.0;
            config.upper_bounds[i] = 10.0;
        }
        std::cout << "Domain: [-10, 10]^" << config.dimension << std::endl;
        break;
    case 3:
        for (int i = 0; i < config.dimension; ++i) {
            config.lower_bounds[i] = -50.0;
            config.upper_bounds[i] = 50.0;
        }
        std::cout << "Domain: [-50, 50]^" << config.dimension << std::endl;
        break;
    case 4:
        std::cout << "Enter lower bounds (" << config.dimension << " values): ";
        for (int i = 0; i < config.dimension; ++i) {
            std::cin >> config.lower_bounds[i];
        }
        std::cout << "Enter upper bounds (" << config.dimension << " values): ";
        for (int i = 0; i < config.dimension; ++i) {
            std::cin >> config.upper_bounds[i];
        }
        std::cout << "Custom domain set." << std::endl;
        break;
    default:
        for (int i = 0; i < config.dimension; ++i) {
            config.lower_bounds[i] = -5.0;
            config.upper_bounds[i] = 5.0;
        }
        std::cout << "Invalid selection, using [-5, 5]^" << config.dimension << std::endl;
        break;
    }
}


void ConsoleMenu::selectCriterial(OptimizationConfig& config) {
    std::cout << "\n=== Select Stop Criterial ===" << std::endl;
    std::cout << "1. Max Iterations (100)" << std::endl;
    std::cout << "2. Max Iterations (1000)" << std::endl;
    std::cout << "3. Function Change < 1e-6" << std::endl;
    std::cout << "4. Point Change < 1e-6" << std::endl;

    // Gradient Norm только для Conjugate Gradient
    if (!config.use_random_search) {
        std::cout << "5. Gradient Norm < 1e-6" << std::endl;
    }

    std::cout << "Select criterial (1-" << (config.use_random_search ? "4" : "5") << "): ";

    int choice;
    std::cin >> choice;

    // Для Random Search ограничиваем выбор
    if (config.use_random_search && choice == 5) {
        choice = 2; // По умолчанию Max Iterations (1000)
        std::cout << "Gradient Norm not available for Random Search. Using Max Iterations (1000)." << std::endl;
    }

    switch (choice) {
    case 1:
        config.criterial = std::make_unique<CriterialMaxIter>(nullptr, 100);
        config.max_iterations = 100;
        break;
    case 2:
        config.criterial = std::make_unique<CriterialMaxIter>(nullptr, 1000);
        config.max_iterations = 1000;
        break;
    case 3:
        config.criterial = std::make_unique<CriterialFunctionChange>(nullptr, 1e-6);
        config.max_iterations = MaxI;
        break;
    case 4:
        config.criterial = std::make_unique<CriterialPointChange>(nullptr, 1e-6);
        config.max_iterations = MaxI;
        break;
    case 5:
        if (!config.use_random_search) {
            config.criterial = std::make_unique<CriterialGradientNorm>(nullptr, 1e-6);
            config.max_iterations = MaxI;
        }
        else {
            config.criterial = std::make_unique<CriterialMaxIter>(nullptr, MaxI);
            config.max_iterations = MaxI;
        }
        break;
    default:
        config.criterial = std::make_unique<CriterialMaxIter>(nullptr, MaxI);
        config.max_iterations = MaxI;
        std::cout << "Invalid selection, using Max Iterations (10000)." << std::endl;
        break;
    }

    std::cout << "Selected: " << config.criterial->getName() << std::endl;
}

void ConsoleMenu::selectInitialPoint(OptimizationConfig& config) {
    std::cout << "\n=== Select Initial Point ===" << std::endl;
    std::cout << "1. Use random point in domain" << std::endl;
    std::cout << "2. Use center of domain" << std::endl;
    std::cout << "3. Enter custom point" << std::endl;
    std::cout << "Select option (1-3): ";

    int choice;
    std::cin >> choice;

    config.initial_point.resize(config.dimension);

    switch (choice) {
    case 1:
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        for (int i = 0; i < config.dimension; ++i) {
            std::uniform_real_distribution<double> dist(config.lower_bounds[i], config.upper_bounds[i]);
            config.initial_point[i] = dist(gen);
        }
        std::cout << "Initial point: random ";
        printPoint(config.initial_point);
        std::cout << std::endl;
    }
    break;
    case 2:
        // Используем центр области
        for (int i = 0; i < config.dimension; ++i) {
            config.initial_point[i] = (config.lower_bounds[i] + config.upper_bounds[i]) / 2.0;
        }
        std::cout << "Initial point: center ";
        printPoint(config.initial_point);
        std::cout << std::endl;
        break;
    case 3:
        while (true) {
            std::cout << "Enter initial point (" << config.dimension << " values): ";
            for (int i = 0; i < config.dimension; ++i) {
                std::cin >> config.initial_point[i];
            }

            if (isPointInDomain(config.initial_point, config.lower_bounds, config.upper_bounds)) {
                break;
            }
            else {
                std::cout << "Error: Point is outside the domain! Please enter a point within [";
                printPoint(config.lower_bounds);
                std::cout << "] to [";
                printPoint(config.upper_bounds);
                std::cout << "]" << std::endl;
                std::cout << "Please try again." << std::endl;
            }
        }
        std::cout << "Initial point: custom ";
        printPoint(config.initial_point);
        std::cout << std::endl;
        break;
    default:
        // По умолчанию используем центр области
        for (int i = 0; i < config.dimension; ++i) {
            config.initial_point[i] = (config.lower_bounds[i] + config.upper_bounds[i]) / 2.0;
        }
        std::cout << "Invalid selection, using center of domain." << std::endl;
        break;
    }
}

bool ConsoleMenu::isPointInDomain(const std::vector<double>& point,
    const std::vector<double>& lower_bounds,
    const std::vector<double>& upper_bounds) const {
    if (point.size() != lower_bounds.size() || point.size() != upper_bounds.size()) {
        return false;
    }

    for (size_t i = 0; i < point.size(); ++i) {
        if (point[i] < lower_bounds[i] || point[i] > upper_bounds[i]) {
            return false;
        }
    }
    return true;
}

void ConsoleMenu::selectMethod(OptimizationConfig& config) {
    std::cout << "\n=== Select Optimization Method ===" << std::endl;
    std::cout << "1. Random Search" << std::endl;
    std::cout << "2. Conjugate Gradient (Fletcher-Reeves)" << std::endl;
    std::cout << "Select method (1-2): ";

    int choice;
    std::cin >> choice;

    config.use_random_search = (choice == 1);

    if (config.use_random_search) {
        std::cout << "Enter delta for random search (default 0.8): ";
        std::cin >> config.delta;
        if (config.delta <= 0) {
            config.delta = 0.8;
            std::cout << "Invalid delta, using 0.8." << std::endl;
        }

        std::cout << "Enter probability for LOCAL search (p, default 0.2, range 0.0-1.0): ";
        std::cin >> config.random_search_p;

        // Валидация ввода
        if (config.random_search_p < 0.0 || config.random_search_p > 1.0) {
            config.random_search_p = 0.2;
            std::cout << "Invalid probability. Using default value 0.2." << std::endl;
            std::cout << "Note: p=0.0 means only GLOBAL search" << std::endl;
            std::cout << "      p=1.0 means only LOCAL search" << std::endl;
            std::cout << "      p=0.2 means 20% local, 80% global search" << std::endl;
        }

        // Показать объяснение пользователю
        std::cout << "Using p=" << config.random_search_p
            << " (" << (config.random_search_p * 100) << "% local, "
            << ((1.0 - config.random_search_p) * 100) << "% global search)" << std::endl;

        std::cout << "Enter alpha (coefficient for delta adaptation, default 0.8, range 0.1-0.9): ";
        std::cin >> config.random_search_alpha;

        // Валидация alpha
        if (config.random_search_alpha <= 0.0 || config.random_search_alpha >= 1.0) {
            config.random_search_alpha = 0.8;
            std::cout << "Invalid alpha. Using default 0.8." << std::endl;
        }

    }
    else {
        std::cout << "Enter gradient epsilon (default 1e-8): ";
        std::cin >> config.grad_epsilon;
        if (config.grad_epsilon <= 0) {
            config.grad_epsilon = 1e-8;
            std::cout << "Invalid epsilon, using 1e-8." << std::endl;
        }
    }

    std::cout << "Selected: " << (config.use_random_search ? "Random Search" : "Conjugate Gradient") << std::endl;
}

void ConsoleMenu::runOptimization(const OptimizationConfig& config) {
    std::cout << "\n=== Running Optimization ===" << std::endl;
    std::cout << "Function: " << config.function->getName() << std::endl;
    std::cout << "Dimension: " << config.dimension << "D" << std::endl;
    std::cout << "Method: " << (config.use_random_search ? "Random Search" : "Conjugate Gradient") << std::endl;

    if (config.use_random_search) {
        std::cout << "Random Search parameters:" << std::endl;
        std::cout << "  - Delta: " << config.delta << std::endl;
        std::cout << "  - Probability: " << config.random_search_p
            << " (" << (config.random_search_p * 100) << "% local search)" << std::endl;
        std::cout << "  - Alpha: " << config.random_search_alpha
            << " (delta multiplier)" << std::endl;
    }

    std::cout << "Stop criterial: " << config.criterial->getName() << std::endl;
    std::cout << "Initial point: ";
    printPoint(config.initial_point);
    std::cout << std::endl;
    std::cout << "Initial value: " << (*config.function)(config.initial_point) << std::endl;
    std::cout << "Optimizing..." << std::endl;

    try {
        AbstrOptim::Result result;

        if (config.use_random_search) {
            RandomSearchOptim optimizer(config.function.get(),
                config.criterial->clone(),
                config.initial_point,
                config.lower_bounds,
                config.upper_bounds,
                config.delta,
                std::random_device{}(),  // seed
                config.random_search_p,
                config.random_search_alpha);
            result = optimizer.optimize();
        }
        else {
            ConjugateGradientFRConstrained optimizer(config.function.get(),
                config.criterial->clone(),
                config.initial_point,
                config.lower_bounds,
                config.upper_bounds,
                1e-6, 100, config.grad_epsilon);
            result = optimizer.optimize();
        }

        showResults(result, config);

    }
    catch (const std::exception& e) {
        std::cerr << "Optimization error: " << e.what() << std::endl;
    }
}

void ConsoleMenu::showResults(const AbstrOptim::Result& result, const OptimizationConfig& config) {
    std::cout << "\n=== Optimization Results ===" << std::endl;
    std::cout << "Method: " << (config.use_random_search ? "Random Search" : "Conjugate Gradient") << std::endl;

    if (config.use_random_search) {
        std::cout << "Parameters: delta =" << config.delta
            << ", p=" << config.random_search_p << ", alpha=" << config.random_search_alpha << std::endl;
    }

    std::cout << "Stop reason: " << result.stop_reason << std::endl;
    std::cout << "Found minimum point: ";
    printPoint(result.point);
    std::cout << std::endl;
    std::cout << "Function value at minimum: " << result.value << std::endl;
    std::cout << "Number of iterations: " << result.iterations << std::endl;
    std::cout << "Improvement: " << ((*config.function)(config.initial_point) - result.value) << std::endl;
}

void ConsoleMenu::printPoint(const std::vector<double>& point) {
    std::cout << "(";
    for (size_t i = 0; i < point.size(); ++i) {
        std::cout << point[i];
        if (i < point.size() - 1) std::cout << ", ";
    }
    std::cout << ")";
}