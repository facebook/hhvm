<?hh // strict

coroutine function g(): int {
  // ok - argument of suspend is call to coroutine anonymous function
  $a = suspend (coroutine function() {
                  return 1;
                })();
  return $a;
}
