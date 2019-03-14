<?hh // partial

function test(): varray_or_darray<int> {
  return darray[true => 0];
}
