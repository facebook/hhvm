<?hh

class A {}

class B extends A {}

function variadic_B(B ...$y): void {}

function hint_single_A((function(A): void) $f): void {}

function test(): void {
  hint_single_A(variadic_B<>);
}
