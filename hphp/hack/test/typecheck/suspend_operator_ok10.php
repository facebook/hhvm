<?hh // strict

function f((coroutine function(): int) $a): void {}

function b(): void {
  // ok - pass coroutine anonymous function as coroutine typed value
  f(
    coroutine function() {
      return 1;
    },
  );
}
