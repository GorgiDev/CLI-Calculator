#define _USE_MATH_DEFINES
#include <algorithm>
#include <iostream>
#include <cmath>
#include <string>
#include <stack>
#include <vector>
#include <cctype>
#include <sstream>
#include <map>
#include <stdexcept>

#ifdef _WIN32
#include <windows.h>
void clearScreen() {
    HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    DWORD count;
    DWORD cellCount;
    COORD homeCoords = { 0, 0 };

    if (hStdOut == INVALID_HANDLE_VALUE) return;

    if (!GetConsoleScreenBufferInfo(hStdOut, &csbi)) return;
    cellCount = csbi.dwSize.X * csbi.dwSize.Y;

    if (!FillConsoleOutputCharacter(hStdOut, ' ', cellCount, homeCoords, &count)) return;
    if (!FillConsoleOutputAttribute(hStdOut, csbi.wAttributes, cellCount, homeCoords, &count)) return;

    SetConsoleCursorPosition(hStdOut, homeCoords);
}
#else
void clearScreen() {
    std::cout << "\033[2J\033[1;1H";
}
#endif

enum CalcTokenType { NUMBER, OPERATOR, PARENTHESIS, FUNCTION, VARIABLE };

struct Token {
    CalcTokenType type;
    double value;
    char op;
    std::string funcName;
    std::string varName;
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

std::map<std::string, double> variables;

std::vector<Token> tokenize(const std::string& expr) {
    std::vector<Token> tokens;
    size_t i = 0;
    std::map<std::string, Token> functions = {
        {"sin", Token{FUNCTION, 0, 0, "sin", ""}},
        {"cos", Token{FUNCTION, 0, 0, "cos", ""}},
        {"tan", Token{FUNCTION, 0, 0, "tan", ""}},
        {"log", Token{FUNCTION, 0, 0, "log", ""}},
        {"ln",  Token{FUNCTION, 0, 0, "ln", ""}},
        {"sqrt",Token{FUNCTION, 0, 0, "sqrt", ""}},
        {"abs", Token{FUNCTION, 0, 0, "abs", ""}},
        {"exp", Token{FUNCTION, 0, 0, "exp", ""}},
    };

    Token* lastToken = nullptr;

    while (i < expr.size()) {
        if (isspace(expr[i])) {
            i++;
            continue;
        }

        Token newToken;

        if (isdigit(expr[i]) || expr[i] == '.') {
            size_t j = i;
            while (j < expr.size() && (isdigit(expr[j]) || expr[j] == '.')) {
                j++;
            }
            std::string numStr = expr.substr(i, j - i);
            double num = std::stod(numStr);
            newToken.type = NUMBER;
            newToken.value = num;
            newToken.op = 0;
            newToken.funcName.clear();
            newToken.varName.clear();
            i = j;
        } else if (isalpha(expr[i])) {
            size_t j = i;
            while (j < expr.size() && isalpha(expr[j])) {
                j++;
            }
            std::string func = expr.substr(i, j - i);
            if (functions.count(func)) {
                Token funcToken = functions[func];
                newToken = funcToken;
            } else if (func == "pi") {
                newToken.type = NUMBER;
                newToken.value = M_PI;
                newToken.op = 0;
                newToken.funcName.clear();
                newToken.varName.clear();
            } else if (func == "e") {
                newToken.type = NUMBER;
                newToken.value = M_E;
                newToken.op = 0;
                newToken.funcName.clear();
                newToken.varName.clear();
            } else {
                newToken.type = VARIABLE;
                newToken.varName = func;
                newToken.value = 0;
                newToken.op = 0;
                newToken.funcName.clear();
            }
            i = j;
        } else {
            char op = expr[i];
            if (op == '+' || op == '-' || op == '*' || op == '/' || op == '^') {
                bool isUnary = false;
                if (op == '-') {
                    if (tokens.empty() || (tokens.back().type == OPERATOR && tokens.back().op != ')')
                        || (tokens.back().type == PARENTHESIS && tokens.back().op == '(')) {
                        isUnary = true;
                    }
                }
                if (isUnary) {
                    newToken.type = NUMBER;
                    newToken.value = 0;
                    newToken.op = 0;
                    newToken.funcName.clear();
                    newToken.varName.clear();
                    i++;
                } else {
                    newToken.type = OPERATOR;
                    newToken.value = 0;
                    newToken.op = op;
                    newToken.funcName.clear();
                    newToken.varName.clear();
                    i++;
                }
            } else if (op == '(' || op == ')') {
                newToken.type = PARENTHESIS;
                newToken.value = 0;
                newToken.op = op;
                newToken.funcName.clear();
                newToken.varName.clear();
                i++;
            } else {
                throw std::runtime_error(std::string("Invalid Character in expression!") + op);
            }
        }

        // Special handling for unary minus directly followed by '-' operator in input, e.g., "0-..."
        if (newToken.type == NUMBER && newToken.value == 0 && i < expr.size() && expr[i] == '-') {
            if (lastToken != nullptr) {
                bool lastIsNumberVarOrCloseParen =
                    (lastToken->type == NUMBER) ||
                    (lastToken->type == VARIABLE) ||
                    (lastToken->type == PARENTHESIS && lastToken->op == ')');

                bool newIsNumber = newToken.type == NUMBER;

                if (lastIsNumberVarOrCloseParen && newIsNumber) {
                    Token mulToken;
                    mulToken.type = OPERATOR;
                    mulToken.value = 0;
                    mulToken.op = '*';
                    mulToken.funcName.clear();
                    mulToken.varName.clear();
                    tokens.push_back(mulToken);
                }
            }

            tokens.push_back(newToken);
            lastToken = &tokens.back();

            Token minusToken;
            minusToken.type = OPERATOR;
            minusToken.value = 0;
            minusToken.op = '-';
            minusToken.funcName.clear();
            minusToken.varName.clear();
            tokens.push_back(minusToken);
            lastToken = &tokens.back();

            i++;

            continue;
        }

        if (lastToken != nullptr) {
            bool lastIsNumberVarOrCloseParen =
                (lastToken->type == NUMBER) ||
                (lastToken->type == VARIABLE) ||
                (lastToken->type == PARENTHESIS && lastToken->op == ')');

            bool newIsVarFuncNumberOrOpenParen =
                (newToken.type == VARIABLE) ||
                (newToken.type == FUNCTION) ||
                (newToken.type == NUMBER) ||
                (newToken.type == PARENTHESIS && newToken.op == '(');

            if (lastIsNumberVarOrCloseParen && newIsVarFuncNumberOrOpenParen) {
                Token mulToken;
                mulToken.type = OPERATOR;
                mulToken.value = 0;
                mulToken.op = '*';
                mulToken.funcName.clear();
                mulToken.varName.clear();
                tokens.push_back(mulToken);
            }
        }

        tokens.push_back(newToken);
        lastToken = &tokens.back();
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
            while (!opStack.empty() && opStack.top().type == OPERATOR &&
                   (precedence(opStack.top().op) > precedence(token.op) ||
                   (precedence(opStack.top().op) == precedence(token.op) && !isRightAssociative(token.op)))) {
                outputQueue.push_back(opStack.top());
                opStack.pop();
            }
            opStack.push(token);
        } else if (token.type == PARENTHESIS) {
            if (token.op == '(') {
                opStack.push(token);
            } else if (token.op == ')') {
                bool foundLeftParenthesis = false;
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
                    throw std::runtime_error("Mismatched parenthesis in expression!");
                }

                if (!opStack.empty() && opStack.top().type == FUNCTION) {
                    outputQueue.push_back(opStack.top());
                    opStack.pop();
                }
            }
        } else if (token.type == FUNCTION) {
            opStack.push(token);
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
        } else if (token.type == FUNCTION) {
            if (valStack.empty()) throw std::runtime_error("Missing argument for function!");
            double arg = valStack.top(); valStack.pop();
            double result;

            if (token.funcName == "sin")
                result = sin(arg);
            else if (token.funcName == "cos")
                result = cos(arg);
            else if (token.funcName == "tan")
                result = tan(arg);
            else if (token.funcName == "log")
                result = log10(arg);
            else if (token.funcName == "ln")
                result = log(arg);
            else if (token.funcName == "sqrt")
                result = sqrt(arg);
            else if (token.funcName == "abs")
                result = std::abs(arg);
            else if (token.funcName == "exp")
                result = std::exp(arg);
            else
                throw std::runtime_error("Unknown function: " + token.funcName);

            valStack.push(result);
        } else if (token.type == VARIABLE) {
            if (!variables.count(token.varName)) {
                throw std::runtime_error("Unknown variable: " + token.varName);
            }
            valStack.push(variables[token.varName]);
        }
    }
    if (valStack.size() != 1)
        throw std::runtime_error("Invalid expression!");

    return valStack.top();
}

