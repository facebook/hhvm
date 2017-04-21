<?hh

function provideVarrayOfInt(): varray<int> {
  return varray[0, 1, 2, 3];
}

function provideVarrayOfArraykey(): varray<arraykey> {
  return provideVarrayOfInt();
}
