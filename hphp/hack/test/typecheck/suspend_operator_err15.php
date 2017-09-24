<?hh // strict

function a((coroutine function(): int) $a): void {}

function b(): void {
  // not ok - regular function is not compatible with coroutine
  a(() ==> 1);
}
