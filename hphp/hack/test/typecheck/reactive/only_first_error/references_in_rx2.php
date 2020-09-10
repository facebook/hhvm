<?hh // partial

function by_ref(int &$ref) {}

<<__Rx>>
function foo(): void {
  $x = 1;
  by_ref(&$x); // bad
}
