#ifndef CALCULATOR_H
#define CALCULATOR_H

#include <string>

class Calculator {
public:
    double evaluateExpression(const std::string& expression);
    std::string preprocessExpression(const std::string& expression);
private:
    int num=0;
    int opnum;
    int getPrecedence(char op);
    double calOp(double a, double b, char op);
};

#endif