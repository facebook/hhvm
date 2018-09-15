<?hh // strict

coroutine function g(): int {
  return 1;
}

function f(): int {
  // not ok - call to coroutine outside the suspend
  $a = g();
  return 1;
}
