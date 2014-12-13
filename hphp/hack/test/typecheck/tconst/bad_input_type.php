<?hh // strict

class C {
  public type const T = int;
}

function foo(C::T $x): void {}

function test(): void {
  foo('');
}
