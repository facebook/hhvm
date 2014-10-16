<?hh

function f(int $foo): void {}

function test(): void {
  $args = array();
  // arity error
  f('string', ...$args);
}
