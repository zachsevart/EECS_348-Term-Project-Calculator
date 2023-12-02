#include <iostream>
#include <vector>
#include <stack>
#include <sstream>
#include <cctype>
#include <cmath>
#include <limits>
#include <stdexcept>

bool isOperator(const char c) {
    return std::string("+-*/%^").find(c) != std::string::npos;
}

bool isUnaryOperator(const std::string &token, const std::string &prevToken) {
    if (token == "+" || token == "-") {
        return prevToken.empty() || prevToken == "(" || isOperator(prevToken[0]);
    }
    return false;
}

int getPrecedence(const char op, bool isUnary = false) {
    if (isUnary) {
        return 4; // Unary operators have the highest precedence
    }
    switch (op) {
        case '+': case '-': return 1;
        case '*': case '/': case '%': return 2;
        case '^': return 3;
        default: return 0;
    }
}

std::vector<std::string> tokenize(const std::string& expression) {
    std::vector<std::string> tokens;
    std::stringstream token;
    std::string lastToken;

    for (char c : expression) {
        if (std::isspace(c)) continue;

        if (isOperator(c) || c == '(' || c == ')') {
            if (token.str().empty() && isUnaryOperator(std::string(1, c), lastToken)) {
                token << c;
            } else {
                for (size_t i = 0; i < expression.size(); ++i) {

                    if (!token.str().empty()) {
                        bool couldBeUnary = token.str() == "+" || token.str() == "-";
                        bool nextCharIsInvalid =
                                (i + 1 < expression.size()) && !isdigit(expression[i + 1]) && expression[i + 1] != '(';

                        if (couldBeUnary && nextCharIsInvalid)
                            throw std::runtime_error("Missing operand after operator " + token.str());

                        tokens.push_back(token.str());
                        token.str("");
                        token.clear();

                    }
                }
                tokens.push_back(std::string(1, c));
            }
            lastToken = std::string(1, c);
        } else if (std::isdigit(c) || c == '.') {
            token << c;
            lastToken = "num";
        } else {
            throw std::runtime_error("Invalid character detected: " + std::string(1, c));
        }
    }

    if (!token.str().empty()) {
        tokens.push_back(token.str());
    }

    return tokens;
}

std::vector<std::string> infixToRPN(const std::vector<std::string>& tokens) {
    std::vector<std::string> rpn;
    std::stack<std::string> stack;

    std::string prevToken;
    for (const auto& token : tokens) {
        if (token.size() == 1 && isOperator(token.front())) {
            bool isUnary = isUnaryOperator(token, prevToken);
            if (isUnary) {
                auto next = &token - &tokens[0] + 1; // Calculate index of the next token
                if (next >= tokens.size() || isOperator(tokens[next].front()) || tokens[next] == ")") {
                    throw std::runtime_error("Missing operand after unary operator: " + token);
                }
            }
            while (!stack.empty() && isOperator(stack.top().front()) &&
                   (getPrecedence(token.front(), isUnary) <= getPrecedence(stack.top().front()))) {
                rpn.push_back(stack.top());
                stack.pop();
            }
            stack.push(token);
        } else if (token == "(") {
            stack.push(token);
        } else if (token == ")") {
            while (!stack.empty() && stack.top() != "(") {
                rpn.push_back(stack.top());
                stack.pop();
            }
            if (!stack.empty()) stack.pop(); // Pop the '(' symbol
        } else {
            rpn.push_back(token);
        }
        prevToken = token;
    }

    while (!stack.empty()) {
        if (stack.top() == "(" || stack.top() == ")") {
            throw std::runtime_error("Mismatched parentheses detected.");
        }
        rpn.push_back(stack.top());
        stack.pop();
    }

    return rpn;
}

#include <limits>
#include <stdexcept>

