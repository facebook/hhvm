<?hh // partial

function consumesArrayOfInt(array<int> $array): void {}

function providesArrayOfUnknown(): array {
  return array("tingley");
}

function test(): void {
  consumesArrayOfInt(providesArrayOfUnknown());
}
