<?hh

function f(string ...$args,): void {}

function test() {
  f('str', 'str', 20);
}
