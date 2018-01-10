<?hh // strict

class C {}

<<__Rx, __MutableReturn>>
function make(bool $b, C $c): C {
  if ($b) {
    $a = new C();
  } else {
    $a = $c;
  }
  // should not be OK - return value is not mutably owned
  return $a;
}
