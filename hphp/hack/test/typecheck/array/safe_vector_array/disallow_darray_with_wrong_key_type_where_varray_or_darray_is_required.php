<?hh

function test1(): varray_or_darray<int> {
  return darray[true => 0]; // error
}

function test2(bool $x): varray_or_darray<int, int> {
  if ($x) {
    return darray['foo' => 0]; // error
  } else {
    return darray[0 => 0]; // ok
  }
}
