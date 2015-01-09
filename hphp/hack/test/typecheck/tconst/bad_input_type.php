<?hh // strict

class C {
  const type T = int;
}

function foo(C::T $x): void {}

function test(): void {
  foo('');
}
