<?hh // partial

function f(int $foo, ...$args) {}

function test() {
  $args = 'string';
  f(...$args); // should be error
}
