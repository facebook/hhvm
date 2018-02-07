<?hh

function f(int $integral, string $str, float $flt): void {}

function test(): void {
  $args = tuple("hello", 1.0);
  // positional args should be typechecked
  f(5, ...$args);
}
