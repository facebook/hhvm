<?hh // strict

coroutine function g((coroutine function(): int) $c): int {
  // ok - argument of suspend is call to coroutine typed parameter
  $a = suspend $c();
  return $a;
}
