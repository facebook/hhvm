<?hh // strict

function consumeArrayOfInt(varray<int> $arg): void {}

function provideDarrayOrVarrayOfInt(): varray_or_darray<int> {
  return darray["a" => 0];
}

function test(): void {
  consumeArrayOfInt(provideDarrayOrVarrayOfInt());
}
