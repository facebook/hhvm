<?hh // partial

function consumesArrayOfStringToInt(array<string, int> $array): void {}

function providesArrayOfUnknown(): array {
  return varray["tingley"];
}

function test(): void {
  consumesArrayOfStringToInt(providesArrayOfUnknown());
}
