; int x, y, m;
; if(x > y)
;   m = x;
; else
;   m = y
;
; Prove that m = max{x,y} <=> (m >= x & m >= y & (m == x || m == y))
(set-logic QF_LIA)
(declare-fun x () Int)
(declare-fun y () Int)
(declare-fun m () Int)

(assert
    (and 
        ; Program specification (P)
        ; (=> (> x y) (= m x))
        ; (=> (not (> x y)) (= m y))
        (ite (> x y) (= m x) (= m y))
        
        ; Postcondition negation (~Q)
        (not (and
            (>= m x)
            (>= m y)
            (or (= m x) (= m y))
        ))
    )
)

(check-sat)
(exit)