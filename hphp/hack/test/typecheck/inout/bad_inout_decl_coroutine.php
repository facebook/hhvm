<?hh // strict

coroutine function test(inout string $s): int {
  $f = coroutine () ==> 42;
  return suspend $f();
}
