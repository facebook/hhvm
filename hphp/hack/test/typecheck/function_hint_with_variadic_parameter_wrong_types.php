<?hh

function variadic_int(int ...$y): void {}

function hint_mixed((function(int, int, bool): void) $f): void {}

function test(): void {
  hint_mixed(variadic_int<>);
}
