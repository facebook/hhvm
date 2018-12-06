<?hh // strict

<<__Rx>>
function f(<<__Mutable>>Map<int, string> $m): void {
  // OK
  $m[1] = "42";
}
