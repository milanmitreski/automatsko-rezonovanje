; int x, y, m;
; if(x > y)
;   m = x;
; else
;   m = y
;
; Prove that m = max{x,y} <=> (m >= x & m >= y & (m == x || m == y))
(set-logic QF_BV) ; Quantifier Free Bit Vector
(declare-fun x () (_ BitVec 32)) ; _ -- we have a parameter, 32 -- size of the BitVector
(declare-fun y () (_ BitVec 32))
(declare-fun m () (_ BitVec 32))

(assert
    (and 
        ; Program specification (P)
        ; (=> (> x y) (= m x))
        ; (=> (not (> x y)) (= m y))
        (ite (bvsgt x y) (= m x) (= m y)) ; bit vector signed greater than
        
        ; Post condition negation (~Q)
        (not (and
            (bvsge m x) ; bit vector signed great or equal
            (bvsge m y)
            (or (= m x) (= m y))
        ))
    )
)

(check-sat)
(exit)