<?hh // strict

coroutine function g(): int {
  // ok - argument of suspend is call to coroutine lambda
  $a = suspend (coroutine () ==> 10)();
  return $a;
}
