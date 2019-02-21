<?hh // partial

function consumesArrayOfStringToInt(array<string, int> $array): void {}

function providesArrayOfUnknown(): array {
  return array("tingley");
}

function test(): void {
  consumesArrayOfStringToInt(providesArrayOfUnknown());
}
