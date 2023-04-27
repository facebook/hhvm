<?hh //strict

function variadic_int(int ...$y): void {}

function hint_single_string((function(string): void) $f): void {}

function test(): void {
  hint_single_string(variadic_int<>);
}
