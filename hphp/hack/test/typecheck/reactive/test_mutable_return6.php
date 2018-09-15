<?hh // strict

class C {}

<<__Rx, __MutableReturn>>
function make(C $c): C {
  // not OK - returns parameter
  return $c;
}
