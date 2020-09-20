<?hh // partial

function consumesUnknownArray(array $array): void {}

function providesArrayOfInt(): array<int> {
  return varray[0];
}

function test(): void {
  consumesUnknownArray(providesArrayOfInt());
}
