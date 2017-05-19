<?hh

function consumeArrayWithNoTypeParameters(array $arg): void {}

function provideDarrayOrVarrayOfInt(): darray_or_varray<int> {
  return darray["a" => 0];
}

function test(): void {
  consumeArrayWithNoTypeParameters(provideDarrayOrVarrayOfInt());
}
