<?hh // strict

coroutine function f((function(): int) $s): int {
  // not ok - argument to suspend is not a call to coroutine
  $a = suspend $s();
  return 1;
}
