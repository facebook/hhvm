<?hh //strict

function variadic_int(int ...$y): void {}

function hint_variadic_string((function(string...): void) $f): void {}

function test(): void {
  hint_variadic_string(variadic_int<>);
}
