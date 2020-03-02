<?hh // partial

function consumesArrayOfInt(array<int> $array): void {}

function providesArrayOfUnknown(): array {
  return varray["tingley"];
}

function test(): void {
  consumesArrayOfInt(providesArrayOfUnknown());
}
