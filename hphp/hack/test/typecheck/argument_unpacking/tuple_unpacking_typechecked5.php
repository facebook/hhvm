<?hh

function f(
  int $integral,
  string $str,
  float $flt,
  int $another_integral,
): void {}

function test(): void {
  $args = tuple("hello", 2.0);
  f(11, ...$args);
}
