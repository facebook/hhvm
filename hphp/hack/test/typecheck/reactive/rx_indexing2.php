<?hh // strict

function f(Vector<string> $m)[]: void {
  // ERROR
  $m[0] = "42";
}
