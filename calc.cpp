// calc.cpp - Scientific calculator engine using OOP concepts in C++

#include <cctype>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
using namespace std;
const double PI = 3.14159265358979323846;

// ---------------------------------------------------------------
// Custom exception class
// ---------------------------------------------------------------
class CalcError : public runtime_error {
public:
    explicit CalcError(const string &msg) : runtime_error(msg) {}
};

// ---------------------------------------------------------------
// ABSTRACTION: abstract base class for every math function
// ---------------------------------------------------------------
class Function {
public:
    virtual double apply(double x) const = 0;   // pure virtual
    virtual ~Function() {}
};

// ---------------------------------------------------------------
// INHERITANCE: Trigonometric family (shares degree/radian logic)
// ---------------------------------------------------------------
class TrigFunction : public Function {
protected:
    bool degrees;                                // encapsulated data
    double toRad(double x) const { return degrees ? x * PI / 180.0 : x; }
    double fromRad(double x) const { return degrees ? x * 180.0 / PI : x; }
public:
    explicit TrigFunction(bool deg) : degrees(deg) {}
};

class Sin : public TrigFunction {
public:
    using TrigFunction::TrigFunction;
    double apply(double x) const override { return sin(toRad(x)); }
};

class Cos : public TrigFunction {
public:
    using TrigFunction::TrigFunction;
    double apply(double x) const override { return cos(toRad(x)); }
};

class Tan : public TrigFunction {
public:
    using TrigFunction::TrigFunction;
    double apply(double x) const override {
        if (fabs(cos(toRad(x))) < 1e-12) throw CalcError("tan undefined here");
        return tan(toRad(x));
    }
};

class ASin : public TrigFunction {
public:
    using TrigFunction::TrigFunction;
    double apply(double x) const override {
        if (x < -1 || x > 1) throw CalcError("asin domain is -1 to 1");
        return fromRad(asin(x));
    }
};

class ACos : public TrigFunction {
public:
    using TrigFunction::TrigFunction;
    double apply(double x) const override {
        if (x < -1 || x > 1) throw CalcError("acos domain is -1 to 1");
        return fromRad(acos(x));
    }
};

class ATan : public TrigFunction {
public:
    using TrigFunction::TrigFunction;
    double apply(double x) const override { return fromRad(atan(x)); }
};

// Hyperbolic functions
class Sinh : public Function {
public:
    double apply(double x) const override { return sinh(x); }
};
class Cosh : public Function {
public:
    double apply(double x) const override { return cosh(x); }
};
class Tanh : public Function {
public:
    double apply(double x) const override { return tanh(x); }
};

// ---------------------------------------------------------------
// Logarithmic family
// ---------------------------------------------------------------
class LogFunction : public Function {
protected:
    void check(double x, const string &name) const {
        if (x <= 0) throw CalcError(name + " needs a positive number");
    }
};

class Log10 : public LogFunction {
public:
    double apply(double x) const override { check(x, "log"); return log10(x); }
};

class NaturalLog : public LogFunction {
public:
    double apply(double x) const override { check(x, "ln"); return log(x); }
};

// Other functions
class Sqrt : public Function {
public:
    double apply(double x) const override {
        if (x < 0) throw CalcError("sqrt of a negative number");
        return sqrt(x);
    }
};
class Exp : public Function {
public:
    double apply(double x) const override { return exp(x); }
};
class Abs : public Function {
public:
    double apply(double x) const override { return fabs(x); }
};

// ---------------------------------------------------------------
// FunctionRegistry: name -> Function object  (POLYMORPHISM)
// ---------------------------------------------------------------
class FunctionRegistry {
    map<string, unique_ptr<Function>> table;
public:
    explicit FunctionRegistry(bool deg) {
        table["sin"]  = make_unique<Sin>(deg);
        table["cos"]  = make_unique<Cos>(deg);
        table["tan"]  = make_unique<Tan>(deg);
        table["asin"] = make_unique<ASin>(deg);
        table["acos"] = make_unique<ACos>(deg);
        table["atan"] = make_unique<ATan>(deg);
        table["sinh"] = make_unique<Sinh>();
        table["cosh"] = make_unique<Cosh>();
        table["tanh"] = make_unique<Tanh>();
        table["log"]  = make_unique<Log10>();
        table["ln"]   = make_unique<NaturalLog>();
        table["sqrt"] = make_unique<Sqrt>();
        table["exp"]  = make_unique<Exp>();
        table["abs"]  = make_unique<Abs>();
    }
    const Function &get(const string &name) const {
        auto it = table.find(name);
        if (it == table.end()) throw CalcError("Unknown function: " + name);
        return *it->second;            // base-class reference, derived behaviour
    }
};

