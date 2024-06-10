<?hh
class A {}
class B {}
class C {}
<<__NoAutoLikes>>
function union_r(C $c): (A|B) {
  return $c;
}
