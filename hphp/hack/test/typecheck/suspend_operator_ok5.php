<?hh // strict

coroutine function g(): int {
  // ok - argument of suspend is call to coroutine typed value
  $c = coroutine () ==> 1;
  $a = suspend $c();
  return $a;
}
