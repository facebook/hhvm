<?hh // strict

<<__Rx>>
function f(<<__Mutable>>Vector<string> $m): void {
  // OK
  $m[0] = "42";
}
