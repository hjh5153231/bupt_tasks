#include <string>
#include <iostream>
#include "calculator.h"
using namespace std;
int main(){
    system("chcp 65001");
    Calculator calculator;
    cout << "简易计算器，请输入表达式（按回车计算，输入q退出）：" << endl;
    string input;
    while(1){
        getline(cin,input);
        if(input=="q"){
            break;
        }
        try {
            input=calculator.preprocessExpression(input); //算式预处理，(针对负号的情况)
            double result = calculator.evaluateExpression(input);//计算结果
            cout << "结果：" << result << endl;
        } catch (...) {
            cout << "表达式错误，请重新输入：" << endl;
            continue;
        }
    }
}