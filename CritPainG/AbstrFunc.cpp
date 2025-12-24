#include "pch.h"
#include "AbstrFunc.h"
#include <stdexcept>
#include <string>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef M_E
#define M_E 2.71828182845904523536
#endif

// Численное вычисление градиента 
std::vector<double> numericalGradient(const AbstrFunc& func, const std::vector<double>& x, double h) {
    std::vector<double> grad(x.size());
    std::vector<double> x_plus = x;

    for (size_t i = 0; i < x.size(); ++i) {
        x_plus[i] = x[i] + h;
        double f_plus = func(x_plus);

        x_plus[i] = x[i] - h;
        double f_minus = func(x_plus);

        grad[i] = (f_plus - f_minus) / (2.0 * h);
        x_plus[i] = x[i];
    }

    return grad;
}

double QuadraticFunc2D::operator()(const std::vector<double>& x) const {
    if (x.size() != 2) {
        throw std::invalid_argument("QuadraticFunc2D requires exactly 2 dimensions");
    }
    double dx = x[0] - 3.0;
    double dy = x[1] + 1.0;
    return dx * dx + dy * dy;
}

std::vector<double> QuadraticFunc2D::getGradient(const std::vector<double>& x) const {
    if (x.size() != 2) {
        throw std::invalid_argument("QuadraticFunc2D requires exactly 2 dimensions");
    }
    std::vector<double> grad(2);
    grad[0] = 2 * (x[0] - 3.0);
    grad[1] = 2 * (x[1] + 1.0);
    return grad;
}

std::string QuadraticFunc2D::getName() const {
    return "Quadratic 2D: f(x,y) = (x-3)^2 + (y+1)^2";
}

int QuadraticFunc2D::getDimension() const {
    return 2;
}

double SphereFunc2D::operator()(const std::vector<double>& x) const {
    if (x.size() != 2) {
        throw std::invalid_argument("SphereFunc2D requires exactly 2 dimensions");
    }
    return x[0] * x[0] + x[1] * x[1];
}

std::vector<double> SphereFunc2D::getGradient(const std::vector<double>& x) const {
    if (x.size() != 2) {
        throw std::invalid_argument("SphereFunc2D requires exactly 2 dimensions");
    }
    return { 2.0 * x[0], 2.0 * x[1] };
}

std::string SphereFunc2D::getName() const {
    return "Sphere 2D: f(x,y) = x^2 + y^2";
}

int SphereFunc2D::getDimension() const {
    return 2;
}

double RastriginFunc2D::operator()(const std::vector<double>& x) const {
    if (x.size() != 2) {
        throw std::invalid_argument("RastriginFunc2D requires exactly 2 dimensions");
    }
    return 2 * A + x[0] * x[0] - A * cos(2.0 * M_PI * x[0])
        + x[1] * x[1] - A * cos(2.0 * M_PI * x[1]);
}

std::vector<double> RastriginFunc2D::getGradient(const std::vector<double>& x) const {
    if (x.size() != 2) {
        throw std::invalid_argument("RastriginFunc2D requires exactly 2 dimensions");
    }
    return {
        2.0 * x[0] + 2.0 * M_PI * A * sin(2.0 * M_PI * x[0]),
        2.0 * x[1] + 2.0 * M_PI * A * sin(2.0 * M_PI * x[1])
    };
}

std::string RastriginFunc2D::getName() const {
    return "Rastrigin 2D: f(x,y) = 20 + x^2 + y^2 - 10(cos(2pix) + cos(2piy))";
}

int RastriginFunc2D::getDimension() const {
    return 2;
}