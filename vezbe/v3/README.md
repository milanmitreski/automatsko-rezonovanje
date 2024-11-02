# Automatsko rezonovanje 2024/2025 - Vežbe
## 3. čas

### Normalne forme

Normalne forme su oblici iskaznih formula koje su pogodne za rešavanje određenih zadataka.
Potrebno je definisati algoritam za transformaciju formula u neku datu normalnu formu.
Iako je transformacija formule sintaksička operacija, često se opravdanje za takve
transformacije nalazi u semantičkom značenju pravila transformacija.
Najčešće se zahteva da formula u normalnoj formi bude ekvivalentna polaznoj, ali
se može zahtevati i (samo) ekvizadovoljivost.

### Negaciona normalna forma (NNF)

Formula je u negacionoj normalnoj formi (NNF) ako i samo ako je sastavljenja
od literala (atomi ili negacija atoma) korišćenjem isključivo veznika `And` ili `Or`
ili je logička konstanta (`True` ili `False`).

Dakle, da bi proizvoljnu formulu "preveli" u NNF, potrebno je
1. Eliminsati korišćenje veznika `Impl` i `Eq`
2. Spustiti negacije na nivo atoma

Dakle, potrebno je definisati pravila kako za proizvoljnu formulu možemo sprovesti
potrebne korake.

```
Pravila:

1. 
P => Q      ===  ~P || Q
P <=> Q     ===  (P && Q) || (~P && ~Q)
            ===  (P || ~Q) && (~P || Q)  

2.         
~~P         ===  P
~(P && Q)   ===  ~P || ~Q
~(P || Q)   ===  ~P && ~Q
~(P => Q)   ===  P && ~Q
~(P <=> Q)  ===  (~P || ~Q) && (P || Q)
            ===  (~P && Q) || (P && ~Q)
```

Ono što primećujemo jeste da nakon eliminacije `Impl` i `Eq`, može se pojaviti
potreba da ponovo eliminišemo `Not` . Zbog toga, potrebno je da prvo implementiramo
algoritam kojim eliminišemo `Not`, pa onda možemo eliminisati `Impl` i `Eq`.

Međutim, radi lakše implementacije, ova dva koraka ćemo uslovno rečeno spojiti u jedan.
Implementiraćemo dve funkcije - `nnf` i `nnfNot`.
1. Funkcija `nnf` prevodi nenegiranu formulu u NNF
2. Funkcija `nnfNot` prevodi negiranu formulu u NNF

Ideja je da svaki put kada je potrebno da formulu koja nije negirana prevedemo u NNF
koristimo funkciju `nnf`, a kada je potrebno da formulu koja jeste negirana prevedemo u NNF
koristimo funkciju `nnfNot`.

```c++
FormulaPtr nnf(const FormulaPtr& f);

FormulaPtr nnfNot(const FormulaPtr& f) {
    // negacija atoma već jeste u NNF-u
    if(is<Atom>(f))
        return ptr(Not{f});
    // dvostruku negaciju formule prevodimo u NNF
    // tako što formulu prevedemo u NNF
    if(is<Not>(f))
        return nnf(as<Not>(f).subformula);
        
    // ovde koristimo goredefinisana pravila
    auto b = as<Binary>(f);
    if(b.type == Binary::And)
        return ptr(Binary{Binary::Or, nnfNot(b.left), nnfNot(b.right)});
    if(b.type == Binary::Or)
        return ptr(Binary{Binary::And, nnfNot(b.left), nnfNot(b.right)});
    if(b.type == Binary::Impl)
        return ptr(Binary{Binary::And, nnf(b.left), nnfNot(b.right)});
    if(b.type == Binary::Eq)
        return ptr(Binary{
            Binary::Or,
            ptr(Binary{Binary::And, nnf(b.left), nnfNot(b.right)}),
            ptr(Binary{Binary::And, nnfNot(b.left), nnf(b.right)})
        });
}

FormulaPtr nnf(const FormulaPtr& f) {
   // konstante i atomi već jesu u NNF-u
   if(is<False>(f) || is<True>(f) || is<Atom>(f))
       return f;
   // za trasnformaciju negacije formule u NNF koristimo nnfNot funkciju
   if(is<Not>(f))
       return nnfNot(as<Not>(f).subformula);
       
   // Ovde koristimo goredefinisana pravila
   auto b = as<Binary>(f);
   if(b.type == Binary::And)
       return ptr(Binary{Binary::And, nnf(b.left), nnf(b.right)});
   if(b.type == Binary::Or)
       return ptr(Binary{Binary::Or, nnf(b.left), nnf(b.right)});
   if(b.type == Binary::Impl)
       return ptr(Binary{Binary::Or, nnfNot(b.left), nnf(b.right)});
   if(b.type == Binary::Eq)
       return ptr(Binary{Binary::And,
                         ptr(Binary{Binary::Or, nnfNot(b.left), nnf(b.right)}),
                         ptr(Binary{Binary::Or, nnf(b.left), nnfNot(b.right)})
                  });
   return FormulaPtr{};
}
```

