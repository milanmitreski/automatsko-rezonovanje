#include<iostream>
#include<variant>
#include<vector>
#include<set>
#include<map>
#include<functional>

/* Uvodjenje termova */

struct Variable;
struct Function;

using Term = std::variant<Variable, Function>;
using TermPtr = std::shared_ptr<Term>;
using VariableSet = std::set<std::string>;

struct Variable {
    std::string name;
};
struct Function {
    std::string symbol;
    std::vector<TermPtr> args;
};

/* Uvodjenje sintaksnog stabla za logiku prvog reda */

struct False;
struct True;
struct Atom;
struct Not;
struct Binary;
struct Quantifier;

using Formula = std::variant<False, True, Atom, Not, Binary, Quantifier>;
using FormulaPtr = std::shared_ptr<Formula>;

struct False {};
struct True {};
struct Atom {
    std::string symbol;
    std::vector<TermPtr> args;
};
struct Not {
    FormulaPtr subformula;
};
struct Binary {
    enum Type { And, Or, Impl, Eq } type ;
    FormulaPtr left, right;
};
struct Quantifier {
    enum Type { All, Exists } type;
    std::string variable;
    FormulaPtr subformula;
};

/* Pomocne funkcije */

TermPtr ptr(const Term& term) { return std::make_shared<Term>(term); }
FormulaPtr ptr(const Formula& formula) { return std::make_shared<Formula>(formula); }

template<typename T> bool is(const TermPtr& term) { return std::holds_alternative<T>(*term); }
template<typename T> bool is(const FormulaPtr& formula) { return std::holds_alternative<T>(*formula); }

template<typename T> T as(const TermPtr& term) { return std::get<T>(*term); }
template<typename T> T as(const FormulaPtr& formula) { return std::get<T>(*formula); }

/* Ispisivanje formula */

std::string print(const TermPtr& t) {
    if(is<Variable>(t)) {
        return as<Variable>(t).name;
    } else { // is<Function>(t)
        std::string result = as<Function>(t).symbol;
        if(!as<Function>(t).args.empty()) {
            result += "(" + print(as<Function>(t).args[0]);
            for(unsigned i = 1; i < as<Function>(t).args.size(); i++) {
                result += ", " + print(as<Function>(t).args[i]);
            }
            result += ")";
        }
    }
}

std::string print(const FormulaPtr& f) {
    if(is<False>(f)) {
        return "F";
    } else if(is<True>(f)) {
        return "T";
    } else if(is<Atom>(f)) {
        std::string result = as<Atom>(f).symbol;
        if(!as<Atom>(f).args.empty()) {
            result += "(" + print(as<Atom>(f).args[0]);
            for (unsigned i = 1; i < as<Atom>(f).args.size(); i++) {
                result += ", " + print(as<Atom>(f).args[i]);
            }
            result += ")";
        }
    } else if(is<Not>(f)) {
        return "~" + print(as<Not>(f).subformula);
    } else if(is<Binary>(f)) {
        std::string sign;
        switch(as<Binary>(f).type) {
            case Binary::And: sign = " & "; break;
            case Binary::Or: sign = " | "; break;
            case Binary::Impl: sign = " -> "; break;
            default: /* Binary::Eq */ sign = " <-> "; break;
        }
        return "(" + print(as<Binary>(f).left) + sign + print(as<Binary>(f).right) + ")";
    } else { // is<Quantifier>(f)
        std::string sign;
        switch (as<Quantifier>(f).type) {
            case Quantifier::All: sign = "A "; break;
            default: /* Quantifier::Exists */ sign = "E "; break;
        }
        return sign + as<Quantifier>(f).variable + " " + print(as<Quantifier>(f).subformula);
    }
}

/* Signatura tj. jezik */

struct LSignature {
    std::map<std::string, unsigned> functions;
    std::map<std::string, unsigned> relations;
};

/* Definisemo semantiku tj. interpretaciju */

