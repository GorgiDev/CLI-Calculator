#include <iostream>
#include <cmath>
#include <string>
#include <stack>
#include <vector>
#include <cctype>
#include <sstream>
#include <map>
#include <stdexcept>

enum TokenType { NUMBER, OPERATOR, PARENTHESIS };

struct Token {
    TokenType type;
    double value;
    char op;
};

int precedence(char op) {
    if (op == '+' || op == '-') 
        return 1;
    if (op == '*' || op == '/') 
        return 2;
    if (op == '^')
        return 3;

    return 0;
}

bool isRightAssociative(char op) {
    return op == '^';
}

std::vector<Token> tokenize(const std::string& expr) {
    std::vector<Token> tokens;
    size_t i = 0;
    while (i < expr.size()) {
        if (isspace(expr[i])) {
            i++;
            continue;
        }

        if (isdigit(expr[i]) || expr[i] == '.') {
            size_t j = i;
            while (j < expr.size() && (isdigit(expr[j]) || expr[j] == '.')) {
                j++;
            }
            std::string numStr = expr.substr(i, j - i);
            double num = std::stod(numStr);
            tokens.push_back({NUMBER, num, 0});
            i = j;
        } else {
            char op = expr[i];
            if (op == '+' || op == '-' || op == '*' || op == '/' || op == '^') {
                tokens.push_back({ OPERATOR, 0, op });
                i++;
            } else if (op == '(' || op == ')') {
                tokens.push_back({ PARENTHESIS, 0, op});
                i++;
            } else {
                throw std::runtime_error(std::string("Invalid Character in expression!") + op);
            }
        }
    }

    return tokens;
}

std::vector<Token> infixToPostfix(const std::vector<Token>& tokens) {
    std::vector<Token> outputQueue;
    std::stack<Token> opStack;

    for (const auto& token : tokens) {
        if (token.type == NUMBER) {
            outputQueue.push_back(token);
        } else if (token.type == OPERATOR) {
            while (!opStack.empty() && opStack.top().type == OPERATOR &&(precedence(opStack.top().op) > precedence(token.op) ||(precedence(opStack.top().op) == precedence(token.op) &&!isRightAssociative(token.op)))) {
                outputQueue.push_back(opStack.top());
                opStack.pop();
            }
            opStack.push(token);
        } else if (token.type == PARENTHESIS) {
            if (token.op == '(') {
                opStack.push(token);
            } else if (token.op == ')') {
                bool foundLeftParenthesis = false;;
                while (!opStack.empty()) {
                    if (opStack.top().type == PARENTHESIS && opStack.top().op == '(') {
                        foundLeftParenthesis = true;
                        opStack.pop();
                        break;
                    } else {
                        outputQueue.push_back(opStack.top());
                        opStack.pop();
                    }
                }
                if (!foundLeftParenthesis) {
                    throw std::runtime_error("Mismatched parenthesis in expreion!");
                }
            }
        }
    }

    while (!opStack.empty()) {
        if (opStack.top().type == PARENTHESIS) {
            throw std::runtime_error("Mismatched parenthesis in expression!");
        }
        outputQueue.push_back(opStack.top());
        opStack.pop();
    }

    return outputQueue;
}

double evaluatePostfix(const std::vector<Token>& postfix) {
    std::stack<double> valStack;

    for (const auto& token : postfix) {
        if (token.type == NUMBER) {
            valStack.push(token.value);
        } else if (token.type == OPERATOR) {
            if (valStack.size() < 2)
                throw std::runtime_error("Invalid expression!");

            double right = valStack.top();
            valStack.pop();
            double left = valStack.top();
            valStack.pop();
            double result = 0;

            switch (token.op) {
                case '+':
                    result = left + right;
                    break;
                case '-':
                    result = left - right;
                    break;
                case '*':
                    result = left * right;
                    break;
                case '/':
                    if (right == 0)
                        throw std::runtime_error("Cannot divide by 0!");
                    
                    result = left / right;
                    break;
                case '^':
                    result = std::pow(left, right);
                    break;
                default:
                    throw std::runtime_error("Unknown operator!");
            }
            valStack.push(result);
        }
    }
    if (valStack.size() != 1)
        throw std::runtime_error("Invalid expression!");
    
        return valStack.top();
}

int main() {
    std::cout << R"(
    _______________________
    |  _________________  |
    | | GorgiDev's Calc | |  Welcome to
    | |_________________| |  GorgiDev's
    |  ___ ___ ___   ___  |  CLI Calculator 
    | | 7 | 8 | 9 | | + | |
    | |___|___|___| |___| |  made in C++!
    | | 4 | 5 | 6 | | - | |
    | |___|___|___| |___| |  
    | | 1 | 2 | 3 | | x | | 
    | |___|___|___| |___| | 
    | | . | 0 | = | | / | |
    | |___|___|___| |___| | 
    |                     |   
    |_____________________|  

    )" << std::endl;

    while (true) {
        std::cout << "Enter expression to calculate (type 'exit' to quit OR type 'help' for help):\n> ";
        std::string expression;
        std::getline(std::cin, expression);

        if (expression == "exit" || expression == "EXIT" || expression == "Exit") {
            std::cout << "Goodbye! Thanks for using GorgiDev's Calculator.\n";
            break; 
        }

        if (expression == "help" || expression == "HELP" || expression == "Help") {
            std::cout << R"(
            
    Operators supported : 
        + = addition
        - = subtraction
        * = multiplication
        / = division
        ^ = power
        () = parenthesis
                
        exit' = quits the app
        'help' = shows help 
                
        NO EQUATIONS AVAILABLE!

            )" << std::endl;
            continue;
        }

        try {
            auto tokens = tokenize(expression);
            auto postfix = infixToPostfix(tokens);

            double result = evaluatePostfix(postfix);
            std::cout << "Result: " << result << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }

    std::cin.get();
    return 0;
}