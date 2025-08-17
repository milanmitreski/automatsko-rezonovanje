# Automatsko rezonovanje 2024/2025 - Vežbe
## 2. čas

### Supstitucija (zamena) formule formulom

```c++
FormulaPtr substitute(const FormulaPtr& f, const FormulaPtr& what, const FormulaPtr& with) {
    // Svako pojavljivanje formule what u formuli f menjamo formulom with
    
    // Ako je f jednaka formuli what, rezultat je formula with (celu formulu menjamo)
    if(equal(f, what)) 
        return with;
    if(is<False>(f) || is<True>(f) || is<Atom>(f)) 
        return f;
    if(is<Not>(f))
        return ptr(Not{substitute(as<Not>(f).subformula, what, with)}); 
    if(is<Binary>(f)) {
        auto binaryF = as<Binary>(f);
        return ptr(Binary{
            binaryF.type,
            substitute(binaryF.left, what, with), 
            substitute(binaryF.right, what, with)
        });
    }
    return FormulaPtr{}; 
}
```

### Istinitosne tablice formula

U zavisnosti od date valuacije, možemo odrediti istinitosnu vrednost
zadate formule. Sledeći korak jeste da u zavisnosti od svih mogućih
valuacija, odredimo istinitosne vrednosti date formula u tim valuacijama.


Prvo, potrebno nam je da iz date formule izvučemo skup svih atoma koji
se nalaze u toj formuli.

```c++
void getAtoms(const FormulaPtr& f, AtomSet& atoms) { 
    // False i True nemaju atome u sebi
    // Ako je f atom, dodajemo ime tog atoma u naš skup
    if(is<Atom>(f))
        atoms.insert(as<Atom>(f).name);
    // U ostalim slučajevima, tražimo atome u podformulama
    else if(is<Not>(f)) 
        getAtoms(as<Not>(f).subformula, atoms);
    else if(is<Binary>(f)) { 
        getAtoms(as<Binary>(f).left, atoms);
        getAtoms(as<Binary>(f).right, atoms);
    }
}
```

Ovde takođe koristimo alijas

```c++
using AtomSet = std::set<std::string>;
```

S obzirom da smo već implementirali funkciju `evaluate`, koja evaluira
vrednost date formule u datoj valuaciji, potrebno je još implementirati
algoritam kojim možemo generisati sve moguće valuacije za dati skup atoma.

```c++
bool next(Valuation& v) {
    auto it = begin(v);
    while(it != end(v) && it->second) {
        it->second = false;
        it++;
    }
    
    if(it == end(v))
        return false;

    return it->second = true;
}
```

Generisanje valuacija radićemo na sledeći način: počinjemo od početne
valuacije u kojoj su vrednosti svih atoma `false`. Nakon toga, u `while`
petlji pozivamo funkciju `next` koja vraća vrednost `true` ukoliko postoji
naredna valuacija (koju ranije nismo izgenerisali)  i `false` u suprotnom
(za poslednju valuaciju smatramo valuaciju u kojoj su vrednosti svih atoma
`true`). Primetimo da funkcija `next` prime referencu na objekat tipa `Valuation`
što znači da će se prosleđeni objekat promeniti i te promene će biti
vidljive u funkciji u kojoj smo pozvali funkciju `next`.

Pitanje je kako (za datu valuaciju) generišemo sledeću valuaciju.
Posmatrajmo problem rekurzivno: ako želimo da izgenerišemo sve valuacije
od `n` atoma, potrebno je izgenerisati sve valuacije od `n-1` atoma i
za svaku od tih valuacija generisati dve valuacije od od `n` atoma - jednu
kada je vrednost poslednjeg atoma `false`, a drugu kada je vrednost poslednjeg
atoma `true`.

Algoritam je sledeći: za datu valuaciju, prolazimo kroz vrednosti svih atoma.
Ukoliko su sve vrednosti `true`, algoritam će postaviti vrednosti svih atoma
na `false` (vratiti se na početnu valuaciju) i vratiti `false` kojim signalizira
da ne postoji sledeća valuacija. U suprotnom, algoritam pronalazi prvi atom
čija je vrednost `false`, postavlja ga na `true` i vraća `true`.

