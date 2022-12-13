#lang pl
#| Q1
First we define a digit type:
<Digit> := 0 | 1 | 2 | ... | 9

Character is a digit but with #\ before.
<Character> := #\<Digit>


~~ List of characters, empty list or a character and then rest of the list.
<C> :=   λ
       <Character> <C>

~~ Number, sequence of digits, or the length of a given string.
<D> :=   λ
       <Digit><D>
       {string-length <str-op>}

~ String operations
<str-op> := a. "<D>"
            b. {string <C>}
            c. {string-append <str-op>}
            d. {string-insert <str-op> <Character> <D>}
            e. {number->string <D>}
            f. <str-op> <str-op> -- sequence of strs, nothing says it's forbidden outside funcs

~ SE Language
Then we define our string expression SE:
<SE> := 1. <Character> --- sequence of single characters are not allowed as shown in example.
        2. <D>
        3. <str-op>
_____________________________________________________________________________________

<SE> => rule 3
<str-op> =>rule  a
(string-append <str-op>) => rule f
(string-append <str-op> <str-op>) => rules a, e
(string-append "42" (number->string <D>) => rule 2 of <D>
(string-append "42" (number->string (string-length <str-op>)) => rule a
(string-append "42" (number->string (string-length "<D>"))) => rule 1 of <D>
(string-append "42" (number->string (string-length "003344")))
(string-append "42" (number->string 6))
(string-append "42" "6")
_____________________________________________________________________________________

<SE> => rule 2
<D> => rule 3 of D
(string-length <str-op>) rule b of str-op, it turns into a string of #\ chars
(string-length (string <C>) ) 
(string-length (string #\1 #\2 #\4 ))
3

_____________________________________________________________________________________


<SE> =>
<str-op>
{string-append <str-op>} =>
{string-append <str-op> <str-op>} =>
{string-append "<D>" {string-append <str-op>}} =>
{string-append "<D>" {string-append <str-op> <str-op>}} =>
{string-append "<D>" {string-append "<D>" <str-op>}} =>
{string-append "<D>" {string-append "<D>" {string-append <str-op>}}} =>
{string-append "<D>" {string-append "<D>" {string-append "<D>"}}} =>
{string-append "<D>" {string-append "<D>" {string-append "3"}}} =>
{string-append "<D>" {string-append "2" {string-append "3"}}} =>
(string-append "1" (string-append "2" (string-append "3")))

|#


#| Q2 |#

#| Input : Number, returns the number squared. |#
(: square-num : Number -> Number)
(define (square-num n)
  (* n n)
  )

(test (square-num 0) => 0)
(test (square-num 1) => 1)
(test (square-num -1) => 1)
(test (square-num 5) => 25)

#| Input: List of numbers, return a list with each number squared. |#
;; Using the square function on a single element, we use the map function to apply square to all elements in the list.
(: square-list : (Listof Number) -> (Listof Number))
(define (square-list lst)
  (map square-num lst)
  )

(test (square-list '()) => '())
(test (square-list '(0 1 2)) => '(0 1 4))
(test (square-list '(-2 4 2)) => '(4 16 4))
(test (square-list '(1 1 1 1 1)) => '(1 1 1 1 1))

#| Input: List of numbers, return the sum of the squared list. |#
;; Using foldl function to apply summation between 0 and each element of the squared list.
(: sum-of-squares : (Listof Number) -> Number)
(define (sum-of-squares lst)
  (foldl + 0 (square-list lst))
  )

(test (sum-of-squares '()) => 0)
(test (sum-of-squares '(0 1 2)) => 5)
(test (sum-of-squares '(-2 4 2)) => 24)
(test (sum-of-squares '(1 1 1 1 1)) => 5)

#| Q3 |#

(: createPolynomial : (Listof Number) -> (Number -> Number))
(define (createPolynomial coeffs)
  (: poly : (Listof Number) Number Integer Number -> Number)
  (define (poly argsL x power accum)
    (if (null? argsL) accum ;; if end, return sum
        (poly (rest argsL) x (+ power 1) (+ accum (* (first argsL) (expt x power))))
        ) ;; return current sum + current coefficint multiplied by x to some power + next sum on the rest of the list
    )
  (: polyX : Number -> Number)
  (define (polyX x)
    (poly coeffs x 0 0) ;; start poly calculation with power 0, sum 0 and first coefficient
    )
  polyX
  )

(define p2345 (createPolynomial '(2 3 4 5)))
(test (p2345 0) => (+ (* 2 (expt 0 0)) (* 3 (expt 0 1)) (* 4 (expt 0 2)) (* 5 (expt 0 3))))
(test (p2345 4) =>(+ (* 2 (expt 4 0)) (* 3 (expt 4 1)) (* 4 (expt 4 2)) (* 5 (expt 4 3))))
(test (p2345 11) => (+ (* 2 (expt 11 0)) (* 3 (expt 11 1)) (* 4 (expt 11 2)) (* 5 (expt 11 3))))

(define p536 (createPolynomial '(5 3 6)))
(test (p536 11) => (+ (* 5 (expt 11 0)) (* 3 (expt 11 1)) (* 6 (expt 11 2))))

(define p_0 (createPolynomial '()))
(test (p_0 4) => 0)

;; f(x) = x
(define myPoly (createPolynomial '(0 1)))
(test (myPoly 1) => 1)
(test (myPoly 2) => 2)
(test (myPoly 3) => 3)
(test (myPoly 4) => 4)

;; f(x) = x^2
(define mySqrPoly (createPolynomial '(0 0 1)))
(test (mySqrPoly 1) => 1)
(test (mySqrPoly -2) => 4)
(test (mySqrPoly 3) => 9)
(test (mySqrPoly -4) => 16)

#| PLANG |#

#|
 The grammar:
 <PLANG> ::= {poly {<AEs>} {<AEs>}}       Two groups of numbers.
 <AEs> ::= {<AE>} | {<AE>} {<AEs>}   One or several expressions.
 <AE> ::= <Num>         1            Arithmetic expression.
       | {+ <AE> <AE> } 2
       | {- <AE> <AE> } 3
       | {* <AE> <AE> } 4
       | {/ <AE> <AE> } 5
 |#

#| PARSER |#

(define-type PLANG
  [Poly (Listof AE) (Listof AE)])
(define-type AE
  [Num Number]
  [Add AE AE]
  [Sub AE AE]
  [Mul AE AE]
  [Div AE AE])
(: parse-sexpr : Sexpr -> AE) ;; to convert s-expressions into AEs
(define (parse-sexpr sexpr)
  (match sexpr
    [(number: n) (Num n)]
    [(list '+ lhs rhs) (Add (parse-sexpr lhs)
                            (parse-sexpr rhs))]
    [(list '- lhs rhs) (Sub (parse-sexpr lhs)
                            (parse-sexpr rhs))]
    [(list '* lhs rhs) (Mul (parse-sexpr lhs)
                            (parse-sexpr rhs))]
    [(list '/ lhs rhs) (Div (parse-sexpr lhs)
                            (parse-sexpr rhs))]
    [else (error 'parse-sexpr "bad syntax in ~s" sexpr)]))

(: parse : String -> PLANG) ;; parses a string containing a PLANG expressionto a PLANG AST
(define (parse str)
  (let ([code (string->sexpr str)])
    (match code
      [(list (list 'poly) (list points ...)) (error 'parse "at least one coefficient is required in ~s" code)] ;; No coeffs
      [(list (list 'poly coeffs ...) '()) (error 'parse "at least one point is required in ~s" code)]          ;; No points
      [(list (list 'poly coeffs ...) (list points ...))                                                        ;; Valid syntax
       (Poly (map parse-sexpr coeffs) (map parse-sexpr points))] ;; apply parse via map on each element in coefficint list and in points list.
      [else (error 'parse "bad syntax in ~s" code)]
      )
    )
  )

(test (parse "{{poly 1 2 3} {1 2 3}}") => (Poly (list (Num 1) (Num 2) (Num 3)) (list (Num 1) (Num 2) (Num 3))))
(test (parse "{{poly } {1 2} }") =error> "parse: at least one coefficient is required in ((poly) (1 2))")
(test (parse "{{poly 1 2} {} }") =error> "parse: at least one point is required in ((poly 1 2) ())")

#| EVAL POLY |#

;; evaluates AE expressions to numbers
(: eval : AE -> Number)
(define (eval expr)
  (cases expr
    [(Num n) n]
    [(Add l r) (+ (eval l) (eval r))]
    [(Sub l r) (- (eval l) (eval r))]
    [(Mul l r) (* (eval l) (eval r))]
    [(Div l r) (/ (eval l) (eval r))]
    )
  )
(: eval-poly : PLANG -> (Listof Number))
(define (eval-poly p-expr)
  (cases p-expr ;; For (Poly (coeff list) (point list)) first evaluate all coeffs and points
    [(Poly l r) (map (createPolynomial(map eval l)) (map eval r))] ;; and then create polynom with coeffs and apply it to all points with map.
    )
  )
(: run : String -> (Listof Number)) ;; evaluate a FLANG program contained in a string
(define (run str)
  (eval-poly (parse str)))

(test (run "{{poly 1 2 3} {1 2 3}}") => '(6 17 34))
(test (run "{{poly 4 2 7} {1 4 9}}") => '(13 124 589))
(test (run "{{poly 1 2 3} {1 2 3}}") => '(6 17 34))
(test (run "{{poly 4/5 } {1/2 2/3 3}}") => '(4/5 4/5 4/5))
(test (run "{{poly 2 3} {4}}") => '(14))
(test (run "{{poly 1 1 0} {-1 3 3}}") => '(0 4 4))
(test (run "{{poly {/ 4 2} {- 4 1}} {{- 8 4}}}") => '(14))
(test (run "{{poly {+ 0 1} 1 {* 0 9}} {{- 4 5} 3 {/ 27 9}}}") => '(0 4 4))