double evaluateExpressionWithVariable(const std::string& expr, const std::string& varName, double varValue) {
    std::string replacedExpr = expr;
    size_t pos = 0;
    std::string varValueStr = "(" + std::to_string(varValue) + ")";
    while ((pos = replacedExpr.find(varName, pos)) != std::string::npos) {
        replacedExpr.replace(pos, varName.length(), varValueStr);
        pos += varValueStr.length();
    }
    auto tokens = tokenize(replacedExpr);
    auto postfix = infixToPostfix(tokens);
    return evaluatePostfix(postfix);
}

void solveLinearEquation(const std::string& equation) {
    size_t eqPos = equation.find('=');
    if (eqPos == std::string::npos) {
        throw std::runtime_error("No '=' found in equation!");
    }

    std::string lhs = equation.substr(0, eqPos);
    std::string rhs = equation.substr(eqPos + 1);

    // Find variable name by scanning lhs
    std::string varName;
    for (char ch : lhs) {
        if (isalpha(ch)) {
            varName += ch;
        }
    }
    if (varName.empty()) {
        std::cerr << "No variable found in equation." << std::endl;
        return;
    }

    // Evaluate expression value with variable = 0 => constant term
    double valAtZero = evaluateExpressionWithVariable(lhs + "-(" + rhs + ")", varName, 0);
    // Evaluate expression value with variable = 1 => coeff + constant
    double valAtOne = evaluateExpressionWithVariable(lhs + "-(" + rhs + ")", varName, 1);

    double coeff = valAtOne - valAtZero;
    double constant = valAtZero;

    const double EPS = 1e-12;
    if (std::abs(coeff) < EPS) {
        if (std::abs(constant) < EPS)
            std::cout << "Infinite solutions (identity equation)." << std::endl;
        else
            std::cout << "No solution (contradiction)." << std::endl;
        return;
    }

    double x = -constant / coeff;
    std::cout << varName << " = " << x << std::endl;
}

