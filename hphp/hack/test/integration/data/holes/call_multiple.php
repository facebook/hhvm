<?hh

function f(int $x, vec<int> $y):void {}

function call_multiple(bool $x, vec<bool> $y): void {
  /* HH_FIXME[4110] */
  f($x,$y); /* call_multiple.php:7:5 call_multiple.php:7:8 */
}
