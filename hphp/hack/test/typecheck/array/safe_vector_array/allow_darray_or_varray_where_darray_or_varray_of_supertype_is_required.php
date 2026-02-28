<?hh

function consumeDarrayOrVarrayOfNum(varray_or_darray<num> $arg): void {}

function provideDarrayOrVarrayOfInt(): varray_or_darray<int> {
  return dict["a" => 0];
}

function test(): void {
  consumeDarrayOrVarrayOfNum(provideDarrayOrVarrayOfInt());
}
