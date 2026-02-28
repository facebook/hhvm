<?hh

function f(int $foo, mixed ...$args): void {}

function test(): void {
  $args = vec[1, 2, 3];
  // positional args should be typechecked
  f('string', ...$args);
}
