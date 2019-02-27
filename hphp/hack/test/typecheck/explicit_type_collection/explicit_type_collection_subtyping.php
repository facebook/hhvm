<?hh // strict

class B {}
class C extends B {}
class D extends B {}

function f(B $b, C $c, D $d): void {
  $e = vec<B>[$b, $c, $d];
}
