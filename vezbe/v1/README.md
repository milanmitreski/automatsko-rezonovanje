# Automatsko rezonovanje 2024/2025 - Vežbe
## 1. čas

### Modeliranje iskaznih formula

Implementiran je model za predstavljanje iskaznih formula u C++:

```c++
struct False {};
struct True {};
struct Atom {
    std::string name; // ime atoma
};
struct Not {
    FormulaPtr subformula; // podformula koja se negira
};
struct Binary {
    enum Type {And, Or, Impl, Eq} type; // tip binarnog veznika
    FormulaPtr left, right; // podforumule sa leve, odnosno desne strane veznika
};
```

pri čemu koristimo sledeće alijase tipova:

```c++
using Formula = std::variant<False, True, Or, Impl, Eq>;
using FormulaPtr = std::shared_ptr<Formula>;
```

Zbog specifičnosti kompajlera, potrebno je prvo deklarisati strukture
(bez definicija) pa uvesti alijase tipova i na kraju definisati
svaku od navedenih struktura - za više detalja pogledati `v1/main.cpp`.

Za potrebe lakšeg rada sa ovakvom implementacijom, iskoristili smo dve
klase iz C++ STL bibiloteke:

- `std::variant` klasa predstavlja **uniju**. U našem slučaju, definišemo
  alijas `Formula` za uniju tipova `False, True, Atom, Not, Binary`.\
  *Napomena:* `std::variant` se može koristiti od standarda C++17, pa je
  pri prevođenju potrebno dodati oznaku standarda: `-std=c++17`. U našem
  slučaju, svaki objekat tipa `False, True, Atom, Not, Binary` može se
  posmatrati kao objekat tipa `Formula`. Međutim, pitanje je kako iz
  objekta tipa `Formula` izvući kojeg tipa je taj objekat i kako izvući
  instancu tog objekta. Ovo se postiže korišćenjem dve funkcije:

    - `std::holds_alternative<T>` koja prima objekat tipa `std::variant` i
      tip `T` pri čemu je povratna vrednost `true` ako je objekat koji čuva
      objekat tipa `std::variant` baš tipa T, i `false` u suprotnom.

    - `std::get<T>` koja prima objekat tipa `std::variant` i
      tip `T` pri čemu je povratna vrednost objekat tipa `T` ukoliko je
      objekat koji čuva objekat tipa `std::variant` baš tipa `T`, dok u
      suprotnom vraća grešku tipa `std::bad_variant_access`

  Radi lakšeg korišćenja ovih funkcija, u skladu sa našim potrebama,
  implementirane su funkcije `is<T>` i `as<T>`. Prilikom "raspakivanja"
  objekta tipa `std::variant`, uvek ćemo prvo ispitivati (funkcijom `is<T>`)
  kojeg je tipa objekat koji je sačuvan u pomenutom objektu tipa `std::variant`,
  pa tek nakon toga (funkcijom `as<T>`) izvlačiti objekat tipa `T` iz
  objekta tipa `std::variant`


- `std::shared_pointer<T>` klasa predstavlja **pametne pokazivače** - tj. "wrapper"
  klasu za "obične" pokazivače. Pri radu sa pokazivačima, potrebno je voditi
  računa o curenju memorije, pokazivačima koji pokazuju na dealociranu
  memoriju itd. Pomenuta klasa omogućava lakše korišćenje pokazivača, jer
  "sama" vodi računa o curenju memorije, dealociranim pokazivačima itd. Jednom
  kada je pametni pokazivač inicijalizovan, način rada je identičan radu sa
  običnim pokazivačima. Funkcija `std::make_shared<T>` koristi se za kreiranje
  pametnih pokazivača.

```c++
FormulaPtr ptr(Formula f) { return std::make_shared<Formula>(f); }

template <typename T>
bool is(const FormulaPtr& f) { return std::holds_alternative<T>(*f); }

template <typename T>
T as (const FormulaPtr& f) { return std::get<T>(*f); }
```

Uzevši u obzir rekurzivnu prirodu ovakve implementacije modela iskaznih formula
većina funkcionalnosti koje budu implementarina oslanjaće se na rekurziju.


### Kompleksnost iskazne formule

Definiše se kao broj veznika koji se pojavljuje u okviru jedne iskazne formule.
```c++
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
      return 1 + complexity(as<Not>(f).subformula); // Not je veznik
   }
   if(is<Binary>(f)) {
      return 1 + complexity(as<Binary>(f).left) + complexity(as<Binary>(f).right);
   }
   return 0;
}
```

### Ispisivanje iskazne formule

```c++
std::string print(const FormulaPtr& f) {
   if(is<False>(f)) return "F";
   if(is<True>(f)) return "T";
   if(is<Atom>(f)) return as<Atom>(f).name;
   if(is<Not>(f)) return "~" + print(as<Not>(f).subformula);
   if(is<Binary>(f)) {
      std::string sign; // stringovni zapis binarnog veznika
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
```

### Jednakost dve iskazne formule

```c++
bool equal(const FormulaPtr& f, const FormulaPtr& g) {
   // za tip std::variant, index() je pozicija
   // tipa T u definiciji variant tipa, pri čemu je
   // T tip objekta koji je sačuvan u objektu tipa variant
   // nad kojim pozivamo funkciju index()
   
   // Dakle, ako formule f i g nisu istog tipa - onda nisu jednake
   if(f->index() != g->index()) {
      return false;
   }
   
   // Ako su obe formule iste konstante - jednake su
   if(is<False>(f) || is<True>(f))
      return true;
      
   // Ako su obe formule atomi - jednake su ako su im imena jednaka
   if(is<Atom>(f))
      return as<Atom>(f).name == as<Atom>(g).name;
   
   // Ako su obe formule negacije - jednake su ako su im podformule jednake
   if(is<Not>(f))
      return equal(as<Not>(f).subformula, as<Not>(g).subformula);
   
   // Ako su obe formule binarne - jednake su ako su im jednaki veznici i ako su podformule jednake
   if(is<Binary>(f)) {
      return as<Binary>(f).type == as<Binary>(g).type 
        && equal(as<Binary>(f).left, as<Binary>(g).left) 
        && equal(as<Binary>(f).right, as<Binary>(g).right);
   }

   return false;
}
```

### Valuacije

Valuacija predstavlja dodeljivanje logičkih vrednosti tačno (`true`)
i netačno (`false`) atomima (odnosno promenljivama). Ovo je lako
implementirati preko mape, pri čemu mapiramo stringove (imena atoma)
na `bool` vrednosti. Ovde ponovo uvodimo alijas:

```c++
using Valuation = std::map<std::string, bool>;
```

Za fiksiranu valuaciju, možemo odrediti istinitosnu vrednost date formule
u toj valuaciji. Jasno, pri definisanju vrednosti logičkih konstanti, negacija
i binarnih formula, koristimo (od ranije poznate) istinitosne tablice.

```c++
bool evaluate(const FormulaPtr& f, Valuation& v) {
   if(is<False>(f))
      return false; // vrednost False-a je false
   if(is<True>(f))
      return true; // vrednost True-a je true
   if(is<Atom>(f))
      return v[as<Atom>(f).name]; // Vrednost atoma je vrednost tog atoma u valuaciji
   if(is<Not>(f))
      return !evaluate(as<Not>(f).subformula, v); // Vrednost negacije je suprotna vrednost podformule
   if(is<Binary>(f)) { 
      // Ovde koristimo istinitosne tablice
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
```
