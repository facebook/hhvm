<?hh

class A {}
class C extends A {}

function inter_equal(?A $a, C $c): C {
  if($a == null) {
    $z = $a;
  } else {
    $z = $c;
  }
  return $z;
}
