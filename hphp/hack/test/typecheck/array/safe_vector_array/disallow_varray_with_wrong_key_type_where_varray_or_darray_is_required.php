<?hh

function test1(): varray_or_darray<string, int> {
  return varray[0]; // error
}

function test2(): varray_or_darray<int, int> {
  return varray[0]; // ok
}
