<?hh

function g(int &$x): void {
}

<<__Rx>>
function f(int $x): void {
  if (\HH\Rx\IS_ENABLED) {
  } else {
    /* HH_IGNORE_ERROR[4299] exempt due to partial-mode reference ban, not because of RX */
    $y = &$x;
    g(&$x);
  }
}
