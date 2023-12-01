<?hh

function test1(): varray_or_darray<int> {
  return dict[true => 0]; // error
}

function test2(bool $x): varray_or_darray<int, int> {
  if ($x) {
    return dict['foo' => 0]; // error
  } else {
    return dict[0 => 0]; // ok
  }
}
