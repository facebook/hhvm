<?hh // partial

<<__Rx>>
function foo(): int {
  static $a;
  $a = 5;
  return $a;
}
