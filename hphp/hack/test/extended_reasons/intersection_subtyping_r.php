<?hh

interface I {}
class A {}
class C extends A {}

function inter_r(C $c) : (A & I) {
  return $c;
}
