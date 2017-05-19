<?hh

function consumeArrayWithNoTypeParameters(array $arg): void {}

function provideDarrayOrVarrayOfAny(): darray_or_varray {
  return darray["a" => 0];
}

function test(): void {
  consumeArrayWithNoTypeParameters(provideDarrayOrVarrayOfAny());
}
