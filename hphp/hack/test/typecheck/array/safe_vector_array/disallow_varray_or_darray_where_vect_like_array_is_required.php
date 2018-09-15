<?hh // strict

function consumeArrayOfInt(array<int> $arg): void {}

function provideDarrayOrVarrayOfInt(): varray_or_darray<int> {
  return darray["a" => 0];
}

function test(): void {
  consumeArrayOfInt(provideDarrayOrVarrayOfInt());
}
