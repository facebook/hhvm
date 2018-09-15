<?hh

// Has mixed types
function f(int $integral, string $str, float $flt): void {}

function test(): void {
  $args = tuple(1, "hello", 3.0);
  // positional args should be typechecked
  f(...$args);
}
