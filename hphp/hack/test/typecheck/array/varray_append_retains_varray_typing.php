<?hh

// hh_show is discouraged. However, due to the nature of how the type system
// convolutes array and varray, this is the most straightforward approach.

function test(): void {
  $int_array = varray[];
  hh_show($int_array);
  $int_array[] = 42;
  expect_vector_of_int($int_array);
}

function expect_vector_of_int(varray<int> $varr): void {}
