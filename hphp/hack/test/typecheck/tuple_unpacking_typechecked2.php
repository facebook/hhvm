<?hh

// Repeated types
function f(int $integral, int $another_integral): void {}

function test(): void {
  $args = tuple(11, 12);
  // positional args should be typechecked
  f(...$args);
}
