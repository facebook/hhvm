<?hh // strict

<<__Rx>>
function f(int $a): void {
  // ERROR
  $b = \HH\Rx\mutable($a);
}
