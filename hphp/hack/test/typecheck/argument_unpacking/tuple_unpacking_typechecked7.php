<?hh // partial

function f(
  int $integral,
  string $str,
  float $flt,
  int $integral_with_default = 3,
): void {}

function test(): void {
  $args = tuple("hello", 2.0);
  f(11, ...$args);
}
