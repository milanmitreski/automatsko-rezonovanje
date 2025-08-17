fof(politicar_lukavi, axiom, ![X]: (politicar(X) => lukav(X))).
fof(samo_pokvareni_su_politicari, axiom, ![X]: (politicar(X) => pokvaren(X))).
fof(postoji_pokvaren_koji_je_lukav, conjecture, ?[X]: politicar(X) => ?[Y]: (lukav(Y) & pokvaren(Y))).