% komentar pocinje sa procenat
% fof(naziv, tip, formula)
%   &, |, =>, <=>, ~ -- veznici
%   X, Y, Z -- promenljive; c, d, ... -- konstante
%   f(X, c), p(X, Y) -- atomi
%   ![X] -- za svako X
%   ?[Y] -- postoji Y
%   primer: ![X] : p(X) => q(X)
%   =, != -- jednako i razlicito

fof(aksioma1, axiom, ![X]: p(X)).
fof(aksioma2, axiom, ![Y]: p(Y)).
fof(probna_formula, conjecture, ![X]: p(X)).