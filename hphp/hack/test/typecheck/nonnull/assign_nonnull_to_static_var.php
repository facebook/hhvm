<?hh // strict

function f(nonnull $x): void {
  static $s;
  $s = $x;
}
