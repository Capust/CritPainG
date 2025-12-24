#ifndef ABSTRFUNC_H
#define ABSTRFUNC_H

#include <vector>
#include <cmath>
#include <string>

class AbstrFunc {
public:
    virtual ~AbstrFunc() = default;
    virtual double operator()(const std::vector<double>& x) const = 0;
    virtual std::vector<double> getGradient(const std::vector<double>& x) const = 0;
    virtual std::string getName() const = 0;
    virtual int getDimension() const = 0;
};

// Численное вычисление градиента
std::vector<double> numericalGradient(const AbstrFunc& func, const std::vector<double>& x, double h = 1e-7);

// Функции для R2
class QuadraticFunc2D : public AbstrFunc {
public:
    double operator()(const std::vector<double>& x) const override;
    std::vector<double> getGradient(const std::vector<double>& x) const override;
    std::string getName() const override;
    int getDimension() const override;
};

// Функции для R3
class SphereFunc2D : public AbstrFunc {
public:
    double operator()(const std::vector<double>& x) const override;
    std::vector<double> getGradient(const std::vector<double>& x) const override;
    std::string getName() const override;
    int getDimension() const override;
};

// Функции для R4
class RastriginFunc2D : public AbstrFunc {
private:
    static constexpr double A = 10.0;
public:
    double operator()(const std::vector<double>& x) const override;
    std::vector<double> getGradient(const std::vector<double>& x) const override;
    std::string getName() const override;
    int getDimension() const override;
};

#endif