<?hh // strict

function f(string $x): void {
  /* UNSAFE_EXPR
   * Multiline comments should not cause following line numbers to break.
   */
  $y = $x + 1;
}

function g(string $x): void {
  $y = $x + 1;
}
