<?hh // strict

function test_varray(varray<int> $x): void {
  unset($x[0]);
}

function test_varray_or_darray(varray_or_darray<int> $x): void {
  unset($x[0]);
}

function test_vec_array(array<int> $x): void {
  unset($x[0]);
}

function test_map_array(array<int, int> $x): void {
  unset($x[0]);
}

function test_darray(darray<int, int> $x): void {
  unset($x[0]);
}