using LDomain = std::set<unsigned>;
using LFunction = std::function<unsigned(const std::vector<unsigned>&)>;
using LRelation = std::function<unsigned(const std::vector<unsigned>&)>;

struct LStructure {
    LSignature signature;
    LDomain domain;
    std::map<std::string, LFunction> functions;
    std::map<std::string, LRelation> relations;
};

bool checkSignature(const TermPtr& t, const LSignature& s) {
    if(is<Variable>(t)) {
        return true;
    } else { // is<Function>(t)
        Function f = as<Function>(t);
        
        if(s.functions.find(f.symbol) == s.functions.end()) {
            return false;
        }
        if(s.functions.at(f.symbol) != f.args.size()) {
            return false;
        }
        
        for(auto& term : f.args) {
            if(!checkSignature(term, s)) {
                return false;
            }
        }  
        return true;
    }
}

bool checkSignature(const FormulaPtr& f, const LSignature& s) {
    if(is<False>(f) || is<True>(f)) {
        return true;
    } else if (is<Atom>(f)) {
        Atom a = as<Atom>(f);

        if(s.relations.find(a.symbol) == s.relations.end()) {
            return false;
        }
        if(s.relations.at(a.symbol) != a.args.size()) {
            return false;
        }

        for(auto& term : a.args) {
            if(!checkSignature(term, s))
                return false;
        }
        return true;
    } else if (is<Not>(f)) {
        return checkSignature(as<Not>(f).subformula, s);
    } else if (is<Binary>(f)) {
        return checkSignature(as<Binary>(f).left, s) && checkSignature(as<Binary>(f).right, s);
    } else { // is<Quantifier>(f)
        return checkSignature(as<Quantifier>(f).subformula, s);
    }
}

/* Definicija valuacije */

using LValuation = std::map<std::string, unsigned>;

unsigned evaluate(const TermPtr& t, const LStructure& s, const LValuation& v) {
    if(is<Variable>(t)) {
        return v.at(as<Variable>(t).name);
    } else { // is<Function>(t)
        Function f = as<Function>(t);
        LFunction lf = s.functions.at(f.symbol);

        std::vector<unsigned> eval_args;
        for(auto& arg : f.args) {
            eval_args.push_back(evaluate(arg, s, v));
        }
        
        return lf(eval_args);
    }
}

bool evaluate(const FormulaPtr& f, const LStructure& s, const LValuation& v) {
    if(is<False>(f)) {
        return false;
    } else if(is<True>(f)) {
        return true;
    } else if(is<Atom>(f)) {
        Atom a = as<Atom>(f);
        LRelation lr = s.relations.at(a.symbol);

        std::vector<unsigned> eval_args;
        for(auto& arg : a.args) {
            eval_args.push_back(evaluate(arg, s, v));
        }

        return lr(eval_args);
    } else if(is<Not>(f)) {
        return !evaluate(as<Not>(f).subformula, s, v);
    } else if(is<Binary>(f)) {
        Binary b = as<Binary>(f);
        bool left = evaluate(b.left, s, v), right = evaluate(b.right, s, v);
        switch (b.type) {
        case Binary::And:
            return left && right;
        case Binary::Or:
            return left || right;
        case Binary::Impl:
            return !left || right;
        default: // Binary::Eq
            return left == right;
        }
    } else { // is<Quantifier>(f)
        Quantifier q = as<Quantifier>(f);
        if(q.type == Quantifier::All) {
            LValuation extended(v);
            for(auto& x : s.domain) {
                extended[q.variable] = x;
                if(!evaluate(q.subformula, s, extended)) {
                    return false;
                }
            }
            return true;
        } else { // q.type == Quantifier::Exists
            LValuation extended(v);
            for(auto& x : s.domain) {
                extended[q.variable] = x;
                if(evaluate(q.subformula, s, extended)) {
                    return true;
                }
            }
            return false;
        }
    }
}