double evaluateRPN(const std::vector<std::string>& rpn) {
    std::stack<double> evalStack;

    for (const std::string& token : rpn) {
        if (isOperator(token[0]) && token.size() == 1) { // Check for operators
            if (evalStack.size() < 2) {
                if ((token == "-" || token == "+") && evalStack.size() < 2) {
                    // Adjusted the error check to handle missing operands for unary operators
                    if (evalStack.empty()) {
                        throw std::runtime_error("Missing operand for unary operator: " + token);
                    }
                    double value = evalStack.top();
                    evalStack.pop();
                    evalStack.push((token == "-" ? -value : value));
                } else if (evalStack.size() < 2) {
                    // Handle insufficient operands for binary operators
                    throw std::runtime_error("Insufficient operands for binary operator: " + token);
                }
            } else {
                // Binary operator case: pop two operands.
                double rhs = evalStack.top();
                evalStack.pop();
                double lhs = evalStack.top();
                evalStack.pop();

                if ((token == "/" || token == "%") && rhs == 0) {
                    throw std::runtime_error((token == "/" ? "Division" : "Modulo") + std::string(" by zero."));
                }

                double result = 0;
                switch (token[0]) {
                    case '+': result = lhs + rhs; break;
                    case '-': result = lhs - rhs; break;
                    case '*': result = lhs * rhs; break;
                    case '/': result = lhs / rhs; break;
                    case '%': result = static_cast<int>(lhs) % static_cast<int>(rhs); break;
                    case '^': result = std::pow(lhs, rhs); break;
                    default:
                        // This should not happen as all operators are checked
                        throw std::runtime_error("Unexpected error with operator: " + token);
                }
                evalStack.push(result);
            }
        } else { // Token is not an operator, must be a number
            std::istringstream iss(token);
            double value;
            if (!(iss >> value)) {
                throw std::runtime_error("Invalid numeric token: " + token);
            }
            evalStack.push(value);
        }
    }

    if (evalStack.size() != 1) {
        throw std::runtime_error("Error in evaluation. Stack has unexpected size: " + std::to_string(evalStack.size()));
    }

    return evalStack.top();
}



int main() {
    std::vector<std::string> testExpressions = {
            "3 + 4",
            "8 - (5 - 2)",
            "10 * 2 / 5",
            "2 ^ 3",
            "4 * (3 + 2) % 7 - 1",
            "(((2 + 3))) + (((1 + 2)))",
            "((5 * 2) - ((3 / 1) + ((4 % 3))))",
            "(((2 ^ (1 + 1)) + ((3 - 1) ^ 2)) / ((4 / 2) % 3))",
            "(((((5 - 3))) * (((2 + 1))) + ((2 * 3))))",
            "((9 + 6)) / ((3 * 1) / (((2 + 2))) - 1)",
            "+(-2) * (-3) - ((-4) / (+5))",
            "-(+1) + (+2)",
            "-(-(-3)) + (-4) + (+5)",
            "+2 ^ (-3)",
            "-(+2) * (+3) - (-4) / (-5)",
            "2 * (4 + 3 - 1",       // Unmatched Parentheses
            "* 5 + 2",              // Operators Without Operands
            "4 / 0",                // Incorrect Operator Usage (Division by zero)
            "5 (2 + 3)",            // Missing Operator
            "7 & 3",                // Invalid Characters
            "(((3 + 4) - 2) + (1",  // Mismatched Parentheses
            "((5 + 2) / (3 * 0))",  // Invalid Operator Usage (Division by zero)
            "((2 -) 1 + 3)",        // Invalid Operator Sequence
            "((4 * 2) + ( - ))",    // Missing Operand
            "((7 * 3) @ 2)",        // Invalid Characters



    };

    for (const std::string& expression : testExpressions) {
        std::cout << "Evaluating: " << expression << std::endl;
        try {
            auto tokens = tokenize(expression);
            if (tokens.empty()) {
                std::cout << "Tokenization failed: No valid tokens found in expression.\n\n";
                continue;
            }

            auto rpn = infixToRPN(tokens);
            if (rpn.empty()) {
                std::cout << "Conversion to RPN failed: Could not convert infix to postfix notation.\n\n";
                continue;
            }

            double result = evaluateRPN(rpn);
            std::cout << "Result: " << result << "\n\n";
        } catch (const std::runtime_error& e) {
            std::cout << "" << e.what() << "\n\n";
            continue;
        }
    }

    return 0;
}