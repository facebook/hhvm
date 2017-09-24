<?hh // strict

coroutine function f(): int {
  return 10;
}

coroutine function g(): int {
  // ok - argument of suspend is call to coroutine function
  $a = suspend f();
  return $a;
}