// ---------------------------------------------------------------
// Parser: recursive-descent expression evaluator
// ---------------------------------------------------------------
class Parser {
    string text;
    size_t pos = 0;
    const FunctionRegistry &functions;

    void skip() { while (pos < text.size() && isspace((unsigned char)text[pos])) pos++; }
    bool eat(char c) {
        skip();
        if (pos < text.size() && text[pos] == c) { pos++; return true; }
        return false;
    }

    static double factorial(double x) {
        if (x < 0 || x != floor(x)) throw CalcError("Factorial needs a whole number >= 0");
        if (x > 170) throw CalcError("Number too large");
        double r = 1;
        for (int i = 2; i <= (int)x; i++) r *= i;
        return r;
    }

    double expr() {                       // + -
        double v = term();
        for (;;) {
            if (eat('+')) v += term();
            else if (eat('-')) v -= term();
            else return v;
        }
    }
    double term() {                       // * /
        double v = unary();
        for (;;) {
            if (eat('*')) v *= unary();
            else if (eat('/')) {
                double d = unary();
                if (d == 0) throw CalcError("Cannot divide by zero");
                v /= d;
            } else return v;
        }
    }
    double unary() {                      // unary + -
        if (eat('-')) return -unary();
        if (eat('+')) return unary();
        return power();
    }
    double power() {                      // ^ (right associative)
        double b = postfix();
        if (eat('^')) {
            double e = unary();
            if (b == 0 && e < 0) throw CalcError("Cannot divide by zero");
            double r = pow(b, e);
            if (isnan(r)) throw CalcError("Invalid power");
            return r;
        }
        return b;
    }
    double postfix() {                    // ! %
        double v = primary();
        for (;;) {
            if (eat('!')) v = factorial(v);
            else if (eat('%')) v /= 100.0;
            else return v;
        }
    }
    double primary() {                    // numbers, ( ), functions, constants
        skip();
        if (pos >= text.size()) throw CalcError("Incomplete expression");

        if (isdigit((unsigned char)text[pos]) || text[pos] == '.') {
            size_t used = 0;
            double v = stod(text.substr(pos), &used);
            pos += used;
            return v;
        }
        if (eat('(')) {
            double v = expr();
            if (!eat(')')) throw CalcError("Missing )");
            return v;
        }
        if (isalpha((unsigned char)text[pos])) {
            string name;
            while (pos < text.size() && isalpha((unsigned char)text[pos])) name += text[pos++];
            if (name == "pi") return PI;
            if (name == "e") return exp(1.0);
            if (!eat('(')) throw CalcError("Expected ( after " + name);
            double x = expr();
            if (!eat(')')) throw CalcError("Missing )");
            return functions.get(name).apply(x);   // polymorphic call
        }
        throw CalcError(string("Unexpected character '") + text[pos] + "'");
    }

public:
    Parser(const string &t, const FunctionRegistry &f) : text(t), functions(f) {}

    double evaluate() {
        double v = expr();
        skip();
        if (pos != text.size()) throw CalcError("Unexpected input");
        if (isnan(v) || isinf(v)) throw CalcError("Math error");
        return v;
    }
};

// ---------------------------------------------------------------
// ScientificCalculator: the main class the outside world uses
// ---------------------------------------------------------------
class ScientificCalculator {
    bool degreeMode;
    FunctionRegistry registry;
public:
    explicit ScientificCalculator(bool deg) : degreeMode(deg), registry(deg) {}

    double calculate(const string &expression) const {
        double r = Parser(expression, registry).evaluate();
        if (fabs(r) < 1e-12) r = 0;       // remove float noise, e.g. sin(180)
        return r;
    }
};

// ---------------------------------------------------------------
int main(int argc, char *argv[]) {
    if (argc < 3) {
        cout << "Error: usage: calc <deg|rad> \"expression\"" << endl;
        return 1;
    }
    bool deg = string(argv[1]) != "rad";
    try {
        ScientificCalculator calc(deg);
        ostringstream out;
        out << setprecision(12) << calc.calculate(argv[2]);
        cout << out.str() << endl;
    } catch (const exception &e) {
        cout << "Error: " << e.what() << endl;
        return 1;
    }
    return 0;
}