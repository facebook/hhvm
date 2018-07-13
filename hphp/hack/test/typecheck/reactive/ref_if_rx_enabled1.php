<?hh

function g(int &$x): void {
}

<<__Rx>>
function f(int $x): void {
  if (\HH\Rx\IS_ENABLED) {
  } else {
    // OK
    $y = &$x;
    g(&$x);
  }
}
