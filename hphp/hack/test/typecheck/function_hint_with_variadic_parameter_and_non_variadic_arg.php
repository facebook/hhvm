<?hh //strict

function non_variadic(int $a, string $y): void {}

function hint_variadic((function(mixed...): void) $f): void {}

function test(): void {
  hint_variadic(non_variadic<>);
}
