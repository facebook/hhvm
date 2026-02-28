<?hh

interface I {}
class A {}
class C extends A {}

function inter_l((A & I) $ai): C {
  $c = $ai;
  return $c;
}
