<?hh // partial

function f(string $foo): void {}

function test(): void {
  $args = varray[];
  // arity error
  f('string', ...$args);
}
