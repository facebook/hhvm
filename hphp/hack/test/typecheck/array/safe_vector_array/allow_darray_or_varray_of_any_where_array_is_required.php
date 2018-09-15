<?hh

function consumeArrayWithNoTypeParameters(array $arg): void {}

function provideDarrayOrVarrayOfAny(): varray_or_darray {
  return darray["a" => 0];
}

function test(): void {
  consumeArrayWithNoTypeParameters(provideDarrayOrVarrayOfAny());
}