Korektnost algoritma se zasniva na sledećoj opservaciji: Neka se prvi atom
čija je vrednost `false` nalazi na poziciji `i` - to znači da su
na pozicijama `1..i-1` vrednosti atoma `true` koje su ranije (za fiksirane
vrednosti atoma na pozicijama `i...n`) bile postavljene na `false` - tj. za
konkretne date vrednosti atoma na pozicijama `i...n`, prošli smo kroz sve "podvaluacije"
atoma na pozicijama `1..i-1`. Samim tim, potrebno je da promenimo konfiguraciju
vrednosti atoma na pozicijama `i...n` (i logično, promenićemo vrednost prvog
dostupnog atoma, na poziciji `i` - i to postavaljamo na `true`), pri čemu
ćemo resetovati vrednosti atoma na pozicijama `1..i-1` na `false` i potrebno je ponovo
izgenerisati sve "podvaluacije" atoma na pozicijama `1..i-1`. Kada ponovo dođemo
do valuacije gde su atomi na pozicijama `1..i-1` vrednosti `true`, sada će i vrednost
atoma na poziciji `i` biti `true`, pa ovaj ceo postupak ponavaljamo sa promenljivom
na poziciji `i+1`. Proces generisanja valuacija teče na sledeći način (sa `0` označavamo `false`,
a sa `1` označavamo `false`)

```
0 0 0 ... 0 0 0 ... 0 -->
1 0 0 ... 0 0 0 ... 0 --> // prošli smo sve podvaluacije prvog atoma, za datu konfiguraciju vrednosti ostalih atoma
0 1 0 ... 0 0 0 ... 0 --> // postavili smo vrednost drugog atoma na 1, idemo kroz podvaluacije prvog atoma
1 1 0 ... 0 0 0 ... 0 --> // sad smo prošli kroz sve podvaluacije prva dva atoma
0 0 1 ... 0 0 0 ... 0 --> // sad idemo opet kroz podvaluacije prva dva atoma, ali je vrednost trećeg atoma 1
1 0 1 ... 0 0 0 ... 0 --> // prošli smo sve podvaluacije prvog atoma, za datu konfiguraciju vrednosti ostalih atoma
0 1 1 ... 0 0 0 ... 0 --> // postavili smo vrednost drugog atoma na 1, idemo kroz podvaluacije prvog atoma
1 1 1 ... 0 0 0 ... 0 --> // sad smo prošli kroz sve podvaluacije prva dva atoma kao i sve podvaluacije prva tri atoma
itd.
```

Sada lako možemo ispisati tablicu istinitosne vrednosti date formule u svakoj valuaciji:

```c++
void table(const FormulaPtr& f) {
    
    // nalazimo skup atoma koji se pojavljuju u f
    AtomSet atoms;
    getAtoms(f, atoms);
    
    // formiramo početnu valuaciju od nađenih atoma, gde su vrednosti svih atoma postavljene na false
    Valuation v;
    for(const std::string& atom : atoms) {
        v[atom] = false;
        std::cout << atom << ' ';
    }
    std::cout << std::endl;
    
    // prolazimo kroz valuacije, ispisujemo valuaciju i vrednost f u toj valuaciji
    do {
        print(v);
        std::cout << "| " << evaluate(f, v) << std::endl;
    } while(next(v));
}
```

Ostalo je još da implementiramo funkciju `print` za ispisivanje vrednosti atoma u datoj valuaciji

```c++
void print(Valuation& v) {
    for(const auto& [atom, value] : v)
        std::cout << value << ' ';
}
```

### Zadovoljivost formule

Formula je zadovoljiva ukoliko postoji neka valuacija takva da je istinitosna vrednost
date formule u toj valuaciji jednaka `true`.

