<?hh

function f(mixed ...$args): void {}

function test(): void {
  $v = vec[];
  f(...$v, ...$v);
}
