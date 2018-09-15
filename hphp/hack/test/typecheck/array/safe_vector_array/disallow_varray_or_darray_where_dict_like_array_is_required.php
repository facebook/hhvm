<?hh // strict

function consumeArrayOfArraykeyToInt(array<arraykey, int> $arg): void {}

function provideDarrayOrVarrayOfInt(): varray_or_darray<int> {
  return darray["a" => 0];
}

function test(): void {
  consumeArrayOfArraykeyToInt(provideDarrayOrVarrayOfInt());
}
