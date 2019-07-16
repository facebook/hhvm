<?hh // partial

// This is a function of int and vararg
function func_with_vararg(int $a, mixed ...$args): void {}

// This is a function that takes a function of int and variadic
function hint_variadic((function(int, mixed...): void) $f): void {}

function test(): void {
  hint_variadic(fun('func_with_vararg'));
}
