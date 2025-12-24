#ifndef ABSTRCRITERIAL_H
#define ABSTRCRITERIAL_H

#include <vector>
#include <memory> 
#include <string>

class AbstrOptim;
class AbstrCriterial {
protected:
    const AbstrOptim* optimizer;  ///< ”казатель на оптимизатор дл€ доступа к дополнительным данным

public:
    AbstrCriterial(const AbstrOptim* opt);
    virtual ~AbstrCriterial() = default;
    virtual bool isSatisfied(const std::vector<double>& current_point,
        double current_value,
        int current_iteration) const = 0;
    virtual std::string getName() const = 0;
    virtual std::unique_ptr<AbstrCriterial> clone() const = 0;
};

class CriterialMaxIter : public AbstrCriterial {
private:
    int max_iterations;
public:
    CriterialMaxIter(const AbstrOptim* opt, int max_iter);
    bool isSatisfied(const std::vector<double>& current_point, double current_value, int current_iteration) const override;
    std::string getName() const override;
    std::unique_ptr<AbstrCriterial> clone() const override;
};

class CriterialLastImprovement : public AbstrCriterial {
private:
    int max_iterations_without_improvement;
    mutable int last_improvement_iteration;
    mutable double best_value_so_far; // Kучшее найденное значение функции
    mutable bool first_call;

public:
    CriterialLastImprovement(const AbstrOptim* opt, int max_iter_no_imp);
    bool isSatisfied(const std::vector<double>& current_point, double current_value, int current_iteration) const override;
    std::string getName() const override;
    std::unique_ptr<AbstrCriterial> clone() const override;
};

class CriterialFunctionChange : public AbstrCriterial {
private:
    double epsilon;
    mutable double previous_value;
    mutable std::vector<double> previous_point;
    mutable bool first_call;

public:
    CriterialFunctionChange(const AbstrOptim* opt, double eps);
    bool isSatisfied(const std::vector<double>& current_point,
        double current_value,
        int current_iteration) const override;
    std::string getName() const override;
    std::unique_ptr<AbstrCriterial> clone() const override;
};

class CriterialGradientNorm : public AbstrCriterial {
private:
    double epsilon;
    mutable bool first_call;

public:
    CriterialGradientNorm(const AbstrOptim* opt, double eps);
    bool isSatisfied(const std::vector<double>& current_point, double current_value, int current_iteration) const override;
    std::string getName() const override;
    std::unique_ptr<AbstrCriterial> clone() const override;
};

class CriterialPointChange : public AbstrCriterial {
private:
    double epsilon;
    mutable std::vector<double> previous_point;
    mutable bool first_call;

public:
    CriterialPointChange(const AbstrOptim* opt, double eps);
    bool isSatisfied(const std::vector<double>& current_point, double current_value, int current_iteration) const override;
    std::string getName() const override;
    std::unique_ptr<AbstrCriterial> clone() const override;
};

#endif
