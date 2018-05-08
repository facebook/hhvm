<?hh // strict

class C {}

<<__Rx, __MutableReturn>>
function make(bool $b, <<__Mutable>>C $c): C {
  if ($b) {
    $c = \HH\Rx\mutable(new C());
  } else {
  }
  // should not be OK - return value is not mutably owned
  return $c;
}
