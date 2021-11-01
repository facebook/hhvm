<?hh

function f(int $foo, mixed ...$args): void {}

function test(): void {
  $args = 'string';
  f(...$args); // should be error
}