int main() {
    #ifdef _WIN32
        SetConsoleOutputCP(CP_UTF8);
    #endif
    auto trim = [](std::string& s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch){ return !std::isspace(ch); }));
        s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch){ return !std::isspace(ch); }).base(), s.end());
    };

    std::cout << R"(

  ╔══════════════════════════════════════════════════════════════════════════════════════════╗
  ║      ██████╗  █████╗ ██╗      ██████╗██╗   ██╗██╗      █████╗ ████████╗ █████╗ ██████╗   ║
  ║     ██╔════╝ ██╔══██╗██║     ██╔════╝██║   ██║██║     ██╔══██╗╚══██╔══╝██   ██╗██╔══██   ║
  ║     ██║  ███╗███████║██║     ██║     ██║   ██║██║     ███████║   ██║   ██   ██║████╝     ║
  ║     ██║   ██║██╔══██║██║     ██║     ██║   ██║██║     ██╔══██║   ██║   ██   ██║██╔╝██╔   ║
  ║     ╚██████╔╝██║  ██║███████╗╚██████╗╚██████╔╝███████╗██║  ██║   ██║   ║█████║║██║ ██║   ║
  ║      ╚═════╝ ╚═╝  ╚═╝╚══════╝ ╚═════╝ ╚═════╝ ╚══════╝╚═╝  ╚═╝   ╚═╝   ╚═════╝╝╚═╝ ╚═╝   ║
  ╚══════════════════════════════════════════════════════════════════════════════════════════╝

                            WELCOME TO GorgiDev's CLI CALCULATOR!
                                       <<<MADE IN C++>>>


    )" << std::endl;

    while (true) {
        std::cout << "Enter expression to calculate (type 'help' for help):\n> ";
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

    You can assign variables:
        e.g. x = 3 * 5

    You can solve linear equations in one variable:
        e.g. 3x + 2 = 0

    You can also use mathematical functions:
        sin
        cos
        tan
        log
        ln
        sqrt
        abs
        exp

    Commands:
        exit   = quits the app
        help   = shows this message
        clear  = clears the screen

    Note:
        ONLY ALPHABETIC EQUATIONS ALLOWED

            )" << std::endl;
            continue;
        }

        if (expression == "clear" || expression == "CLEAR" || expression == "Clear") {
            clearScreen();
            continue;
        }

        trim(expression);
        if (expression.empty())
            continue;

        size_t eqPos = expression.find('=');
        if (eqPos != std::string::npos) {
            std::string beforeEq = expression.substr(0, eqPos);
            std::string afterEq = expression.substr(eqPos + 1);

            // Check if there is any alphabetic character before '=' to distinguish equation vs assignment
            bool isEquation = beforeEq.find_first_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ") != std::string::npos;

            if (isEquation) {
                // Solve linear equation like "3x + 2 = 0"
                try {
                    solveLinearEquation(expression);
                } catch (const std::exception& e) {
                    std::cerr << "Error solving equation: " << e.what() << std::endl;
                }
            } else {
                // Variable assignment like "x = 3 * 5"
                std::string varName = beforeEq;
                varName.erase(std::remove_if(varName.begin(), varName.end(), ::isspace), varName.end());

                if (varName.empty() || !std::all_of(varName.begin(), varName.end(), ::isalpha)) {
                    std::cerr << "Invalid variable name!" << std::endl;
                    continue;
                }

                try {
                    auto tokens = tokenize(afterEq);
                    auto postfix = infixToPostfix(tokens);
                    double val = evaluatePostfix(postfix);
                    variables[varName] = val;
                    std::cout << varName << " = " << val << std::endl;
                } catch (const std::exception& e) {
                    std::cerr << "Error: " << e.what() << std::endl;
                }
            }
            continue;
        }

        // If no '=', just evaluate expression normally
        try {
            auto tokens = tokenize(expression);
            auto postfix = infixToPostfix(tokens);
            double result = evaluatePostfix(postfix);
            std::cout << "Result: " << result << std::endl;
        } catch (const std::invalid_argument&) {
            std::cerr << "Error: Invalid number format\n";
        } catch (const std::out_of_range&) {
            std::cerr << "Error: Number too large" << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }
    return 0;
}