<?hh

function f(string ...$args,): void {}

function test(): void {
  f('str', 'str', 20);
}
