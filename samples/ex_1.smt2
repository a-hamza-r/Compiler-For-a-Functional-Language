; program 1
; (eval (= (if (> 1 0) (+ 2 3) (+ 3 4)) 5))

(declare-var v2 Int)
(declare-var v3 Int)
(declare-var v4 Bool)
(declare-var v5 Int)
(declare-var v6 Int)
(declare-var v8 Int)
(declare-var v9 Int)
(declare-var v11 Int)

(assert
(and
(= v2 1)
(= v3 0)
(= v4 (> v2 v3))))

(assert
(=> v4
(and
(= v5 2)
(= v6 3)
(= v11 (+ v5 v6)))))

(assert
(=> (not v4)
(and
(= v8 3)
(= v9 4)
(= v11 (+ v8 v9))
)))

; negation of the formula under `eval`
(assert (not (= v11 5)))

(check-sat)
(get-model)
