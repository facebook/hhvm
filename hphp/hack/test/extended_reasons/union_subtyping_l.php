<?hh
class A {}
class B {}
class C {}

function union_l((A | B) $ab): C {
  return $ab;
}