void getVariables(const TermPtr& t, VariableSet& v) {
    if(is<Variable>(t)) {
        v.insert(as<Variable>(t).name);
    } else { // is<Function(t)>
        for(auto& arg : as<Function>(t).args) {
            getVariables(arg, v);
        }
    }
}

void getVariables(const FormulaPtr& f, VariableSet& v, bool includeBound) {
    if(is<False>(f) || is<True>(f)) {
        return;
    } else if(is<Atom>(f)) {
        for(auto& arg : as<Atom>(f).args) {
            getVariables(arg, v);
        }
    } else if(is<Not>(f)) {
        getVariables(as<Not>(f).subformula, v, includeBound);
    } else if(is<Binary>(f)) {
        getVariables(as<Binary>(f).left, v, includeBound);
        getVariables(as<Binary>(f).right, v, includeBound);
    } else { // is<Quantifier>(f)
        if(includeBound) {
            getVariables(as<Quantifier>(f).subformula, v, includeBound);
            v.insert(as<Quantifier>(f).variable);
        } else {
            bool varHasFreeOccurrence = v.find(as<Quantifier>(f).variable) != v.end();
            getVariables(as<Quantifier>(f).subformula, v, includeBound);
            if(!varHasFreeOccurrence)
                v.erase(as<Quantifier>(f).variable);
        }
    }
}

bool containsVariable(const TermPtr& t, const std::string& var) {
    VariableSet v;
    getVariables(t, v);
    return v.find(var) != v.end();
}

bool containsVariable(const FormulaPtr& f, const std::string& var, bool includeBound) {
    VariableSet v;
    getVariables(f, v, includeBound);
    return v.find(var) != v.end();
}

std::string uniqueVar(const FormulaPtr& f, const TermPtr& g) {
    VariableSet f_v, g_v;
    getVariables(g, g_v);
    getVariables(f, f_v, false);

    static unsigned uniqueCounter = 0;
    std::string var;
    do {
        var = "u" + std::to_string(++uniqueCounter);
    } while((f_v.find(var) != f_v.end()) || (g_v.find(var) != g_v.end()));
    
    return var;
}

TermPtr substitute(const TermPtr& t, const std::string& var, const TermPtr& subterm) {
    if(is<Variable>(t)) {
        if(as<Variable>(t).name == var) {
            return subterm;
        } else {
            return t;
        }
    } else { // is<Function>(t)
        std::vector<TermPtr> substituted;
        for(auto& arg : as<Function>(t).args) {
            substituted.push_back(substitute(arg, var, subterm));
        }
        return ptr(Function{as<Function>(t).symbol, substituted});
    }
}

FormulaPtr substitute(const FormulaPtr& f, const std::string& var, const TermPtr& subterm) {
    if(is<False>(f) || is<True>(f)) {
        return f;
    } else if(is<Atom>(f)) {
        std::vector<TermPtr> substituted;
        for(auto& arg : as<Atom>(f).args) {
            substituted.push_back(substitute(arg, var, subterm));
        }
        return ptr(Atom{as<Atom>(f).symbol, substituted});
    } else if(is<Not>(f)) {
        return ptr(Not{substitute(as<Not>(f).subformula, var, subterm)});
    } else if(is<Binary>(f)) {
        return ptr(Binary{
            as<Binary>(f).type,
            substitute(as<Binary>(f).left, var, subterm),
            substitute(as<Binary>(f).right, var, subterm)
        });
    } else { // is<Quantifier>(f)
        Quantifier q = as<Quantifier>(f);

        if(q.variable == var) {
            return f;
        } else if(containsVariable(subterm, q.variable)) {
            std::string unique = uniqueVar(f, subterm);
            FormulaPtr subformula_unique = substitute(q.subformula, q.variable, ptr(Variable{unique}));
            return ptr(Quantifier{q.type, unique, substitute(subformula_unique, var, subterm)});
        } else {
            return ptr(Quantifier{q.type, q.variable, substitute(q.subformula, var, subterm)});
        }
    }
} 

int main() {
    return 0;
}