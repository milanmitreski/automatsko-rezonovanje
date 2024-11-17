# Automatsko rezonovanje 2024/2025 - Vežbe
## 4. čas

### SAT rešavači

SAT (od engleskog Satisfability) problem jeste problem ispitivanja da li je zadata iskazna formula zadovoljiva. Podsećanja radi, iskazna formula je zadovoljiva ako postoji barem jedna valuacija atoma koji se pojavljuju u toj formuli tako da je istinitosna vrednost formule u toj valuaciji jednaka `true`. SAT rešavač (eng. solver) je program koji za datu iskaznu formulu `f` daje izlaz `SAT` ako je ta formula zadovoljiva, tj. `UNSAT` ako formula nije zadovoljiva.

Glavni cilj kod implementacije SAT rešavača jeste optimizacija. S obzirom da je SAT NP problem (štaviše i NP-kompletan problem), najgora moguća vremenska složenost je eksponencijalna složenost. 

Naivni algoritam (koji je implementiran - pogledati `v2/README.md`) podrazumeva prolazak kroz sve moguće valuacije i određivanje istinitosnih vrednosti formule u tim valuacijama. 

Danas, najveći broj SAT rešavača na ulazu očekuje formulu u CNF-u (primetimo da priroda SAT problema dozvoljava da proizvoljnu formulu svedemo na Cajtinovu CNF i zatim pokrenemo SAT rešavač nad tako dobijenom formulom). Način na koji se CNF formula daje SAT rešavaču jeste DIMACS, na primer:

```text
c Ovo je komentar
c I ovo je komentar
p cnf 3 2
1 2 -3 0
-2 3 0
```

Linije koje počinju sa `c` označavaju komentare i ignorišu se od strane rešavača (_Napomena_: neki rešavači zahtevaju da se komentari isključivo pišu iznad zaglavlja). Zaglavlje DIMACS fajla jeste linija u obliku: `p cnf <broj_atoma> <broj_klauza>`. U ovom zaglavlju, `<broj_atoma>` predstavlja broj različitih atoma koji se pojavljuju u formuli, dok `<broj_klauza>` predstavlja broj klauza koji se pojavljuju u formuli.

Ovde je potrebno napomenuti i format zapisa atoma odnosno literala kod SAT rešavača (sama CNF formula je i dalje konjunkcija klauza, gde klauze predstavljaju disjunkciju literala). S obzirom da je broj atoma konačan, atomi se predstavljaju pozitivnim celobrojnim vrednostima dok se literali predstavljaju negativnim i pozitivnim celobrojnim vrednostima. Literal `l` predstavlja atom (i to baš atom `l`) ako je `l > 0`, a u slučaju `l < 0` predstavlja negaciju atoma `abs(l)`. Vrednost `0` je rezervisana i ne koristi se za označavanje atoma odnosno literala.

Ispod zaglavlja DIMACS fajla, sledi `<broj_klauza>` linija, gde svaka linija predstavlja jednu klauzu u CNF formuli. Linija kojom je predstavljena klauza je u obliku: `l1 l2 ... ln 0` gde su  `l1,l2,...,ln` literali koji čine datu klauzu (`0` je ovde iskorišćena kao oznaka za kraj klauze - nešto slično `;` u brojnim programskim jezicima). Ako atom `l` predstavimo iskaznim slovom `p_l` tada formula data u gornjem DIMACS fajlu jeste `(p_1 | p_2 | ~p_3) & (~p_2 & p_3)`

Sa ovako definisanim formatom, prilikom implementacije SAT rešavača uvodimo drugačiji način reprezentacije normalne forme, klauza, literala i atoma:

```c++
using Atom = int;
using Literal = int;
using Clause = std::vector<Literal>;
using NormalForm = std::vector<Clause>;
```

Takođe, možemo implementirati i funkciju koja parsira DIMACS format:

