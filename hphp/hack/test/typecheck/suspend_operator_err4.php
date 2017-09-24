<?hh // strict

function g(): int {
  return 1;
}

coroutine function f(): int {
  // not ok - argument to suspend is not a call to coroutine
  $a = suspend g();
  return 1;
}