```c++
std::optional<Valuation> isSatisfiable(const FormulaPtr& f) {
    AtomSet atoms;
    getAtoms(f, atoms);

    Valuation v;
    for(const auto& atom : atoms)
        v[atom] = false;

    do {
        if(evaluate(f, v))
            return v;
    } while(next(v));
    return {};
}
```

Ovde koristimo klasu `std::optional<T>` - želimo da, u slučaju zadovoljivosti prosleđene
formule, vratimo valuaciju u kojoj je ta formula tačna. Međutim, šta vratiti ukoliko
formula nije zadovoljiva? Ovde koristimo pomenutu klasu, čiji objekat može sadržati objekat
tipa T, ali može i biti prazan (ovo nam označava da formula nije zadovoljiva)

Koristimo identičnu ideju kao kod formiranja istinitosne tablice - prolazimo kroz
sve valuacije - ukoliko naiđemo na valuaciju u kojoj je data formula tačna - tu stajemo i
vraćamo tu valuaciju (kao dokaz da je naša formula zadovoljiva). U suprtnom vraćamo praznu
vrednost.

### Eliminacija konstanti

Sledeći korak pri radu sa iskaznim formulama jeste njihovo pojednostavljivanje, tako da
dobijena formula bude **logički ekvivalentna** polaznoj (tj. da su vrednosti formula jednake
za svaku datu valuaciju). Prvi korak u tome jeste eliminiacija konstanti.

```c++
FormulaPtr simplify(const FormulaPtr& f) {
    // ne možemo eliminisati konstantu ako je cela formula konstanta
    // ili ako u njoj nema konstanti (u slučaju da je formula Atom)
    if(is<False>(f) || is<True>(f) || is<Atom>(f))
        return f;

    // Ovde pokušavamo da eliminišemo konstantu iz podformule
    // Ako pojednostavljenjem dobijemo logičku konstantu
    // Možemo i negaciju pojednostaviti (suprotnom logičkom konstantom
    // koju ćemo kasnije eliminsati (osim u slučaju, da je posle ovakvog
    // pojednostavljenja, cela formula jednaka konstanti))  
    if(is<Not>(f)) {
        FormulaPtr s = simplify(as<Not>(f).subformula);
        if(is<True>(s))
            return ptr(False{});
        if(is<False>(s))
            return ptr(True{});
        return ptr(Not{s});
    }

    // slično radimo i slučaju da je f binarna formula
    // pojednostavljujemo levu i desnu stranu i u zavisnosti
    // od veznika, pojednostavljujemo izraz ako se leva i/ili desna
    // strana pojednostavi do konstante
    auto b = as<Binary>(f);
    FormulaPtr ls = simplify(b.left);
    FormulaPtr rs = simplify(b.right);
    if(b.type == Binary::And) {
        if(is<False>(ls) || is<False>(rs))
            return ptr(False{});
        if(is<True>(ls))
            return rs;
        if(is<True>(rs))
            return ls;
        return ptr(Binary{Binary::And, ls, rs});
    }
    if(b.type == Binary::Or) {
        if(is<True>(ls) || is<True>(rs))
            return ptr(True{});
        if(is<False>(ls))
            return rs;
        if(is<False>(rs))
            return ls;
        return ptr(Binary{Binary::Or, ls, rs});
    }
    if(b.type == Binary::Impl) {
        if(is<False>(ls) || is<True>(rs))
            return ptr(True{});
        if(is<True>(ls))
            return rs;
        if(is<False>(rs))
            return ptr(Not{ls});
        return ptr(Binary{Binary::Impl, ls, rs});
    }
    if(b.type == Binary::Eq) {
        if(is<True>(ls))
            return rs;
        if(is<True>(rs))
            return ls;
        if(is<False>(ls) && is<False>(rs))
            return ptr(True{});
        if(is<False>(ls))
            return ptr(Not{rs});
        if(is<False>(rs))
            return ptr(Not{ls});
        return ptr(Binary{Binary::Eq, ls, rs});
    }
    return FormulaPtr{};
}
```