### Konjuktivna normalna forma (CNF)

Formula je u konjuktivnoj normalnoj formi (CNF) ako i samo ako je sastavljena
od konjukcija **klauza** pri čemu su klauze disjunkcije literala. Jednostavnosti
radi, uvešćemo novi tip `Literal` (koji u suštini predstavlja `Atom` ili `Not{Atom}`)
kao i alijase `Clause` i `NormalForm` koji su alijasi za listu `Literal`-a tj. `Clause`-a.
Tu implementiramo i funkciju za ispisivanje objekata tipa `NormalForm` (koji u suštini
predstavljaju listu listi literala).

```c++
struct Literal {
    bool pos;
    std::string name;
};

using Clause = std::vector<Literal>;
using NormalForm = std::vector<Clause>;

void print(const NormalForm& f) {
    for(const auto& clause : f) {
        std::cout << "[ ";
        for (const auto &literal: clause)
            std::cout << (literal.pos ? "" : "~") << literal.name << " ";
        std::cout << "]";
    }
    std::cout << std::endl;
}

```

Potrebno je implementirati algoritam kojim se proizvoljna formula prevodi u CNF.
Možemo iskoristiti implementiranu funkciju da formulu prvo prevedemo u NNF, pa
je potrebno implementirati funkciju koja NNF formulu prevodi u CNF. Ponovo je
potrebno definisati pravila kojim prevodimo NNF formule u CNF.

```
Pravila:

P || (Q && R)  ===  (P || Q) && (P || R)
(Q && R) || P  ===  (Q || P) && (R || P)
```

Međutim, za zapis CNF-a ne koristimo isti model kao za zapis obične formule.
Zato je potrebno i prevesti zapis koji smo već implementirali u zapis za CNF,
preko tipova `NormalForm, Clause, Literal`. Ovaj posao nam olakšava činjenica
da je potrebno razmatrati samo formule koje su zapisane u NNF-u i njih
prevesti u zapis za CNF.

```
True             ===  {} 
                      // Normalna forma bez klauza
False            ===  {{}} 
                      // Normalna forma sa praznom klauzom
Atom(name)       ===  {{Literal{true, name}}} 
                      // Normalna forma sa klauzom koja sadrzi jedan literal
Not(Atom(name))  ===  {{Literal{false, name}}}
                      // Normalna forma sa klauzom koja sadrzi jedan literal
And(p, q)        ===  {toCnf(p), toCnf(q)}
                      // prevedemo p i q u CNF - to ce biti liste klauza
                      // te dve liste nadovezemo
Or(p, q)         ===  cross(toCnf(p), toCnf(q))
                      // prevedemo p i q u CNF - problem je sto 
                      // sad moramo Or koje se nalazi izmedju 
                      // njih da spustimo skroz dole: (A1..BM su klauze - disjunkcije)
                      // (A1 && ... && AN) || (B1 && ... && BM) 
                      // (A1 || B1) && ...  && (A1 || BM) &&  ... && (AN || BM)
                      // Dakle moramo svake dve klauze spojiti iz CNF formi od p i q
```

Iz ovih pravila zaključujemo da su nam potrebne funkcije za nadovezivanje
kao i za spajanje svake dve klauze iz dve date normalne forme.

```c++
template<typename List>
List concat(const List& l, const List& r) {
    // Nadovezivanje liste l i liste r
    List result;
    std::copy(begin(l), end(l), std::back_inserter(result));
    std::copy(begin(r), end(r), std::back_inserter(result));
    return result;
}

NormalForm cross(const NormalForm& l, const NormalForm& r) {
    // Prolazimo korz klauze od l, pa za svaku klauzu u l
    // na nju redom dodajemo klauze iz r i svaki rezultat
    // posebno dodajemo u result
    NormalForm result;
    for(const auto& lc : l)
        for(const auto& rc : r)
            result.push_back(concat(lc, rc));
    return result;
}
```

Sada konačno možemo implementirati algoritam za prevođenje formule
zadate u NNF-u u CNF (u skladu sa gorenavedenim pravilima).

```c++
NormalForm cnf(const FormulaPtr& f) {
    if(is<True>(f))
        return {};
    if(is<False>(f))
        return {{}};
    if(is<Atom>(f))
        return {{Literal{true, as<Atom>(f).name}}};
    if(is<Not>(f))
        return {{Literal{false, as<Atom>(as<Not>(f).subformula).name}}};
    auto b = as<Binary>(f);
    if(b.type == Binary::And)
        return concat(cnf(b.left), cnf(b.right));
    if(b.type == Binary::Or)
        return cross(cnf(b.left), cnf(b.right));
    return NormalForm{};
}
```

### Disjunktivna normalna forma (DNF)

Formula je u disjunktivnoj normalnoj formi (DNF) ako i samo ako je sastavljena
od disjunkcija **klauza** pri čemu su klauze konjunkcije literala. Implementacija funkcije koja prevodi formulu u DNF
zadata je kao 1. domaći zadatak na kursu.