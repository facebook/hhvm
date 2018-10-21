<?hh // strict

class C {}

<<__Rx, __MutableReturn>>
function make(): C {
  $a = \HH\Rx\mutable(new C());
  $b = \HH\Rx\freeze($a);
  // not OK - returns immutable
  return $b;
}
