<?hh

function f(int $integral, string $str, float $flt): void {}

function test(): void {
  $args = tuple(1, "hello");
  f(...$args);
}
