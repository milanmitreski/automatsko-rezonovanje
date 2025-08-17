fof(asocijativnost, axiom, ![X, Y, Z]: f(X, f(Y, Z)) = f(f(X, Y), Z)).
fof(postoji_neutral, axiom, ?[E]: ![X]: (f(X, E) = X & f(E, X) = X)).
fof(jedinstvenost, conjecture, ![E1, E2]: (
   (![X]: (f(X, E1) = X & f(E1, X) = X))
   &
   (![X]: (f(X, E2) = X & f(E2, X) = X))
   =>
   E1 = E2 
)).