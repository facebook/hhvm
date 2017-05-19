<?hh // strict

function consumeArrayOfArraykeyToInt(array<arraykey, int> $arg): void {}

function provideDarrayOrVarrayOfInt(): darray_or_varray<int> {
  return darray["a" => 0];
}

function test(): void {
  consumeArrayOfArraykeyToInt(provideDarrayOrVarrayOfInt());
}
