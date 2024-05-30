<?hh
class A {}
class B {}
class C {}

function union_r(C $c): (A|B) {
  return $c;
}
