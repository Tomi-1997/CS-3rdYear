#lang pl
#|
For each list in a given list of lists, take the first list,
remove it from the list of lists and append it to the result
after applying the same function on the reduced list of lists.
|#
(: open-list : (Listof (Listof Number)) -> (Listof Number))
(define (open-list lst)
       (cond [(null? lst) null]
        [else (append (first lst) (open-list (rest lst)))]))
#|
My direction to solve this and most of the functions here are to think of the problem
as if my input is only one or two elements, convert it to recursive and then generalise it.
|#
(test (open-list '(() () () (1))) => '(1))
(test (open-list '(() () () ())) => '())
(test (open-list '((1 2 3) (4 5 6))) => '(1 2 3 4 5 6))
(test (open-list '((1) () () () () () () (1))) => '(1 1))
(test (open-list '((1 2 3) (1 2 3) (1 2 3) (1 2 3))) => '(1 2 3 1 2 3 1 2 3 1 2 3))

#|
For each function visit, remove the first item, using the built-in min function,
compare the first item with the result of the same function on the list,
without the first item.
|#
(: my_min : (Listof Number) -> Number)
(define (my_min lst)
       (cond [(null? lst) +inf.0]
             [else (min (first lst) (my_min (rest lst)))]
             )
  )
(test (my_min '( 2 9 3 8 4 7 5)) => 2.0)
(test (my_min '( 1 1 1 1 1 1 1)) => 1.0)
(test (my_min '( -4 9 2 1 20 7 1)) => -4.0)
(test (my_min '()) => +inf.0)


#|
For each function visit, remove the first item, using the built-in max function,
compare the first item with the result of the same function on the list,
without the first item.
|#
(: my_max : (Listof Number) -> Number)
(define (my_max lst)
       (cond [(null? lst) -inf.0]
             [else (max (first lst) (my_max (rest lst)))]
             )
  )
(test (my_max '( 2 9 3 8 4 7 5)) => 9.0)
(test (my_max '( 1 1 1 1 1 1 1)) => 1.0)
(test (my_max '( -4 -9 -2 -1 -20 -7 -1)) => -1.0)
(test (my_max '()) => -inf.0)

#|
Use the above functions, merge many lists into one, and construct a new
list having only the minimum and maximum from the original list of lists.
|#
(: min&max : (Listof (Listof Number)) -> (Listof Number))
(define (min&max lst)
  (cond [(null? (open-list lst)) '(-inf.0 +inf.0)]
        [else (cons (my_min (open-list lst)) (cons (my_max (open-list lst)) null))]
  ))

(test (min&max '((1 2 3) (2 3 3 4) (9 2 -1) (233 11 90))) => '(-1.0 233.0))
(test (min&max '(() () () ())) => '(-inf.0 +inf.0))
(test (min&max '(() () (1) ())) => '(1.0 1.0))
(test (min&max '((1) () (1) ())) => '(1.0 1.0))

#|
Using only the open-list function above, transform a list of lists into one merged list,
and apply the min and max functions on the result to construct a new list.
|#
(: min&max_apply : (Listof (Listof Number)) -> (Listof Number))
(define (min&max_apply lst)
  (cond [(null? (open-list lst)) '(-inf.0 +inf.0)]
        [else (cons (apply min (open-list lst)) (cons (apply max (open-list lst)) null))]
  ))

(test (min&max_apply '((1 2 3) (2 3 3 4) (9 2 -1) (233 11 90))) => '(-1 233))
(test (min&max_apply '(() () () ())) => '(-inf.0 +inf.0))
(test (min&max_apply '(() () (1) ())) => '(1 1))
(test (min&max_apply '((1) () (1) ())) => '(1 1))

#|
A table which every element is made of (Symbol, String, Link to next element \ table)
|#
(define-type Table
            [EmptyTbl]
            [Add Symbol String Table]
            )

(test (EmptyTbl) => (EmptyTbl))
(test (Add 'b "B" (Add 'a "A" (EmptyTbl))) => (Add 'b "B" (Add 'a "A" (EmptyTbl))))
(test (Add 'a "aa" (Add 'b "B" (Add 'a "A" (EmptyTbl)))) => (Add 'a "aa" (Add 'b "B" (Add 'a "A" (EmptyTbl)))))
(test (Add 'd "shalom" (Add 'c "HI" (Add 'b "B" (Add 'a "A" (EmptyTbl))))) => (Add 'd "shalom" (Add 'c "HI" (Add 'b "B" (Add 'a "A" (EmptyTbl))))))
(test (Add 'l "l for long" (Add 't "tomas" (Add 'd "shalom" (Add 'c "HI" (Add 'b "B" (Add 'a "A" (EmptyTbl))))))) => (Add 'l "l for long" (Add 't "tomas" (Add 'd "shalom" (Add 'c "HI" (Add 'b "B" (Add 'a "A" (EmptyTbl))))))))

#|
Find target symbol, if current table is the target return the string,
else call the same function but with the next table \ element until it is empty.
|#
(: search-table : Symbol Table -> (U #f String))
(define (search-table target table)
  (cases table
  [(Add sym st tb) (cond [(eq? target sym) st]
                         [else (search-table target tb)])]
  [EmptyTbl #f]
    ))

(test (search-table 'c (Add 'l "l for long" (Add 't "tomas" (Add 'd "shalom" (Add 'c "HI" (Add 'b "B" (Add 'a "A" (EmptyTbl)))))))) => "HI")
(test (search-table 'e (Add 'l "l for long" (Add 't "tomas" (Add 'd "shalom" (Add 'c "HI" (Add 'b "B" (Add 'a "A" (EmptyTbl)))))))) => #f)
(test (search-table 'a (Add 'l "l for long" (Add 't "tomas" (Add 'd "shalom" (Add 'c "HI" (Add 'b "B" (Add 'a "A" (EmptyTbl)))))))) => "A")
(test (search-table 'b (Add 'l "l for long" (Add 't "tomas" (Add 'd "shalom" (Add 'c "HI" (Add 'b "B" (Add 'a "A" (EmptyTbl)))))))) => "B")

#|
Find target symbol, if found then link previous table to next table, skipping the unwanted symbol element.
Else, link the previous table while returning to the function with the next table, to preserve the structure.
|#
(: remove-item : Table Symbol -> Table)
(define (remove-item table target)
  (cases table
  [(Add sym st tb) (cond [(eq? target sym) tb]
                         [else (Add sym st (remove-item tb target))])]
  [EmptyTbl table]
    ))

(test (search-table 'e (remove-item (Add 'e "E" (Add 'd "D" (Add 'c "C" (Add 'b "B" (Add 'a "A" (EmptyTbl)))))) 'e)) => #f)
(test (remove-item (remove-item (remove-item (remove-item (remove-item (Add 'e "E" (Add 'd "D" (Add 'c "C" (Add 'b "B" (Add 'a "A" (EmptyTbl)))))) 'a) 'b) 'c) 'd) 'e) => (EmptyTbl))
(test (remove-item (EmptyTbl) 'd) => (EmptyTbl))
(test (remove-item (Add 'a "A" (EmptyTbl)) 'b) => (Add 'a "A" (EmptyTbl)))
