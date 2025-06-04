<?hh

interface I {}
class A {}
class C extends A {}

function inter_is(A $a, C $c): C {
 if($a is I) {
   $z = $a;
 } else {
   $z = $c;
 }
 return $z;
 //      ^ at-caret
}
