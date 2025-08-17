(set-logic QF_LRA) ; Quantifier Free Linear Real Arithmetic
(declare-fun x () Real)
(declare-fun y () Real)

(assert
    (and 
        (> (+ (* 2 x) (* 3 y)) 1)
        (< (- (* 3 x) (* 2 y)) 0)
    )
)

(check-sat)
(get-value (x y))
(exit)