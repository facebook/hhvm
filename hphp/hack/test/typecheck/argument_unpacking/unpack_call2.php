<?hh

function f(int $foo, ...$args): void {}

function test(): void {
  $args = array(1, 2, 3);
  // positional args should be typechecked
  f('string', ...$args);
}
