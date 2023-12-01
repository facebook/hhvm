<?hh

function consumeArrayOfArraykeyToInt(darray<arraykey, int> $arg): void {}

function provideDarrayOrVarrayOfInt(): varray_or_darray<int> {
  return dict["a" => 0];
}

function test(): void {
  consumeArrayOfArraykeyToInt(provideDarrayOrVarrayOfInt());
}
