<?hh // strict

coroutine function f(): int {
  // not ok - argument to suspend is not a call to coroutine
  $a = suspend 10;
  return 1;
}
