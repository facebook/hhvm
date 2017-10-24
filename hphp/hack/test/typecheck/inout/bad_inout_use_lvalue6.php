<?hh

function f(inout int $i): void {}

function & g(): int {
  static $x = 42;
  return $x;
}

function test(): void {
  f(inout g());
}
