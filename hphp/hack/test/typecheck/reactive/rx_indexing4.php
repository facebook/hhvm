<?hh // strict

<<__Rx>>
function f(<<__Mutable>>Vector<string> $m): void {
  // ERROR
  $m[0] = "42";
}
