<?hh

function f(string $foo): void {}

function test(): void {
  $args = vec[];
  // arity error
  f('string', ...$args);
}
