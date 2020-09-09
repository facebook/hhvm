<?hh // partial

function by_ref(&$ref) {}

<<__Rx>>
function foo(): void {
  $x = 1;
  by_ref(&$x); // bad
}
