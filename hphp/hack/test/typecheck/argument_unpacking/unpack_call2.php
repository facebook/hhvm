<?hh

function f(int $foo, mixed ...$args): void {}

function test(): void {
  $args = varray[1, 2, 3];
  // positional args should be typechecked
  f('string', ...$args);
}
