; program 2
;(define-fun (f1 (int i)) int (+ i 1))
;(define-fun (f2 (int i)) bool (= (f1 i) (f1 i)))
;(eval (f2 (get-int)))

(declare-var v2 Int)
(declare-var v3 Int)
(declare-var v4 Int)
(declare-var v5 Int)
(declare-var v6 Int)
(declare-var v8 Int)
(declare-var v9 Int)
(declare-var v10 Int)
(declare-var v17 Bool)
(declare-var v11 Int)
(declare-var v12 Int)
(declare-var v13 Int)
(declare-var v14 Bool)
(declare-var v15 Int)
(declare-var v16 Int)
(declare-var v22 Int)
(declare-var v23 Int)
(declare-var v24 Int)
(declare-var v25 Int)
(declare-var v26 Int)
(declare-var a1 Int)
(declare-var a2 Int)
(declare-var a3 Int)
(declare-var a4 Int)
(declare-var rv1 Bool)
(declare-var rv2 Int)
(declare-var rv3 Int)
(declare-var rv Int)

; f1
(assert (and
(= v2 a2)
(= v4 v2)
(= v5 1)
(= v6 (+ v4 v5))
(= rv3 v6)))

; f1 (clone + rename)
(assert (and
(= v22 a3)
(= v24 v22)
(= v25 1)
(= v26 (+ v24 v25))
(= rv2 v26)))

; f2
(assert (and
(= v8 a1)
(= v10 v8)
(= a2 v10)
(= v11 rv3)
(= v12 v8)
(= a3 v12)
(= v13 rv2)
(= v14 (= v11 v13))
(= rv1 v14)))

; entry
(assert (and
(= v16 rv)
(= a1 v16)
(= v17 rv1)))

(assert (not v17))

(check-sat)
