<?hh //strict

class A {}

class B extends A {}

class C extends A {}

class D extends C {}

function variadic_A(A ...$y): void {}

function hint_single_B((function(B): void) $f): void {}

function hint_variadic_B((function(B...): void) $f): void {}

function hint_variadic_D((function(D...): void) $f): void {}

function hint_mixed_A_B_C_D((function(A, B, C, D): void) $f): void {}

function hint_mixed_variadic((function(A, B, C, D...): void) $f): void {}

function test(): void {
  hint_single_B(fun('variadic_A'));
  hint_variadic_B(fun('variadic_A'));
  hint_variadic_D(fun('variadic_A'));
  hint_mixed_A_B_C_D(fun('variadic_A'));
  hint_mixed_variadic(fun('variadic_A'));
}
