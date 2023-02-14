<?hh //strict

function variadic_int(int ...$y): void {}

function hint_different_return_type((function(int): string) $f): void {}

function test(): void {
  hint_different_return_type(variadic_int<>);
}
