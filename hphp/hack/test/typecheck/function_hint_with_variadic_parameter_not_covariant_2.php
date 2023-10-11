<?hh

class A {}

class B extends A {}

function variadic_B(B ...$y): void {}

function hint_variadic_A((function(A...): void) $f): void {}

function test(): void {
  hint_variadic_A(variadic_B<>);
}
