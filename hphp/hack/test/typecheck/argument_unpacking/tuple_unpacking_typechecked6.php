<?hh

function f(int $integral, float $flt, string $str): void {}

function test(): void {
  $args = tuple(1.0, 1);
  // positional args should be typechecked
  f(5, ...$args);
}
