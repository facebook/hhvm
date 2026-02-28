<?hh

function test1(): varray_or_darray<string, int> {
  return vec[0]; // error
}

function test2(): varray_or_darray<int, int> {
  return vec[0]; // ok
}
