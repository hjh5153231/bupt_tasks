#include <iostream>
#include <string>
#include <stack>
#include <cmath>
#include "Calculator.h"
using namespace std;

string Calculator::preprocessExpression(const string& expression) {//对负数进行预处理
    string preprocessed = expression;
    bool isValid=true;
    stack<char> bracket;
    for (int i = 0; i < preprocessed.length(); i++) {
        if(preprocessed[i]==' '){
            continue;
        }
        if(preprocessed[i]=='('){
            bracket.push(')');
        }
        if(preprocessed[i]==')'){
            if(bracket.empty()){
                isValid=false;
            }
            else{
                bracket.pop();
            }
        }
        if (preprocessed[i] == '-') {
            if (i == 0 || preprocessed[i - 1] == '(') {
                preprocessed.insert(i, "0");//检测到负数，则在前方加一个0来处理
            }
        }
    }
    if(!bracket.empty()){
        isValid=false;
    }
    if(!isValid){
        exception e;
        throw e;
    }
    return preprocessed;
}

double Calculator::evaluateExpression(const string& expression) {//计算表达式
    stack<double> values;
    stack<char> ops;

    for (int i = 0; i < expression.length(); i++) {
        char c = expression[i];

        if (c == ' '){
            continue;
        }
        else if (isdigit(c) || c == '.') {
            // 将表达式中的数提取出来
            int j = i;
            string numStr;
            while (j < expression.length() && (isdigit(expression[j]) || expression[j] == '.')) {
                numStr += expression[j];
                j++;
            }
            i = j - 1;  //将i定位到这个数最后一位
            double num = stod(numStr);//string类型转换成double类型
            values.push(num);
        } else if (c == '(') {
            ops.push(c);
        } else if (c == ')') {//遇到右括号就把这对括号中的算式值算出来，压入运算数栈
            while (!ops.empty() && ops.top() != '(') {
                double b = values.top();
                values.pop();
                double a = values.top();
                values.pop();
                char op = ops.top();
                ops.pop();
                values.push(calOp(a, b, op));
            }
            ops.pop(); // 弹出 '('
        } else {
            // 处理运算符
            while (!ops.empty() && getPrecedence(ops.top()) >= getPrecedence(c)) { //维护一个优先级单调递增的符号栈
                double b = values.top();
                values.pop();
                double a = values.top();
                values.pop();
                char op = ops.top();
                ops.pop();
                values.push(calOp(a, b, op));
            }
            ops.push(c);
        }
    }

    while (!ops.empty()) { //若符号栈中未计算的符号全部计算并弹出
        if(values.empty()){
            exception e;
            throw e;
            break;
        }
        double b = values.top();
        values.pop();
        if(values.empty()){
            exception e;
            throw e;
            break;
        }
        double a = values.top();
        values.pop();
        char op = ops.top();
        ops.pop();
        values.push(calOp(a, b, op));
    }

    return values.top();
}

int Calculator::getPrecedence(char op) {//获取符号优先级
    if (op == '+' || op == '-'){
        return 1;
    }
    else if (op == '*' || op == '/'){
        return 2;
    }
    return 0;
}

double Calculator::calOp(double a, double b, char op) {//计算某个运算符两侧数的值
    if(b==0 && op=='/'){
        exception e;
        throw e;
    }
    switch (op) {
        case '+': return a + b;
        case '-': return a - b;
        case '*': return a * b;
        case '/': return a / b;
        default: return 0;
    }
}