```c++
NormalForm parse(std::istream& fin, int& atomCount) {
    std::string buffer;
    do {
        fin >> buffer;
        if(buffer == "c")
            fin.ignore(10000, '\n');
    } while(buffer != "p");

    // cnf
    fin >> buffer;

    int clauseCount;
    fin >> atomCount >> clauseCount;

    NormalForm formula;
    for(int i = 0; i < clauseCount; i++) {
        Clause c;

        Literal l;
        fin >> l;
        while(l != 0) {
            c.push_back(l);
            fin >> l;
        }

        formula.push_back(c);
    }

    return formula;
}
```

### DPLL procedura

DPLL procedura jeste jedan od algoritama za implementaciju SAT rešavača (koji na ulazu očekuje formulu u CNF-u). U osnovi DPLL algoritma jeste tehnika bektrekinga (eng. backtracking). Algoritam je u svojoj prirodi rekurzivan i sastoji se od sledećeg koraka:
1. Izabrati atom kojem istinitosna vrednost nije dodeljena i dodeliti mu istinitosnu vrednost tačno.
2. Ako je istinitosna vrednost tako dobijene formule tačno, vratiti `SAT`. U suprotnom vratiti `UNSAT`.
3. Uprostiti formulu nakon dodeljivanja istinitosne vrednost (eliminacija čistih literala (eng. pure literal elimination) i propagacija jediničnih literala (eng. unit propagation)). 
3. Rekurzivno ispitati da li je tako uprošćena formula zadovoljiva ili nije. U zavisnosti od rezultata:
    - Ako je uprošćena formula zadovoljiva, vratiti `SAT`
    - Ako je uprošćena formula nezadovoljiva, postaviti istinitosnu vrednost atoma izabranog u koraku 1. na netačno i ponoviti korake 2. i 3. Nakon toga opet rekurzivno ispitati zadovoljivost uprošćene formule. Ako je formula zadovoljiva, vratiti `SAT`. U suprotnom vratiti `UNSAT`. 

Osnovna razlika između naivnog algoritma i DPLL procedure, jeste uprošćavanje formule nakon izbora istinitosne vrednosti nekog atoma. Tehnike za uprošćavanje formula su **eliminacija čistih literala** i **propagacija jediničnih literala**.

#### Eliminacija čistih literala

Ukoliko se dati atom pojavljuje u isključivo jednom polaritetu u datoj CNF formuli, moguće je odmah dodeliti istinitosnu vrednost tom atomu u saglasnosti sa polaritetom literala koji se pojavljuje u CNF formuli.

Primer: U formuli `(~p | q | r) & (~p | ~q | s) & (~p | ~s | t)` atom `p` se pojavljuje isključivo u negativnom polaritetu pa se vrednost atoma `p` može odmah postaviti na netačno.

Ova tehnika se često koristi samo nad početnom formulom (u fazi preprocesiranja), a ređe na formulama koje se pojavljuju kao među rezultati. Pokazalo se da ova tehnika može umanjiti efikasnost algoritma (odnosno da je pretraga za čistim literalima neefikasna).

#### Propagacija jediničnih literala

Ukoliko u formuli postoji klauza koja sadrži samo jedan literal, moguće je odmah dodeliti istinitosnu vrednost atomu koji čini taj literal, tako da vrednost literala (pa i cele klauze) bude tačno.

Primer: U formuli `p & (~p | q) & (q | ~r)` moguće je odmah dodeliti istinitonsu vrednost tačno atomu `p`.

Ova tehnika se (za razliku od prethodne) koristi nad svim formulama koje se dobijaju kao međurezultati. Pretraga za jediničnim literalima je vremenski efikasna, a njenom primenom prostor pretrage postaje duplo manji.

#### Implementacija DPLL procedure (bez eliminacije čistih literala)

Zbog implementacionih ograničenja (radi efikasnosti), literali i klauze se neće izbacivati kada je to potrebno (recimo nakon propagacije jediničnih literala) već će algoritam uzimati u obzir do tad izabrane istinitosne vrednosti atoma.