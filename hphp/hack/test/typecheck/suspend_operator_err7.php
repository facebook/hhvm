<?hh // strict

coroutine function f(mixed $v): int {
  // not ok - argument to suspend is mixed value
  $a = suspend $v;
  return 1;
}
