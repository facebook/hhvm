<?hh // partial

function f(inout int $i): void {}

function test(): void {
  $c = new C(); // intentional unbound name
  f(inout $c['foo']);
}
