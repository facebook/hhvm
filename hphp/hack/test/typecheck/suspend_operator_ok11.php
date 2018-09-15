<?hh // strict

function f((coroutine function(): int) $a): void {}

function b(): void {
  // ok - pass coroutine lambda as coroutine typed value
  f(coroutine () ==> 42);
}
