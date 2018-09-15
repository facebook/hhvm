<?hh

<<__Rx>>
function foo(): void {
  $x = 1;
  $y = &$x; // bad
}
