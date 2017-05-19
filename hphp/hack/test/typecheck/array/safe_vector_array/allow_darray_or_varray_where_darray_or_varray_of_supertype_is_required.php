<?hh // strict

function consumeDarrayOrVarrayOfNum(darray_or_varray<num> $arg): void {}

function provideDarrayOrVarrayOfInt(): darray_or_varray<int> {
  return darray["a" => 0];
}

function test(): void {
  consumeDarrayOrVarrayOfNum(provideDarrayOrVarrayOfInt());
}
