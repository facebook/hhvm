<?hh

interface I {}
class A {}
class B extends A implements I {}
class C extends A {}

function inter_r(C $c) : (A & I) {
  return $c;
}

function inter_l((A & I) $ai): C {
  $c = $ai;
  return $c;
}
