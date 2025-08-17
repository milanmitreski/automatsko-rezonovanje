; m = a[0];
; for(int i = 1; i < 5; i++)
;   if(a[i] > m)
;       m = a[i]
;
; Prove that m = max{a[0],...,a[n-1]}
; We can prove this for some fixed n

(set-logic QF_ALIA) ; Quantifier Free Array Linear Integer Arithmetic
(declare-fun i0 () Int) ; i value after 0 iterations
(declare-fun i1 () Int) ; i value after 1 iterations
(declare-fun i2 () Int) 
(declare-fun i3 () Int)
(declare-fun i4 () Int)

(declare-fun m0 () Int)
(declare-fun m1 () Int)
(declare-fun m2 () Int)
(declare-fun m3 () Int)
(declare-fun m4 () Int)

(declare-fun a () (Array Int Int))

(assert
    (and
        (= m0 (select a 0))
        (= i0 1)

        (= i1 (+ i0 1))
        (ite (> (select a i0) m0)
            (= m1 (select a i0))
            (= m1 m0)
        )

        (= i2 (+ i1 1))
        (ite (> (select a i1) m1)
            (= m2 (select a i1))
            (= m2 m1)
        )

        (= i3 (+ i2 1))
        (ite (> (select a i2) m2)
            (= m3 (select a i2))
            (= m3 m2)
        )

        (= i4 (+ i3 1))
        (ite (> (select a i3) m3)
            (= m4 (select a i3))
            (= m4 m3)
        )

        (not
            (and
                (>= m4 (select a 0))
                (>= m4 (select a 1))
                (>= m4 (select a 2))
                (>= m4 (select a 3))
                (>= m4 (select a 4))
                (or
                    (= m4 (select a 0))
                    (= m4 (select a 1))
                    (= m4 (select a 2))
                    (= m4 (select a 3))
                    (= m4 (select a 4))
                )
            )
        )
    )
)

(check-sat)
(exit)
