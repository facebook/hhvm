<?hh // strict

<<__Rx>>
function f(Map<int, string> $m): void {
  // ERROR
  $m[1] = "42";
}
