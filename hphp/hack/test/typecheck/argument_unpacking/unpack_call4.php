<?hh

function f(string $foo): void {}

function test(): void {
  $args = array();
  // arity error
  f('string', ...$args);
}
