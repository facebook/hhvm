<?hh // strict

<<__Rx>>
function f(Vector<string> $m): void {
  // ERROR
  $m[0] = "42";
}
