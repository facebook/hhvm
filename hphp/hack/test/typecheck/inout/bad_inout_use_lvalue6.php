<?hh // partial

function f(inout int $i): void {}

function g(): int {
  $x = 42;
  return $x;
}

function test(): void {
  f(inout g());
}
