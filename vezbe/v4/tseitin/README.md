# Automatsko rezonovanje 2024/2025 - Vežbe
## 4. čas

### Cajtinova (Tseitin) transformacija

Prevođenje formule u CNF može eksponencijalno povećati veličinu formule u odnosu
na veličinu početne formule. U slučaju da pronađemo efikasan algoritam
(algoritam koji radi u polinomijalnom vremenu) za ispitivanje
zadovoljivosti CNF formule, celokupan algoritam bi i dalje bio eksponencijalne
složenosti zbog potencijalnog eksponencijalnog uvećanja veličine formule
tokom njenog transformisanja u CNF.

Transformacijom formule u CNF dobijamo formulu koja je logički ekvivalentna
polaznoj formuli. Međutim, ukoliko je potrebno ispitati zadovoljivost neke formule
dovoljno je svesti je na formulu koja je ekvizadovoljiva polaznoj. Uzimajući u okolnost
ovaku "olakšicu", možemo definisati Cajtinovu konjuktivnu normalnu formu (TCNF).

Osnovna ideja Cajtinove transofrmacije jeste sukcesivno uvođenje novih iskaznih slova odnosno smena za potformule koje se javljaju u formuli koju transformišemo u Cajtinov CNF. Pored uvođenja novih iskaznih slova, uvodi i se i novi konjunkt koji opravdava uvedenu smenu. Ovo je lakše videti na primeru:

Neka je data formula

    (p & q) => (~q | r). 

Uvodimo slovo `s1` kao smenu za `p & q`. Tu smenu opravdavamo uvođenjem novog konjunkta `s1 <=> p & q` u formulu ("slovo `s1` je ekvivalentno (ima isto značenje) kao formula `p & q`"). Sada dobijamo novu formulu

    (s1 => (~q | r)) & (s1 <=> p & q). 

Sada uvodimo smenu `s2` za `~q` i dobijamo

    (s1 => (s2 | r)) & (s1 <=> p & q) & (s2 <=> ~q). 

Sada uvodimo smenu `s3` za `s2 | r` tj.

    (s1 => s3) & (s1 <=> p & q) & (s2 <=> ~q) & (s3 <=> s2 | r)

i na kraju uvodimo smenu `s4` za `s1 => s3`

    s4 & (s1 <=> p & q) & (s2 <=> ~q) & (s3 <=> s2 | r) & (s4 <=> s1 => s3)

Primetimo da smo dobili formulu koja je konjukncija više potformula. Ako date potformule prevedemo u CNF, dobićemo traženu formulu. Primetimo da su sve potformule u obliku:

    p <=> (q BINARY r)
    p <=> (~q)

gde `BINARY` predstavlja neki, bilo koji binarni veznik. Dakle potrebno je samo izvršiti sledeće logički ekvivalentne transformacije:

    p <=> (q & r)     ===    (~p | q) & (~p | r) & (p | ~q | ~r)
    p <=> (q | r)     ===    (p | ~q) & (p | ~r) & (~p | q | r)
    p <=> (q => r)    ===    (p | q) & (p | ~r) & (~p | ~q | r)
    p <=> (q <=> r)   ===    (p | ~q | ~r) & (~p | q | ~r) & (~p | ~q | r) & (p | q | r)
    p <=> ~q          ===    (p | q) & (~p | ~q)

Konačno, Cajtinova CNF naše početne formule je:

    s4 & 
    (~s1 | p) & (~s1 | q) & (s1 | ~p | ~q) & 
    (s2 | q) & (~s2 | ~q) & 
    (s3 | ~s2) & (s3 | ~r) & (~s3 | s2 | r) & 
    (s4 | s1) & (s4 | ~s3) & (~s4 | ~s1 | s3)

Pre nego što navedemo implementaciju ovakve transformacije, prokomentarišimo složenost ovakve transformacije: Broj novouvedenih slova jednak je složenosti formule (jer uvodimo novo slovo za svaki veznik). Pored toga, prilikom transformacija novouvedenih konjukata u CNF, dobija se CNF sa najviše 4 konjukta, pri čemu svaki konjukt ima najviše 3 literala u sebi. Dakle, broj konjukta u Cajtinovoj CNF biće proporcionalan složenosti početne formule (tj. uvećana za konstantni faktor). Ovim zaključujemo da smo eliminsali problem eksponencijalnog uvećanja formule, ali nismo dobili logički ekvivalentnu već samo formulu ekvizadovolivu polaznoj. Međutim, kako smo rekli, za potrebe određivanja zadovoljivosti formule, dovoljno je svesti je na CNF koji je ekvizadovoljiv toj formuli.

Implementacija:

```c++
std::string tseitinRec(const FormulaPtr& f, int& subCount, NormalForm& cnf) {
    if(is<False>(f)) {
        std::string sub = "s" + std::to_string(++subCount);
        cnf.push_back({Literal{false, sub}});
        return sub;
    }
    else if(is<True>(f)) {
        std::string sub = "s" + std::to_string(++subCount);
        cnf.push_back({Literal{true, sub}});
        return sub;
    }
    else if(is<Atom>(f)) {
        return as<Atom>(f).name;
    }
    else if(is<Not>(f)) {
        std::string subformula = tseitinRec(as<Not>(f).subformula, subCount, cnf);
        std::string sub = "s" + std::to_string(++subCount);
        cnf.push_back({
                Literal{false, subformula},
                Literal{false, sub}
        });
        cnf.push_back({
                Literal{true, subformula},
                Literal{true, sub}
        });
        return sub;
    }
    else { // is<Binary>(f)
        Binary b = as<Binary>(f);
        std::string l = tseitinRec(b.left, subCount, cnf);
        std::string r = tseitinRec(b.right, subCount, cnf);
        std::string sub = "s" + std::to_string(++subCount);
        if(b.type == Binary::And) { // s <=> (p & q) == ~s || p
            cnf.push_back({
                Literal{false, sub},
                Literal{true, l}
            });
            cnf.push_back({
                Literal{false, sub},
                Literal{true, r}
            });
            cnf .push_back({
                Literal{true, sub},
                Literal{false, l},
                Literal{false, r}
            });
        }
        else if(b.type == Binary::Or) {
            cnf.push_back({
                Literal{true, sub},
                Literal{false, l}
            });
            cnf.push_back({
                Literal{true, sub},
                Literal{false, r}
            });
            cnf .push_back({
                Literal{false, sub},
                Literal{true, l},
                Literal{true, r}
            });
        }
        else if(b.type == Binary::Impl) {
            cnf.push_back({
                Literal{true, sub},
                Literal{true, l}
            });
            cnf.push_back({
                Literal{true, sub},
                Literal{false, r}
            });
            cnf .push_back({
                Literal{false, sub},
                Literal{false, l},
                Literal{true, r}
            });
        }
        else {
            // b.type == Binary::Eq
            cnf .push_back({
                Literal{false, sub},
                Literal{false, l},
                Literal{true, r}
            });
            cnf .push_back({
                Literal{true, sub},
                Literal{false, l},
                Literal{false, r}
            });
            cnf .push_back({
                Literal{false, sub},
                Literal{true, l},
                Literal{false, r}
            });
            cnf .push_back({
                Literal{true, sub},
                Literal{true, l},
                Literal{true, r}
            });
        }
        return sub;
    }
}

NormalForm tseitin(const FormulaPtr& f) {
    NormalForm cnf;
    int subCount = 0;
    std::string sub = tseitinRec(f, subCount, cnf);
    cnf.push_back({Literal{true, sub}});
    return cnf;
}
```
