; if(x < y && !(x + z < y + z)
;   // body
;
; Is it possible to enter the body of this if-condition?

; This gives us unsat: 
;
; (set-logic QF_LIA)
; (declare-fun x () Int)
; (declare-fun y () Int)
; (declare-fun z () Int)
;
; (assert
;    (and
;        (< x y)
;        (not (< (+ x z) (+ y z)))
;    )
; )
;
; (check-sat)
; (exit) 
;
; But can this theory represent the calculations done by the computer?
; The answer is NO because we represent numbers with two's complement and we have possible overflow!
; That's why we need to use QF_BV theory

(set-logic QF_BV)
(declare-fun x () (_ BitVec 32))
(declare-fun y () (_ BitVec 32))
(declare-fun z () (_ BitVec 32))

(assert
    (and
        (bvslt x y)
        (not (bvslt (bvadd x z) (bvadd y z)))
    )
)

(check-sat)
(get-value (x y z))
(exit)