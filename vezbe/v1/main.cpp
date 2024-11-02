#include <iostream>
#include <variant>
#include <map>

struct False;
struct True;
struct Atom;
struct Not;
struct Binary;

using Formula = std::variant<False, True, Atom, Not, Binary>;
using FormulaPtr = std::shared_ptr<Formula>;
using Valuation = std::map<std::string, bool>;

struct False {};
struct True {};
struct Atom { std::string name; };
struct Not { FormulaPtr subformula; };
struct Binary {
   enum Type { And, Or, Impl, Eq } type;
   FormulaPtr left, right;
};

FormulaPtr ptr(Formula f) { return std::make_shared<Formula>(f); }

template <typename T>
bool is(const FormulaPtr& f) { return std::holds_alternative<T>(*f); }

template <typename T>
T as (const FormulaPtr& f) { return std::get<T>(*f); }

int complexity(const FormulaPtr& f) {
   if(is<False>(f)) {
      return 0;
   }
   if(is<True>(f)) {
      return 0;
   }
   if(is<Atom>(f)) {
      return 0;
   }
   if(is<Not>(f)) {
      return 1 + complexity(as<Not>(f).subformula);
   }
   if(is<Binary>(f)) {
      return 1 + complexity(as<Binary>(f).left) + complexity(as<Binary>(f).right);
   }
   return 0;
}

std::string print(const FormulaPtr& f) {
   if(is<False>(f)) return "F";
   if(is<True>(f)) return "T";
   if(is<Atom>(f)) return as<Atom>(f).name;
   if(is<Not>(f)) return "~" + print(as<Not>(f).subformula);
   if(is<Binary>(f)) {
      std::string sign;
      switch(as<Binary>(f).type) {
         case Binary::And: sign = "&"; break;
         case Binary::Or: sign = "|"; break;
         case Binary::Impl: sign = "->"; break;
         case Binary::Eq: sign = "<->"; break;
      }
      return "(" + print(as<Binary>(f).left) + " " + sign + " " + print(as<Binary>(f).right) + ")";
   }
   return "";
}

bool evaluate(const FormulaPtr& f, Valuation& v) {
   if(is<False>(f))
      return false;
   if(is<True>(f))
      return true;
   if(is<Atom>(f))
      return v[as<Atom>(f).name];
   if(is<Not>(f))
      return !evaluate(as<Not>(f).subformula, v);
   if(is<Binary>(f)) {
      bool l_eval = evaluate(as<Binary>(f).left, v);
      bool r_eval = evaluate(as<Binary>(f).right, v);
      switch(as<Binary>(f).type) {
         case Binary::And: return l_eval && r_eval;
         case Binary::Or: return l_eval || r_eval;
         case Binary::Impl: return !l_eval || r_eval;
         case Binary::Eq: return l_eval == r_eval;
      }
   }
   return false;
}

bool equal(const FormulaPtr& f, const FormulaPtr& g) {
   if(f->index() != g->index()) {
      return false;
   }

   if(is<False>(f) || is<True>(f))
      return true;

   if(is<Atom>(f))
      return as<Atom>(f).name == as<Atom>(g).name;

   if(is<Not>(f))
      return equal(as<Not>(f).subformula, as<Not>(g).subformula);

   if(is<Binary>(f)) {
      return as<Binary>(f).type == as<Binary>(g).type && equal(as<Binary>(f).left, as<Binary>(g).left) && equal(as<Binary>(f).right, as<Binary>(g).right);
   }

   return false;
}

int main() {
   FormulaPtr p = ptr(Atom{"p"});
   FormulaPtr q = ptr(Atom{"q"});
   FormulaPtr p_and_q = ptr(Binary{Binary::And, p, q});
   std::cout << complexity(p_and_q) << std::endl;
   std::cout << print(p_and_q) << std::endl;
   Valuation v = {{"p", true}, {"q", false}};
   std::cout << (evaluate(p_and_q, v) ? "True" : "False") << std::endl;
   return 0;
}
